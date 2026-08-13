#pragma once

#include "Defines.h"

typedef struct FCoreEventContext
{
    /** 128 bytes */
    union 
    {
        int64   I64[2];
        uint64  U64[2];
        float64 F64[2];

        int32   I32[4];
        uint32  U32[4];
        float32 F32[4];

        int16   I16[8];
        uint16  U16[8];

        int8    I8[16];
        uint8   U8[16];

        char    C[16];
    } Data;
} FCoreEventContext;

/** Should return true if handled. */
typedef bool8 (*OnEventFunc)(uint16 Code, void* Sender, void* ListenerInstance, FCoreEventContext Data);

bool8 InitializeEvent();
void ReleaseEvent();

/**
 * Register to listen for when events are sent with the provided code. Events with duplicate
 * listener/callback combos will not be registered again and will cause this to return FALSE.
 * @param Code The event code to listen for.
 * @param Listener A pointer to a listener instance. Can be 0/NULL.
 * @param OnEvent The callback function pointer to be invoked when the event code is fired.
 * @returns TRUE if the event is successfully registered; otherwise FALSE.
 */
LUMORA_C_API bool8 RegisterEvent(uint16 Code, void* Listener, OnEventFunc OnEvent);

/**
 * Unregister from listening for when events are sent with the provided code. If no matching
 * registration is found, this function returns FALSE.
 * @param Code The event code to stop listening for.
 * @param Listener A pointer to a listener instance. Can be 0/NULL.
 * @param OnEvent The callback function pointer to be unregistered.
 * @returns TRUE if the event is successfully unregistered; otherwise FALSE.
 */
LUMORA_C_API bool8 UnregisterEvent(uint16 Code, void* Listener, OnEventFunc OnEvent);

/**
 * Fires an event to listeners of the given code. If an event handler returns
 * TRUE, the event is considered handled and is not passed on to any more listeners.
 * @param Code The event code to fire.
 * @param Sender A pointer to the sender. Can be 0/NULL.
 * @param Data The event data.
 * @returns TRUE if handled; otherwise FALSE.
 */
LUMORA_C_API bool8 FireEvent(uint16 Code, void* Sender, FCoreEventContext Context);

/** System internal event codes. Application should use codes beyond 255. */
typedef enum ESystemEventCode
{
    /** Shuts the application down on the next frame. */
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    /** 
     * Keyboard key pressed. 
     * 
     * Context usage:
     * uint16 KeyCode = Data.Data.U16[0];
     */
    EVENT_CODE_KEY_PRESSED = 0x02,

    /** 
     * Keyboard key released. 
     * 
     * Context usage:
     * uint16 KeyCode = Data.Data.U16[0];
     */
    EVENT_CODE_KEY_RELEASED = 0x03,

    /** 
     * Mouse button pressed. 
     * 
     * Context usage:
     * uint16 button = Data.Data.U16[0];
     */
    EVENT_CODE_BUTTON_PRESSED = 0x04,

    /** 
     * Mouse button released. 
     * 
     * Context usage:
     * uint16 button = Data.Data.U16[0];
     */
    EVENT_CODE_BUTTON_RELEASED = 0x05,
    
    /**
     * Mouse moved.
     * 
     * Context usage:
     * uint16 X = Data.Data.U16[0];
     * uint16 Y = Data.Data.U16[1];
     */
    EVENT_CODE_MOUSE_MOVED = 0x06,

    /**
     * Mouse moved.
     * 
     * Context usage:
     * uint16 DeltaZ = Data.Data.U16[0];
     */
    EVENT_CODE_MOUSE_WHEEL = 0x07,

    /**
     * Resized/resolution changed from the OS.
     * 
     * Context usage:
     * uint16 Width = Data.Data.U16[0];
     * uint16 Height = Data.Data.U16[1]
     */
    EVENT_CODE_RESIZED = 0x08,
   
    MAX_EVENT_CODE = 0xFF,
} ESystemEventCode;