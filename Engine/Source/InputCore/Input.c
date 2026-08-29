#include "Input.h"
#include "Core/Misc/CoreEvent.h"
#include "Core/HAL/LumoraMemory.h"
#include "Core/Logger.h"
#include "Core/Asserts.h"

typedef struct FKeyboardState
{
    bool8 Keys[256];
} FKeyboardState;

typedef struct FMouseState
{
    int16 X;
    int16 Y;
    uint8 Buttons[BUTTON_MAX_BUTTONS];
} FMouseState;

typedef struct FInputState
{
    FKeyboardState CurKeyState;
    FKeyboardState PrevKeyState;
    FMouseState CurMouse;
    FMouseState PrevMouse;
} FInputState;

/** Internal input state */
static FInputState* GInputState = NULL;

void InitializeInput(size_t* MemoryRequirement, void* State)
{
    *MemoryRequirement = sizeof(FInputState);
    if (State == NULL)
    {
        return;
    }

    HZeroMemory(State, sizeof(FInputState));
    GInputState = State;
}

void ReleaseInput(void* State)
{
    LUMORA_UNUSED_PARAM(State);

    /** TODO: Add release routines when needed. */
    GInputState = NULL;
}

void UpdateInput(float64 DeltaTime)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");

    /** Copy current states to previous states. */
    HCopyMemory(&GInputState->PrevKeyState, &GInputState->CurKeyState, sizeof(FKeyboardState));
    HCopyMemory(&GInputState->PrevMouse, &GInputState->CurMouse, sizeof(FMouseState));
}

LUMORA_C_API bool8 IsInputKeyDown(EKeys Key)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    return GInputState->CurKeyState.Keys[Key] == TRUE;
}

LUMORA_C_API bool8 IsInputKeyUp(EKeys Key)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    return GInputState->CurKeyState.Keys[Key] == FALSE;
}

LUMORA_C_API bool8 WasInputKeyDown(EKeys Key)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    return GInputState->PrevKeyState.Keys[Key] == TRUE;
}

LUMORA_C_API bool8 WasInputKeyUp(EKeys Key)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    return GInputState->PrevKeyState.Keys[Key] == FALSE;
}

void ProcessInputKey(EKeys Key, bool8 bPressed)
{

    /** Only handle this if the state actually changed. */
    if (GInputState->CurKeyState.Keys[Key] != bPressed)
    {
        /** Update internal state. */
        GInputState->CurKeyState.Keys[Key] = bPressed;

        /** Fire off an event for immediate processing. */
        FCoreEventContext EventContext = { 0 };
        EventContext.Data.U16[0] = Key;

        const uint16 EventCode = bPressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED;
        FireEvent(EventCode , 0, EventContext);
    }
}

/** Mouse input */
LUMORA_C_API bool8 IsInputButtonDown(EButtons Button)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    return GInputState->CurMouse.Buttons[Button] == TRUE;
}

LUMORA_C_API bool8 IsInputButtonUp(EButtons Button)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    return GInputState->CurMouse.Buttons[Button] == FALSE;
}

LUMORA_C_API bool8 WasInputButtonDown(EButtons Button)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    return GInputState->PrevMouse.Buttons[Button] == TRUE;
}

LUMORA_C_API bool8 WasInputButtonUp(EButtons Button)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    return GInputState->PrevMouse.Buttons[Button] == FALSE;
}

LUMORA_C_API void GetInputMousePosition(int32 *X, int32 *Y)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    *X = GInputState->CurMouse.X;
    *Y = GInputState->CurMouse.Y;
}

LUMORA_C_API void GetPrevInputMousePosition(int32 *X, int32 *Y)
{
    LUMORA_ASSERT_MSG(GInputState, "Input subsystem is not initialized.");
    *X = GInputState->PrevMouse.X;
    *Y = GInputState->PrevMouse.Y;
}

void ProcessInputButton(EButtons Button, bool8 bPressed)
{
    /** If the state changed, fire an event. */ 
    if (GInputState->CurMouse.Buttons[Button] != bPressed)
    {
        GInputState->CurMouse.Buttons[Button] = bPressed;

        /** Fire the event. */
        FCoreEventContext EventContext = { 0 };
        EventContext.Data.U16[0] = Button;

        const uint16 EventCode = bPressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED;
        FireEvent(EventCode, 0, EventContext); 
    }
}

void ProcessInputMouseMove(int16 X, int16 Y)
{
    /** Only process if actually different */
    if (GInputState->CurMouse.X != X || GInputState->CurMouse.Y != Y)
    {
        /** NOTE: Enable this if debugging. */ 
        // LUMORA_DEBUG("Mouse position: (%i, %i)", X, Y);

        /** Update internal state. */
        GInputState->CurMouse.X = X;
        GInputState->CurMouse.Y = Y;

        /** Fire the event. */
        FCoreEventContext EventContext = { 0 };
        EventContext.Data.U16[0] = X;
        EventContext.Data.U16[1] = Y;
        FireEvent(EVENT_CODE_MOUSE_MOVED, 0, EventContext);
    }
}

void ProcessInputMouseWheel(int8 DeltaZ)
{
    /** NOTE: no internal state to update */

    /** Fire the event. */
    FCoreEventContext EventContext = { 0 };
    EventContext.Data.U8[0] = DeltaZ;
    FireEvent(EVENT_CODE_MOUSE_WHEEL, 0, EventContext);
}

