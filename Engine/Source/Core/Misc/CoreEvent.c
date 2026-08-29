#include "Misc/CoreEvent.h"

#include "HAL/LumoraMemory.h"
#include "Containers/Array.h"
#include "Asserts.h"
#include "Logger.h"

typedef struct FRegisteredEvent
{
    void* Listener;
    OnEventFunc Callback;
} FRegisteredEvent;

typedef struct FEventCodeEntry
{
    FRegisteredEvent* Events;
} FEventCodeEntry;

/** This should be more than enough codes... */
#define MAX_MESSAGE_CODES 16384

/** State structure. */
typedef struct FEventSystemState
{
    /** Lookup table for event codes. */
    FEventCodeEntry Registered[MAX_EVENT_CODE];
} FEventSystemState;

/**
 * Event system internal state.
 */
static FEventSystemState* GEventSystemState;


void InitializeEvent(size_t* const MemoryRequirement, void* State)
{
    *MemoryRequirement = sizeof(FEventSystemState);
    if (State == NULL) {
        return;
    }

    HZeroMemory(State, sizeof(FEventSystemState));
    GEventSystemState = State;
}

void ReleaseEvent()
{
    /** Free the events array. And objects pointed to should be destroyed on their own. */
    for (uint16 Index = 0; Index < MAX_EVENT_CODE; ++Index)
    {
        if (GEventSystemState->Registered[Index].Events != 0)
        {
            CArrayRelease(GEventSystemState->Registered[Index].Events);
            GEventSystemState->Registered[Index].Events = 0;
        }
    }
}

LUMORA_C_API bool8 RegisterEvent(uint16 Code, void *Listener, OnEventFunc OnEvent)
{
    LUMORA_LOG(GEventSystemState, LOG_LEVEL_FATAL, "CoreEvent has not been initialized.");
    
    if (GEventSystemState->Registered[Code].Events == 0)
    {
        GEventSystemState->Registered[Code].Events = CArrayCreate(sizeof(FRegisteredEvent));
    }

    /**
     * Prevent duplicate listener registration.
     * Duplicate registrations may leave a stale listener reference if the listener
     *  is unregistered only once and later destroyed.
     */
    uint32 RegisteredCount = (uint32)CArrayLength(GEventSystemState->Registered[Code].Events);
    for (uint32 Index = 0; Index < RegisteredCount; ++Index)
    {
        if (GEventSystemState->Registered[Code].Events[Index].Listener == Listener)
        {
            /** TODO: Warn */
            return FALSE;
        }
    }

    /** If at this point, no duplicate was found. Proceed with registration. */
    FRegisteredEvent Event = { 0 };
    Event.Listener = Listener;
    Event.Callback = OnEvent;
    CArrayPush(GEventSystemState->Registered[Code].Events, &Event);

    return TRUE;
}

LUMORA_C_API bool8 UnregisterEvent(uint16 Code, void *Listener, OnEventFunc OnEvent)
{
    LUMORA_LOG(GEventSystemState, LOG_LEVEL_FATAL, "CoreEvent has not been initialized.");

    /** On nothing is registered for the code, boot out. */
    if (GEventSystemState->Registered[Code].Events == 0)
    {
        /** TODO: Warn */
        return FALSE;
    }

    uint32 RegisteredCount = (uint32)CArrayLength(GEventSystemState->Registered[Code].Events);
    for (uint32 Index = 0; Index < RegisteredCount; ++Index)
    {
        FRegisteredEvent Event = GEventSystemState->Registered[Code].Events[Index];
        if (Event.Listener == Listener && Event.Callback == OnEvent)
        {
            /** Found one, remove it */
            FRegisteredEvent PoppedEvent;
            CArrayPopAt(GEventSystemState->Registered[Code].Events, Index, &PoppedEvent);
            return TRUE;
        }
    }

    /** Not found. */
    return FALSE;
}

LUMORA_C_API bool8 FireEvent(uint16 Code, void *Sender, FCoreEventContext Context)
{
    LUMORA_LOG(GEventSystemState, LOG_LEVEL_FATAL, "CoreEvent has not been initialized.");
    
    /** If nothing is registered for the code, boot out. */
    if (GEventSystemState->Registered[Code].Events == 0)
    {
        return FALSE;
    }

    uint32 RegisteredCount = CArrayLength(GEventSystemState->Registered[Code].Events);
    for (uint32 Index = 0; Index < RegisteredCount; ++Index)
    {
        FRegisteredEvent Event = GEventSystemState->Registered[Code].Events[Index];
        if (Event.Callback(Code, Sender, Event.Listener, Context))
        {
            /** Message has been handled, do not send to other listeners. */
            return TRUE;
        }
    }

    /** Not found. */
    return FALSE;
}
