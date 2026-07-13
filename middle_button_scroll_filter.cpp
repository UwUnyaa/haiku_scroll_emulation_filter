#include <InputServerFilter.h>
#include <Message.h>
#include <Point.h>
#include <View.h>
#include <new>

class MiddleButtonScrollFilter : public BInputServerFilter {
private:
	bool fScrolling;
	bool fMiddleButtonPressed;
	BPoint fPreviousMousePosition;
	static constexpr float kScrollScale = 1.0f;
	static constexpr float kScrollThreshold = 1.0f;

public:
	MiddleButtonScrollFilter();
	virtual status_t InitCheck();
	virtual filter_result Filter(BMessage* message, BList* outList);
};

MiddleButtonScrollFilter::MiddleButtonScrollFilter()
	:	fScrolling(false)
	,	fMiddleButtonPressed(false)
{
}

status_t
MiddleButtonScrollFilter::InitCheck()
{
	return B_OK;
}

filter_result
MiddleButtonScrollFilter::Filter(BMessage* message, BList* outList)
{
	if (message->what != B_MOUSE_DOWN && message->what != B_MOUSE_MOVED
		&& message->what != B_MOUSE_UP)
		return B_DISPATCH_MESSAGE;

	if (message->what == B_MOUSE_UP) {
		if (fMiddleButtonPressed) {
			fMiddleButtonPressed = false;
			fScrolling = false;
			return B_SKIP_MESSAGE;
		}

		fScrolling = false;
		return B_DISPATCH_MESSAGE;
	}

	int32 buttons;
	if (message->FindInt32("buttons", &buttons) != B_OK) {
		fScrolling = false;
		return B_DISPATCH_MESSAGE;
	}

	if (message->what == B_MOUSE_DOWN) {
		if ((buttons & B_TERTIARY_MOUSE_BUTTON) == 0) {
			fScrolling = false;
			fMiddleButtonPressed = false;
			return B_DISPATCH_MESSAGE;
		}

		BPoint mousePosition;
		if (message->FindPoint("where", &mousePosition) != B_OK)
			return B_DISPATCH_MESSAGE;

		fMiddleButtonPressed = true;
		fScrolling = true;
		fPreviousMousePosition = mousePosition;
		return B_SKIP_MESSAGE;
	}

	if (!fScrolling)
		return B_DISPATCH_MESSAGE;

	BPoint mousePosition;
	if (message->FindPoint("where", &mousePosition) != B_OK)
		return B_DISPATCH_MESSAGE;

	if ((buttons & B_TERTIARY_MOUSE_BUTTON) == 0) {
		fScrolling = false;
		fMiddleButtonPressed = false;
		return B_DISPATCH_MESSAGE;
	}

	float deltaX = (mousePosition.x - fPreviousMousePosition.x) * kScrollScale;
	float deltaY = (mousePosition.y - fPreviousMousePosition.y) * kScrollScale;
	fPreviousMousePosition = mousePosition;
	if (deltaX > -kScrollThreshold && deltaX < kScrollThreshold
		&& deltaY > -kScrollThreshold && deltaY < kScrollThreshold)
		return B_DISPATCH_MESSAGE;

	message->what = B_MOUSE_WHEEL_CHANGED;
	message->AddFloat("be:wheel_delta_x", deltaX);
	message->AddFloat("be:wheel_delta_y", deltaY);
	message->RemoveName("buttons");
	message->RemoveName("clicks");
	message->RemoveName("transit");
	return B_DISPATCH_MESSAGE;
}

extern "C" BInputServerFilter*
instantiate_input_filter()
{
	return new(std::nothrow) MiddleButtonScrollFilter();
}
