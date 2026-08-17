#include "Core/Platform/Platform.h"

/** Linux platform layer. */
#if PLATFORM_LINUX
#define _POSIX_C_SOURCE 200809L

#include "Core/Logger.h"
#include "Core/Misc/CoreEvent.h"
#include "InputCore/Input.h"
#include "Core/Misc/CString.h"
#include "Core/Containers/Array.h"

#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <sys/time.h>

#if _POSIX_C_SOURCE >= 199309L
    /** nanosleep */
    #include <time.h>
#else
    /** usleep */
    #include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** For surface creation */
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include "Renderer/Vulkan/VulkanTypes.inl"


typedef struct FInternalState
{
    Display* Display;
    xcb_connection_t* Connection;
    xcb_window_t Window;
    xcb_screen_t* Screen;
    xcb_atom_t wmProtocols;
    xcb_atom_t wmDeleteWindow;
    VkSurfaceKHR Surface;
} FInternalState;

static FORCEINLINE EKeys TranslateKeyCode(uint32 xKeyCode);
static FORCEINLINE void PlatformConsoleWriteImpl(const char* Message, uint8 Color);

LUMORA_C_API bool8 PlatformStartup(FPlatformState* PlatformState, const char* ApplicationName, int32 X, int32 Y, int32 Width, int32 Height)
{
    /** Create the internal state. */
    FInternalState* InternalState = malloc(sizeof(FInternalState));
    PlatformState->InternalState = InternalState;

    /** Connect to X */
    InternalState->Display = XOpenDisplay(NULL);

    /** Turn off key repeats. */
    XAutoRepeatOff(InternalState->Display);

    /** Retrieve the connection from the display. */
    InternalState->Connection = XGetXCBConnection(InternalState->Display);

    if (xcb_connection_has_error(InternalState->Connection))
    {
        LUMORA_FATAL("Failed to connect to X server via XCB.");
        return FALSE;
    }

    /** Get date from the X server */
    const struct xcb_setup_t* Setup = xcb_get_setup(InternalState->Connection);

    /** Loop through screens using iterator */
    xcb_screen_iterator_t Iterator = xcb_setup_roots_iterator(Setup);
    int ScreenP = 0;
    for (int32 Screen = ScreenP; Screen > 0; --Screen)
    {
        xcb_screen_next(&Iterator);
    }

    /** After screens have been looped through, assign it. */
    InternalState->Screen = Iterator.data;

    /** Allocate a XID for the window to be created. */
    InternalState->Window = xcb_generate_id(InternalState->Connection);

    /**
     * Registter event types.
     * XCB_CW_BACK_PIXEL = filling then window bg with a single colour
     * XCB_CW_EVENT_MASK is required.
     */
    uint32 EventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    /** Listen for keyboard and mouse buttons */
    uint32 EventValues = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                         XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
                         XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION | 
                         XCB_EVENT_MASK_STRUCTURE_NOTIFY;
    
    /** Values to be sent over XCB (bg colour, events) */
    uint32 ValueList[] = { InternalState->Screen->black_pixel, EventValues };

    /** Create the window */
    xcb_void_cookie_t Cookie = xcb_create_window(
        InternalState->Connection,
        XCB_COPY_FROM_PARENT,           // depth
        InternalState->Window,
        InternalState->Screen->root,    // parent
        X,                              // x
        Y,                              // y
        Width,                          // width
        Height,                         // height
        0,                              // No border
        XCB_WINDOW_CLASS_INPUT_OUTPUT,  // class
        InternalState->Screen->root_visual,
        EventMask,
        ValueList
    );

    /** Change the title */
    xcb_change_property(
        InternalState->Connection,
        XCB_PROP_MODE_REPLACE,
        InternalState->Window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,                              // data should be viewed 8 bits at a time
        Strlen(ApplicationName),
        ApplicationName
    );

    /** 
     * Tell the server to notify when the window manager
     * attempts to destroy the window.
     */
    xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(
        InternalState->Connection, 
        0, 
        Strlen("WM_DELETE_WINDOW"), 
        "WM_DELETE_WINDOW"
    );
    xcb_intern_atom_cookie_t wmProtocolsCookie = xcb_intern_atom(
        InternalState->Connection, 
        0, 
        Strlen("WM_PROTOCOLS"), 
        "WM_PROTOCOLS"
    );
    xcb_intern_atom_reply_t* wmDeleteReplay = xcb_intern_atom_reply(
        InternalState->Connection, 
        wmDeleteCookie, 
        NULL
    );
    xcb_intern_atom_reply_t* wmProcotolsReplay = xcb_intern_atom_reply(
        InternalState->Connection,
        wmProtocolsCookie,
        NULL
    );
    InternalState->wmDeleteWindow = wmDeleteReplay->atom;
    InternalState->wmProtocols = wmProcotolsReplay->atom;

    xcb_change_property(
        InternalState->Connection,
        XCB_PROP_MODE_REPLACE,
        InternalState->Window,
        wmProcotolsReplay->atom,
        4,
        32,
        1,
        &wmDeleteReplay->atom
    );

    /** Map the window to the screen */
    xcb_map_window(InternalState->Connection, InternalState->Window);

    int32 StreamResult = xcb_flush(InternalState->Connection);
    if (StreamResult <= 0)
    {
        LUMORA_FATAL("An error occurred when flusing the stream: %d", StreamResult);
        return FALSE;
    }

    return TRUE;
}

