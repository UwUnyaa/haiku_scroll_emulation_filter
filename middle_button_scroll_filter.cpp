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
	static constexpr float kScrollThreshold = 1.0f;
	float fAccumulatedDeltaX;
	float fAccumulatedDeltaY;

public:
	MiddleButtonScrollFilter();
	virtual status_t InitCheck();
	virtual filter_result Filter(BMessage* message, BList* outList);
};

MiddleButtonScrollFilter::MiddleButtonScrollFilter()
	:	fScrolling(false)
	,	fMiddleButtonPressed(false)
	,	fAccumulatedDeltaX(0.0f)
	,	fAccumulatedDeltaY(0.0f)
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
		int32 buttons = 0;
		message->FindInt32("buttons", &buttons);

		if (fMiddleButtonPressed && (buttons & B_TERTIARY_MOUSE_BUTTON) == 0) {
			fMiddleButtonPressed = false;
			fScrolling = false;
			fAccumulatedDeltaX = 0.0f;
			fAccumulatedDeltaY = 0.0f;
			return B_SKIP_MESSAGE;
		}

		if (!fMiddleButtonPressed)
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
			fAccumulatedDeltaX = 0.0f;
			fAccumulatedDeltaY = 0.0f;
			return B_DISPATCH_MESSAGE;
		}

		BPoint mousePosition;
		if (message->FindPoint("where", &mousePosition) != B_OK)
			return B_DISPATCH_MESSAGE;

		fMiddleButtonPressed = true;
		fScrolling = true;
		fPreviousMousePosition = mousePosition;
		fAccumulatedDeltaX = 0.0f;
		fAccumulatedDeltaY = 0.0f;
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
		fAccumulatedDeltaX = 0.0f;
		fAccumulatedDeltaY = 0.0f;
		return B_DISPATCH_MESSAGE;
	}

	float deltaX = mousePosition.x - fPreviousMousePosition.x;
	float deltaY = mousePosition.y - fPreviousMousePosition.y;
	fPreviousMousePosition = mousePosition;
	fAccumulatedDeltaX += deltaX;
	fAccumulatedDeltaY += deltaY;

	if (fAccumulatedDeltaX > -kScrollThreshold && fAccumulatedDeltaX < kScrollThreshold
		&& fAccumulatedDeltaY > -kScrollThreshold && fAccumulatedDeltaY < kScrollThreshold)
		return B_SKIP_MESSAGE;

	BMessage* wheelMessage = new(std::nothrow) BMessage(B_MOUSE_WHEEL_CHANGED);
	if (wheelMessage == NULL)
		return B_SKIP_MESSAGE;

	wheelMessage->AddFloat("be:wheel_delta_x", fAccumulatedDeltaX);
	wheelMessage->AddFloat("be:wheel_delta_y", fAccumulatedDeltaY);
	fAccumulatedDeltaX = 0.0f;
	fAccumulatedDeltaY = 0.0f;
	outList->AddItem(wheelMessage);
	return B_SKIP_MESSAGE;
}

extern "C" BInputServerFilter*
instantiate_input_filter()
{
	return new(std::nothrow) MiddleButtonScrollFilter();
}
