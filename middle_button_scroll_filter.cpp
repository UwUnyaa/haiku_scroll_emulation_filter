#include <InputServerFilter.h>
#include <Message.h>
#include <Point.h>
#include <new>

class MiddleButtonScrollFilter : public BInputServerFilter {
private:
	bool fScrolling;
	BPoint fPreviousMousePosition;

public:
	MiddleButtonScrollFilter();
	virtual status_t InitCheck();
	virtual filter_result Filter(BMessage* message, BList* outList);
};

MiddleButtonScrollFilter::MiddleButtonScrollFilter()
	:	fScrolling(false)
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
		fScrolling = false;
		return B_DISPATCH_MESSAGE;
	}

	int32 buttons;
	if (message->FindInt32("buttons", &buttons) != B_OK) {
		fScrolling = false;
		return B_DISPATCH_MESSAGE;
	}

	if (message->what == B_MOUSE_DOWN) {
		if (buttons != B_TERTIARY_MOUSE_BUTTON) {
			fScrolling = false;
			return B_DISPATCH_MESSAGE;
		}

		BPoint mousePosition;
		if (message->FindPoint("where", &mousePosition) != B_OK)
			return B_DISPATCH_MESSAGE;

		fScrolling = true;
		fPreviousMousePosition = mousePosition;
		return B_DISPATCH_MESSAGE;
	}

	if (!fScrolling)
		return B_DISPATCH_MESSAGE;

	BPoint mousePosition;
	if (message->FindPoint("where", &mousePosition) != B_OK)
		return B_DISPATCH_MESSAGE;

	if ((buttons & B_TERTIARY_MOUSE_BUTTON) == 0) {
		fScrolling = false;
		return B_DISPATCH_MESSAGE;
	}

	float deltaX = mousePosition.x - fPreviousMousePosition.x;
	float deltaY = mousePosition.y - fPreviousMousePosition.y;
	fPreviousMousePosition = mousePosition;

	message->what = B_MOUSE_WHEEL_CHANGED;
	message->RemoveName("buttons");
	message->RemoveName("clicks");
	message->RemoveName("transit");
	message->RemoveName("be:screen_where");
	message->AddFloat("be:wheel_delta_x", deltaX);
	message->AddFloat("be:wheel_delta_y", deltaY);
	return B_DISPATCH_MESSAGE;
}

extern "C" BInputServerFilter*
instantiate_input_filter()
{
	return new(std::nothrow) MiddleButtonScrollFilter();
}