LUMORA_C_API void PlatformShutdown(FPlatformState* PlatformState)
{
    /** Simply cold-cast to the known type. */
    FInternalState* InternalState = (FInternalState*)PlatformState->InternalState;

    /** Turn key repeat back on since this is global for the OS... just... wow. */
    XAutoRepeatOn(InternalState->Display);

    xcb_destroy_window(InternalState->Connection, InternalState->Window);
}

LUMORA_C_API bool8 PlatformPumpMessage(FPlatformState* PlatformState)
{
    /** Simply cold-cast to the known type. */
    FInternalState* InternalState = (FInternalState*)PlatformState->InternalState;

    xcb_generic_event_t* Event;
    xcb_client_message_event_t* LocalClientMessage;

    bool8 QuitFlagged = FALSE;

    /** Poll for event until null is returned. */
    while (Event != NULL)
    {
        Event = xcb_poll_for_event(InternalState->Connection);
        if (Event == NULL)
        {
            break;
        }

        /** Input events */
        switch (Event->response_type & ~0x80)
        {
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE:
        {
            /** Key press event - xcb_key_press_event_t and xcb_key_release_event_t are the same */
            xcb_key_press_event_t* KeyPressEvent = (xcb_key_press_event_t*)Event;
            bool8 bPressed = Event->response_type == XCB_KEY_PRESS;

            xcb_keycode_t Code = KeyPressEvent->detail;
            KeySym KeySystem = XkbKeycodeToKeysym(InternalState->Display, (KeyCode)Code, 0, Code & ShiftMask ? 1 : 0);
            EKeys Key = TranslateKeyCode(KeySystem);

            /** Pass to the input subsysteem for processing. */
            ProcessInputKey(Key, bPressed);
        }
            break;
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
        {
            xcb_button_press_event_t* MouseEvent = (xcb_button_press_event_t*)Event;
            bool8 bPressed = Event->response_type == XCB_BUTTON_PRESS;
            EButtons MouseButton = BUTTON_MAX_BUTTONS;
            switch (MouseEvent->detail)
            {
            case XCB_BUTTON_INDEX_1:
                MouseButton = BUTTON_LEFT;
                break;
            case XCB_BUTTON_INDEX_2:
                MouseButton = BUTTON_MIDDLE;
                break;
            case XCB_BUTTON_INDEX_3:
                MouseButton = BUTTON_RIGHT;
                break;
            }

            /** Pass over to the input subsystem. */
            if (MouseButton != BUTTON_MAX_BUTTONS)
            {
                ProcessInputButton(MouseButton, bPressed);
            }
        }
            break;
        case XCB_MOTION_NOTIFY:
        {
            /** Mouse move */
            xcb_motion_notify_event_t* MouseEvent = (xcb_motion_notify_event_t*)Event;
            ProcessInputMouseMove(MouseEvent->event_x, MouseEvent->event_y);
        }
            break;
        case XCB_CONFIGURE_NOTIFY:
        {
            /** TODO: Resizing */
        }
            break;
        case XCB_CLIENT_MESSAGE:
        {
            LocalClientMessage = (xcb_client_message_event_t*)Event;

            /** Window close */
            if (LocalClientMessage->data.data32[0] == InternalState->wmDeleteWindow)
            {
                QuitFlagged = TRUE;
            }
        }
            break;
        default:
            /** Something else */
            break;
        }

        free(Event);
    }

    return !QuitFlagged;
}

