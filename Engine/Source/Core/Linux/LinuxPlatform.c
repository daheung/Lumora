#include "Platform/Platform.h"

/** Linux platform layer. */
#if PLATFORM_LINUX
#define _POSIX_C_SOURCE 200809L

#include "Core/Logger.h"

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

typedef struct FInternalState
{
    Display* Display;
    xcb_connection_t* Connection;
    xcb_window_t Window;
    xcb_screen_t* Screen;
    xcb_atom_t wmProtocols;
    xcb_atom_t wmDeleteWindow;
} FInternalState;

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

    if (xcb_connection_has_error(InternalState->Display))
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
        strlen(ApplicationName),
        ApplicationName
    );

    /** 
     * Tell the server to notify when the window manager
     * attempts to destroy the window.
     */
    xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(
        InternalState->Connection, 
        0, 
        strlen("WM_DELETE_WINDOW"), 
        "WM_DELETE_WINDOW"
    );
    xcb_intern_atom_cookie_t wmProtocolsCookie = xcb_intern_atom(
        InternalState->Connection, 
        0, 
        strlen("WM_PROTOCOLS"), 
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

    xcb_generic_error_t* Event;
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
            /** TODO: Key presses and releases */
        }
            break;
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
        {
            /** TODO: Mouse button presses and releases */
        }
            break;
        case XCB_MOTION_NOTIFY:
            /** TODO: Mouse movementsss */
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

#endif