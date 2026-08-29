#pragma once

#include "Defines.h"

typedef enum EButtons
{
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_MIDDLE,
    BUTTON_MAX_BUTTONS,
} EButtons;

#define KEY(Name, Code) KEY_##Name = Code

typedef enum EKeys
{
    KEY(BACKSPACE, 0x08),
    KEY(ENTER    , 0x0D),
    KEY(TAB      , 0x09),
    KEY(SHIFT    , 0x10),
    KEY(CONTROL  , 0x11),

    KEY(PAUSE    , 0x13),
    KEY(CAPITAL  , 0x14),
    
    KEY(ESCAPE   , 0x1B),
    
    KEY(CONVERT   , 0x1C),
    KEY(NONCONVERT, 0x1D),
    KEY(ACCEPT    , 0x1E),
    KEY(MODECHANGE, 0x1F),

    KEY(SPACE   , 0x20),
    KEY(PRIOR   , 0x21),
    KEY(NEXT    , 0x22),
    KEY(END     , 0x23),
    KEY(HOME    , 0x24),
    KEY(LEFT    , 0x25),
    KEY(UP      , 0x26),
    KEY(RIGHT   , 0x27),
    KEY(DOWN    , 0x28),
    KEY(SELECT  , 0x29),
    KEY(PRINT   , 0x2A),
    KEY(EXECUTE , 0x2B),
    KEY(SNAPSHOT, 0x2C),
    KEY(INSERT  , 0x2D),
    KEY(DELETE  , 0x2E),
    KEY(HELP    , 0x2F),

    KEY(A, 0x41),
    KEY(B, 0x42),
    KEY(C, 0x43),
    KEY(D, 0x44),
    KEY(E, 0x45),
    KEY(F, 0x46),
    KEY(G, 0x47),
    KEY(H, 0x48),
    KEY(I, 0x49),
    KEY(J, 0x4A),
    KEY(K, 0x4B),
    KEY(L, 0x4C),
    KEY(M, 0x4D),
    KEY(N, 0x4E),
    KEY(O, 0x4F),
    KEY(P, 0x50),
    KEY(Q, 0x51),
    KEY(R, 0x52),
    KEY(S, 0x53),
    KEY(T, 0x54),
    KEY(U, 0x55),
    KEY(V, 0x56),
    KEY(W, 0x57),
    KEY(X, 0x58),
    KEY(Y, 0x59),
    KEY(Z, 0x5A),

    KEY(LWIN, 0x5B),
    KEY(RWIN, 0x5C),
    KEY(APPS, 0x5D),

    KEY(SLEEP, 0x5F),

    KEY(NUMPAD_ZERO , 0x60),
    KEY(NUMPAD_ONE  , 0x61),
    KEY(NUMPAD_TWO  , 0x62),
    KEY(NUMPAD_THREE, 0x63),
    KEY(NUMPAD_FOUR , 0x64),
    KEY(NUMPAD_FIVE , 0x65),
    KEY(NUMPAD_SIX  , 0x66),
    KEY(NUMPAD_SEVEN, 0x67),
    KEY(NUMPAD_EIGHT, 0x68),
    KEY(NUMPAD_NINE , 0x69),
    KEY(MULTIPLY    , 0x6A),
    KEY(ADD         , 0x6B),
    KEY(SEPARATOR   , 0x6C),
    KEY(SUBTRACT    , 0x6D),
    KEY(DECIMAL     , 0x6E),
    KEY(DIVIDE      , 0x6F),
    
    KEY(F1  , 0x70),
    KEY(F2  , 0x71),
    KEY(F3  , 0x72),
    KEY(F4  , 0x73),
    KEY(F5  , 0x74),
    KEY(F6  , 0x75),
    KEY(F7  , 0x76),
    KEY(F8  , 0x77),
    KEY(F9  , 0x78),
    KEY(F10 , 0x79),
    KEY(F11 , 0x7A),
    KEY(F12 , 0x7B),
    KEY(F13 , 0x7C),
    KEY(F14 , 0x7D),
    KEY(F15 , 0x7E),
    KEY(F16 , 0x7F),
    KEY(F17 , 0x80),
    KEY(F18 , 0x81),
    KEY(F19 , 0x82),
    KEY(F20 , 0x83),
    KEY(F21 , 0x84),
    KEY(F22 , 0x85),
    KEY(F23 , 0x86),
    KEY(F24 , 0x87),

    KEY(NUMLOCK, 0x90),
    KEY(SCROLL, 0x91),

    KEY(NUMPAD_EQUAL, 0x92),

    KEY(LSHIFT, 0xA0),
    KEY(RSHIFT, 0xA1),
    KEY(LCONTROL, 0xA2),
    KEY(RCONTROL, 0xA3),
    KEY(LALT, 0xA4),
    KEY(RALT, 0xA5),

    KEYS_MAX_KEYS,
} EKeys;

/**
 * @brief Initializes the input system. Call twice; once to obtain memory requirement (passing
 * state = 0), then a second time passing allocated memory to state.
 *
 * @param MemoryRequirement The required size of the state memory.
 * @param State Either 0 or the allocated block of state memory.
 */
void InitializeInput(size_t* MemoryRequirement, void* State);

void ReleaseInput(void* State);

/**
 * NOTE: UpdateInput Copies the current input state to the previous input state.
 * This should be called at the end of the frame, after all input
 * events have been recorded and processed, to prepare the input
 * system for the next frame.
 */
void UpdateInput(float64 DeltaTime);

/** Keyboard input */
LUMORA_C_API bool8 IsInputKeyDown(EKeys Key);
LUMORA_C_API bool8 IsInputKeyUp(EKeys Key);
LUMORA_C_API bool8 WasInputKeyDown(EKeys Key);
LUMORA_C_API bool8 WasInputKeyUp(EKeys Key);

void ProcessInputKey(EKeys Key, bool8 bPressed);

/** Mouse input */
LUMORA_C_API bool8 IsInputButtonDown(EButtons Button);
LUMORA_C_API bool8 IsInputButtonUp(EButtons Button);
LUMORA_C_API bool8 WasInputButtonDown(EButtons Button);
LUMORA_C_API bool8 WasInputButtonUp(EButtons Button);
LUMORA_C_API void GetInputMousePosition(int32* X, int32* Y);
LUMORA_C_API void GetPrevInputMousePosition(int32* X, int32* Y);

void ProcessInputButton(EButtons Button, bool8 bPressed);
void ProcessInputMouseMove(int16 X, int16 Y);
void ProcessInputMouseWheel(int8 DeltaZ);

LUMORA_C_API const char* GetKeyName(EKeys Key);