LUMORA_C_API void* PlatformAllocate(uint64 AllocSize, bool8 bAligned)
{
    return malloc(AllocSize);
}

LUMORA_C_API void  PlatformFree(void* Block, bool8 bAligned)
{
    free(Block);
}

LUMORA_C_API void* PlatformZeroMemory(void* Block, uint64 AllocSize)
{
    return memset(Block,  0, AllocSize);
}

LUMORA_C_API void* PlatformCopyMemory(void* Dest, const void* Src, uint64 AllocSize)
{
    return memcpy(Dest, Src, AllocSize);
}

LUMORA_C_API void* PlatformSetMemory(void* Dest, int32 Value, uint64 AllocSize)
{
    return memset(Dest, Value, AllocSize);
}

LUMORA_C_API void PlatformConsoleWrite(const char* Message, uint8 Color)
{
    PlatformConsoleWriteImpl(Message, Color);
}

LUMORA_C_API void PlatformConsoleWriteError(const char* Message, uint8 Color)
{
    PlatformConsoleWriteImpl(Message, Color);
}

static FORCEINLINE void PlatformConsoleWriteImpl(const char* Message, uint8 Color)
{
    /** FATAL, ERROR, WARN, INFO, DEBUG, TRACE */
    const char* ColourStrings[] = { "0;41", "1;31", "1;33", "1;32", "1;34", "1;30" };
    printf("\033[%sm%s\033[0m", ColourStrings[Color], Message);
}

LUMORA_C_API float64 PlatformGetAbsoluteTime(void)
{
    struct timespec LocalCurrentTime;
    clock_gettime(CLOCK_MONOTONIC, &LocalCurrentTime);
    return LocalCurrentTime.tv_sec + LocalCurrentTime.tv_nsec * 0.000000001;
}

LUMORA_C_API void PlatformSleep(uint64 MilliSecond)
{
#if _POSIX_C_SOURCE >= 199309L
    struct timespec TimeSpec;
    TimeSpec.tv_sec = MilliSecond / 1000;
    TimeSpec.tv_nsec = (MilliSecond % 1000) * 1000 * 1000;
    nanosleep(&TimeSpec, 0);
#else
    if (MilliSecond >= 1000)
    {
        sleep(MilliSecond / 1000);
    }

    // usleep((MilliSecond % 1000) * 1000);
#endif
}

void PlatformGetRequiredExtensionNames(const char*** CArrayNames)
{
    const char* LinuxSurface = "VK_KHR_xcb_surface"; 
    CArrayPush(*CArrayNames, &LinuxSurface);    // VK_KHR_xlib_surface?
}

