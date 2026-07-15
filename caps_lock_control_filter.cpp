#include <InputServerFilter.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <string.h>
#include <new>

class CapsLockControlFilter : public BInputServerFilter {
private:
	uint32 fControlKeyCode;

public:
	CapsLockControlFilter();
	virtual status_t InitCheck();
	virtual filter_result Filter(BMessage* message, BList* outList);
};

CapsLockControlFilter::CapsLockControlFilter()
	: fControlKeyCode(0)
{
}

status_t
CapsLockControlFilter::InitCheck()
{
	return get_modifier_key(B_LEFT_CONTROL_KEY, &fControlKeyCode);
}

static inline void
SetKeyState(uint8* states, uint32 key, bool pressed)
{
	uint32 byteIndex = key >> 3;
	uint8 mask = (uint8)(1 << (key & 7));
	if (pressed)
		states[byteIndex] |= mask;
	else
		states[byteIndex] &= (uint8)~mask;
}

filter_result
CapsLockControlFilter::Filter(BMessage* message, BList* outList)
{
	(void)outList;

	if (message->what != B_KEY_DOWN && message->what != B_KEY_UP)
		return B_DISPATCH_MESSAGE;

	int32 key;
	if (message->FindInt32("key", &key) != B_OK || key != B_CAPS_LOCK_KEY)
		return B_DISPATCH_MESSAGE;

	const bool isKeyDown = message->what == B_KEY_DOWN;
	if (fControlKeyCode != 0)
		message->ReplaceInt32("key", fControlKeyCode);

	int32 modifiers;
	if (message->FindInt32("modifiers", &modifiers) == B_OK) {
		modifiers &= ~B_CAPS_LOCK;
		if (isKeyDown)
			modifiers |= B_CONTROL_KEY;
		message->ReplaceInt32("modifiers", modifiers);
	}

	const void* statesData;
	ssize_t statesSize;
	if (message->FindData("states", B_UINT8_TYPE, &statesData, &statesSize) == B_OK
		&& statesSize == 16) {
		uint8 states[16];
		memcpy(states, statesData, sizeof(states));
		SetKeyState(states, B_CAPS_LOCK_KEY, false);
		if (fControlKeyCode != 0)
			SetKeyState(states, fControlKeyCode, isKeyDown);
		message->ReplaceData("states", B_UINT8_TYPE, states, sizeof(states));
	}

	return B_DISPATCH_MESSAGE;
}

extern "C" BInputServerFilter*
instantiate_input_filter()
{
	return new(std::nothrow) CapsLockControlFilter();
}
