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
static bool8 bInitialized = FALSE;
static FInputState GInputState = {};

void InitializeInput(void)
{
    HZeroMemory(&GInputState, sizeof(FInputState));
    bInitialized = TRUE;
    LUMORA_INFO("Input subsytem initialized.");
}

void ReleaseInput(void)
{
    /** TODO: Add release routines when needed. */
    bInitialized = FALSE;
}

void UpdateInput(float64 DeltaTime)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");

    /** Copy current states to previous states. */
    HCopyMemory(&GInputState.PrevKeyState, &GInputState.CurKeyState, sizeof(FKeyboardState));
    HCopyMemory(&GInputState.PrevMouse, &GInputState.CurMouse, sizeof(FMouseState));
}

LUMORA_C_API bool8 IsInputKeyDown(EKeys Key)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    return GInputState.CurKeyState.Keys[Key] == TRUE;
}

LUMORA_C_API bool8 IsInputKeyUp(EKeys Key)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    return GInputState.CurKeyState.Keys[Key] == FALSE;
}

LUMORA_C_API bool8 WasInputKeyDown(EKeys Key)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    return GInputState.PrevKeyState.Keys[Key] == TRUE;
}

LUMORA_C_API bool8 WasInputKeyUp(EKeys Key)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    return GInputState.PrevKeyState.Keys[Key] == FALSE;
}

void ProcessInputKey(EKeys Key, bool8 bPressed)
{
    /** Only handle this if the state actually changed. */
    if (GInputState.CurKeyState.Keys[Key] != bPressed)
    {
        /** Update internal state. */
        GInputState.CurKeyState.Keys[Key] = bPressed;

        /** Fire off an event for immediate processing. */
        FCoreEventContext EventContext = {};
        EventContext.Data.U16[0] = Key;

        const uint16 EventCode = bPressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED;
        FireEvent(EventCode , 0, EventContext);
    }
}

/** Mouse input */
LUMORA_C_API bool8 IsInputButtonDown(EButtons Button)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    return GInputState.CurMouse.Buttons[Button] == TRUE;
}

LUMORA_C_API bool8 IsInputButtonUp(EButtons Button)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    return GInputState.CurMouse.Buttons[Button] == FALSE;
}

LUMORA_C_API bool8 WasInputButtonDown(EButtons Button)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    return GInputState.PrevMouse.Buttons[Button] == TRUE;
}

LUMORA_C_API bool8 WasInputButtonUp(EButtons Button)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    return GInputState.PrevMouse.Buttons[Button] == FALSE;
}

LUMORA_C_API void GetInputMousePosition(int32 *X, int32 *Y)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    *X = GInputState.CurMouse.X;
    *Y = GInputState.CurMouse.Y;
}

LUMORA_C_API void GetPrevInputMousePosition(int32 *X, int32 *Y)
{
    LUMORA_ASSERT_MSG(bInitialized, "Input subsystem is not initialized.");
    *X = GInputState.PrevMouse.X;
    *Y = GInputState.PrevMouse.Y;
}

void ProcessInputButton(EButtons Button, bool8 bPressed)
{
    /** If the state changed, fire an event. */ 
    if (GInputState.CurMouse.Buttons[Button] != bPressed)
    {
        GInputState.CurMouse.Buttons[Button] = bPressed;

        /** Fire the event. */
        FCoreEventContext EventContext = {};
        EventContext.Data.U16[0] = Button;

        const uint16 EventCode = bPressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED;
        FireEvent(EventCode, 0, EventContext); 
    }
}

void ProcessInputMouseMove(int16 X, int16 Y)
{
    /** Only process if actually different */
    if (GInputState.CurMouse.X != X || GInputState.CurMouse.Y != Y)
    {
        /** NOTE: Enable this if debugging. */ 
        LUMORA_DEBUG("Mouse position: (%i, %i)", X, Y);

        /** Update internal state. */
        GInputState.CurMouse.X = X;
        GInputState.CurMouse.Y = Y;

        /** Fire the event. */
        FCoreEventContext EventContext = {};
        EventContext.Data.U16[0] = X;
        EventContext.Data.U16[1] = Y;
        FireEvent(EVENT_CODE_MOUSE_MOVED, 0, EventContext);
    }
}

void ProcessInputMouseWheel(int8 DeltaZ)
{
    /** NOTE: no internal state to update */

    /** Fire the event. */
    FCoreEventContext EventContext = {};
    EventContext.Data.U8[0] = DeltaZ;
    FireEvent(EVENT_CODE_MOUSE_WHEEL, 0, EventContext);
}