/** Surface createion for Vulkan. */
bool8 PlatformCreateVulkanSurface(struct FPlatformState* PlatformState, struct FVulkanContext* VulkanContext)
{
    /** Simply cold-cast to the known type. */
    FInternalState* InternalState = (FInternalState*)PlatformState->InternalState;

    VkXcbSurfaceCreateInfoKHR CreateInfo = { VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
    CreateInfo.connection = InternalState->Connection;
    CreateInfo.window = InternalState->Window;

    VkResult Result = vkCreateXcbSurfaceKHR(
        VulkanContext->Instance,
        &CreateInfo,
        VulkanContext->Allocator,
        &InternalState->Surface
    );
    if (Result != VK_SUCCESS)
    {
        LUMORA_FATAL("Vulkan surface creation failed.");
        return FALSE;
    }

    VulkanContext->Surface = InternalState->Surface;
    return TRUE;
}

#define KEY_RETURN(XCBCode, LumoraCode) case XCBCode: return LumoraCode
#define KEYS_RETURN(XCBCode1, XCBCode2, LumoraCode) case XCBCode1: case XCBCode2: return LumoraCode

/** Key translateion */
FORCEINLINE EKeys TranslateKeyCode(uint32 xKeyCode)
{
    switch (xKeyCode)
    {
    KEY_RETURN(XK_BackSpace, KEY_BACKSPACE);
    KEY_RETURN(XK_Return   , KEY_ENTER);
	KEY_RETURN(XK_Tab      , KEY_TAB);
	KEY_RETURN(XK_Pause    , KEY_PAUSE);
	KEY_RETURN(XK_Caps_Lock, KEY_CAPITAL);
	KEY_RETURN(XK_Escape   , KEY_ESCAPE);

    // Not supported
    // case : return KEY_CONVERT;
    // case : return KEY_NONCONVERT;
    // case : return KEY_ACCEPT;

	KEY_RETURN(XK_Mode_switch, KEY_MODECHANGE);
	KEY_RETURN(XK_space      , KEY_SPACE);
	KEY_RETURN(XK_End        , KEY_END);
	KEY_RETURN(XK_Home       , KEY_HOME);
	KEY_RETURN(XK_Left       , KEY_LEFT);
	KEY_RETURN(XK_Up         , KEY_UP);
	KEY_RETURN(XK_Right      , KEY_RIGHT);
	KEY_RETURN(XK_Down       , KEY_DOWN);
	KEY_RETURN(XK_Select     , KEY_SELECT);
	KEY_RETURN(XK_Print      , KEY_PRINT);
	KEY_RETURN(XK_Execute    , KEY_EXECUTE);

	// case XK_snapshot: return KEY_SNAPSHOT; // not supported
	KEY_RETURN(XK_Insert      , KEY_INSERT);
	KEY_RETURN(XK_Delete      , KEY_DELETE);
	KEY_RETURN(XK_Help        , KEY_HELP);
	KEY_RETURN(XK_multiply    , KEY_MULTIPLY);
	KEY_RETURN(XK_KP_Add      , KEY_ADD);
	KEY_RETURN(XK_KP_Separator, KEY_SEPARATOR);
	KEY_RETURN(XK_KP_Subtract , KEY_SUBTRACT);
	KEY_RETURN(XK_KP_Decimal  , KEY_DECIMAL);
	KEY_RETURN(XK_KP_Divide   , KEY_DIVIDE);

	KEY_RETURN(XK_F1 , KEY_F1);
	KEY_RETURN(XK_F2 , KEY_F2);
	KEY_RETURN(XK_F3 , KEY_F3);
	KEY_RETURN(XK_F4 , KEY_F4);
	KEY_RETURN(XK_F5 , KEY_F5);
	KEY_RETURN(XK_F6 , KEY_F6);
	KEY_RETURN(XK_F7 , KEY_F7);
	KEY_RETURN(XK_F8 , KEY_F8);
	KEY_RETURN(XK_F9 , KEY_F9);
	KEY_RETURN(XK_F10, KEY_F10);
	KEY_RETURN(XK_F11, KEY_F11);
	KEY_RETURN(XK_F12, KEY_F12);
	KEY_RETURN(XK_F13, KEY_F13);
	KEY_RETURN(XK_F14, KEY_F14);
	KEY_RETURN(XK_F15, KEY_F15);
	KEY_RETURN(XK_F16, KEY_F16);
	KEY_RETURN(XK_F17, KEY_F17);
	KEY_RETURN(XK_F18, KEY_F18);
	KEY_RETURN(XK_F19, KEY_F19);
	KEY_RETURN(XK_F20, KEY_F20);
	KEY_RETURN(XK_F21, KEY_F21);
	KEY_RETURN(XK_F22, KEY_F22);
	KEY_RETURN(XK_F23, KEY_F23);
	KEY_RETURN(XK_F24, KEY_F24);

	KEY_RETURN(XK_Num_Lock   ,  KEY_NUMLOCK);
	KEY_RETURN(XK_Scroll_Lock, KEY_SCROLL);
	KEY_RETURN(XK_KP_Equal   , KEY_NUMPAD_EQUAL);

	KEY_RETURN(XK_Shift_L  , KEY_LSHIFT);
	KEY_RETURN(XK_Shift_R  , KEY_RSHIFT);
	KEY_RETURN(XK_Control_L, KEY_LCONTROL);
	KEY_RETURN(XK_Control_R, KEY_RCONTROL);

	KEY_RETURN(XK_0, KEY_NUMPAD_ZERO);
	KEY_RETURN(XK_1, KEY_NUMPAD_ONE);
	KEY_RETURN(XK_2, KEY_NUMPAD_TWO);
	KEY_RETURN(XK_3, KEY_NUMPAD_THREE);
	KEY_RETURN(XK_4, KEY_NUMPAD_FOUR);
	KEY_RETURN(XK_5, KEY_NUMPAD_FIVE);
	KEY_RETURN(XK_6, KEY_NUMPAD_SIX);
	KEY_RETURN(XK_7, KEY_NUMPAD_SEVEN);
	KEY_RETURN(XK_8, KEY_NUMPAD_EIGHT);
	KEY_RETURN(XK_9, KEY_NUMPAD_NINE);

	KEYS_RETURN(XK_a, XK_A, KEY_A);
	KEYS_RETURN(XK_b, XK_B, KEY_B);
	KEYS_RETURN(XK_c, XK_C, KEY_C);
	KEYS_RETURN(XK_d, XK_D, KEY_D);
	KEYS_RETURN(XK_e, XK_E, KEY_E);
	KEYS_RETURN(XK_f, XK_F, KEY_F);
	KEYS_RETURN(XK_g, XK_G, KEY_G);
	KEYS_RETURN(XK_h, XK_H, KEY_H);
	KEYS_RETURN(XK_i, XK_I, KEY_I);
	KEYS_RETURN(XK_j, XK_J, KEY_J);
	KEYS_RETURN(XK_k, XK_K, KEY_K);
	KEYS_RETURN(XK_l, XK_L, KEY_L);
	KEYS_RETURN(XK_m, XK_M, KEY_M);
	KEYS_RETURN(XK_n, XK_N, KEY_N);
	KEYS_RETURN(XK_o, XK_O, KEY_O);
	KEYS_RETURN(XK_p, XK_P, KEY_P);
	KEYS_RETURN(XK_q, XK_Q, KEY_Q);
	KEYS_RETURN(XK_r, XK_R, KEY_R);
	KEYS_RETURN(XK_s, XK_S, KEY_S);
	KEYS_RETURN(XK_t, XK_T, KEY_T);
	KEYS_RETURN(XK_u, XK_U, KEY_U);
	KEYS_RETURN(XK_v, XK_V, KEY_V);
	KEYS_RETURN(XK_w, XK_W, KEY_W);
	KEYS_RETURN(XK_x, XK_X, KEY_X);
	KEYS_RETURN(XK_y, XK_Y, KEY_Y);
	KEYS_RETURN(XK_z, XK_Z, KEY_Z);
	default: return 0;
    }
}

#endif