LUMORA_C_API const char* GetKeyName(EKeys Key)
{
    switch (Key)
    {
    case KEY_BACKSPACE: return "Backspace";
    case KEY_ENTER:     return "Enter";
    case KEY_TAB:       return "Tab";
    case KEY_SHIFT:     return "Shift";
    case KEY_CONTROL:   return "Control";

    case KEY_PAUSE:     return "Pause";
    case KEY_CAPITAL:   return "Caps Lock";

    case KEY_ESCAPE:    return "Escape";

    case KEY_CONVERT:    return "Convert";
    case KEY_NONCONVERT: return "NonConvert";
    case KEY_ACCEPT:     return "Accept";
    case KEY_MODECHANGE: return "ModeChange";

    case KEY_SPACE:    return "Space";
    case KEY_PRIOR:    return "Page Up";
    case KEY_NEXT:     return "Page Down";
    case KEY_END:      return "End";
    case KEY_HOME:     return "Home";
    case KEY_LEFT:     return "Left";
    case KEY_UP:       return "Up";
    case KEY_RIGHT:    return "Right";
    case KEY_DOWN:     return "Down";
    case KEY_SELECT:   return "Select";
    case KEY_PRINT:    return "Print";
    case KEY_EXECUTE:  return "Execute";
    case KEY_SNAPSHOT: return "Print Screen";
    case KEY_INSERT:   return "Insert";
    case KEY_DELETE:   return "Delete";
    case KEY_HELP:     return "Help";

    case KEY_A: return "A";
    case KEY_B: return "B";
    case KEY_C: return "C";
    case KEY_D: return "D";
    case KEY_E: return "E";
    case KEY_F: return "F";
    case KEY_G: return "G";
    case KEY_H: return "H";
    case KEY_I: return "I";
    case KEY_J: return "J";
    case KEY_K: return "K";
    case KEY_L: return "L";
    case KEY_M: return "M";
    case KEY_N: return "N";
    case KEY_O: return "O";
    case KEY_P: return "P";
    case KEY_Q: return "Q";
    case KEY_R: return "R";
    case KEY_S: return "S";
    case KEY_T: return "T";
    case KEY_U: return "U";
    case KEY_V: return "V";
    case KEY_W: return "W";
    case KEY_X: return "X";
    case KEY_Y: return "Y";
    case KEY_Z: return "Z";

    case KEY_LWIN: return "Left Windows";
    case KEY_RWIN: return "Right Windows";
    case KEY_APPS: return "Application";
    case KEY_SLEEP: return "Sleep";

    case KEY_NUMPAD_ZERO:  return "Numpad 0";
    case KEY_NUMPAD_ONE:   return "Numpad 1";
    case KEY_NUMPAD_TWO:   return "Numpad 2";
    case KEY_NUMPAD_THREE: return "Numpad 3";
    case KEY_NUMPAD_FOUR:  return "Numpad 4";
    case KEY_NUMPAD_FIVE:  return "Numpad 5";
    case KEY_NUMPAD_SIX:   return "Numpad 6";
    case KEY_NUMPAD_SEVEN: return "Numpad 7";
    case KEY_NUMPAD_EIGHT: return "Numpad 8";
    case KEY_NUMPAD_NINE:  return "Numpad 9";

    case KEY_MULTIPLY:  return "Numpad Multiply";
    case KEY_ADD:       return "Numpad Add";
    case KEY_SEPARATOR: return "Numpad Separator";
    case KEY_SUBTRACT:  return "Numpad Subtract";
    case KEY_DECIMAL:   return "Numpad Decimal";
    case KEY_DIVIDE:    return "Numpad Divide";

    case KEY_F1:  return "F1";
    case KEY_F2:  return "F2";
    case KEY_F3:  return "F3";
    case KEY_F4:  return "F4";
    case KEY_F5:  return "F5";
    case KEY_F6:  return "F6";
    case KEY_F7:  return "F7";
    case KEY_F8:  return "F8";
    case KEY_F9:  return "F9";
    case KEY_F10: return "F10";
    case KEY_F11: return "F11";
    case KEY_F12: return "F12";
    case KEY_F13: return "F13";
    case KEY_F14: return "F14";
    case KEY_F15: return "F15";
    case KEY_F16: return "F16";
    case KEY_F17: return "F17";
    case KEY_F18: return "F18";
    case KEY_F19: return "F19";
    case KEY_F20: return "F20";
    case KEY_F21: return "F21";
    case KEY_F22: return "F22";
    case KEY_F23: return "F23";
    case KEY_F24: return "F24";

    case KEY_NUMLOCK:      return "Num Lock";
    case KEY_SCROLL:       return "Scroll Lock";
    case KEY_NUMPAD_EQUAL: return "Numpad Equal";

    case KEY_LSHIFT:   return "Left Shift";
    case KEY_RSHIFT:   return "Right Shift";
    case KEY_LCONTROL: return "Left Control";
    case KEY_RCONTROL: return "Right Control";
    case KEY_LALT:     return "Left Alt";
    case KEY_RALT:     return "Right Alt";

    default:
        return "Unknown";
    }
}