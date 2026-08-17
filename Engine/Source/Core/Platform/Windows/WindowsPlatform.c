#include "Platform/Platform.h"
#include "Logger.h"
#include "Asserts.h"
#include "InputCore/Input.h"
#include "Core/Containers/Array.h"

#if PLATFORM_WINDOWS

#include <Windows.h>
#include <windowsx.h> // param input extraction

/** For surface createion */
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <Vulkan/VulkanTypes.inl>

typedef struct FInternalState 
{
    HINSTANCE Instance;
    HWND Hwnd;
    VkSurfaceKHR Surface;
} FInternalState;

/** Clock */
static float64 GClockFrequency;
static LARGE_INTEGER GStartTime;

static LRESULT CALLBACK Win32ProcessMessage(HWND, UINT, WPARAM, LPARAM);
static FORCEINLINE void PlatformConsoleWriteImpl(const char* Message, uint8 Color, HANDLE ConsoleHandle);

static FORCEINLINE void ProcessKeyInputImpl(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

bool8 PlatformStartup(FPlatformState* PlatformState, const char* ApplicationName, int32 X, int32 Y, int32 Width, int32 Height)
{
    FInternalState* InternalState = (FInternalState*)malloc(sizeof(FInternalState));
    PlatformState->InternalState = InternalState;

    InternalState->Instance = GetModuleHandleA(0);

    /** Setup and register window class. */
    HICON Icon = LoadIcon(InternalState->Instance, IDI_APPLICATION);
    WNDCLASSA WindowClass = { 0 };
    WindowClass.style = CS_DBLCLKS;
    WindowClass.lpfnWndProc = Win32ProcessMessage;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = InternalState->Instance;
    WindowClass.hIcon = Icon;
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowClass.hbrBackground = NULL;
    WindowClass.lpszClassName = "Lumora Window Class";

    if (!RegisterClassA(&WindowClass))
    {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    /** Create Window */
    uint32 ClientX = X;
    uint32 ClientY = Y;
    uint32 ClientWidth  = Width;
    uint32 ClientHeight = Height;

    uint32 WindowX = ClientX;
    uint32 WindowY = ClientY;
    uint32 WindowWidth  = ClientWidth;
    uint32 WindowHeight = ClientHeight;

    uint32 WindowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    const uint32 WindowExStyle = WS_EX_APPWINDOW;

    WindowStyle |= WS_MAXIMIZEBOX;
    WindowStyle |= WS_MINIMIZEBOX;
    WindowStyle |= WS_THICKFRAME;

    /** Obtain the size of the border. */
    RECT BorderRect = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&BorderRect, WindowStyle, 0, WindowExStyle);

    /** In this case, the border rectangle is negative. */
    WindowX += BorderRect.left;
    WindowY += BorderRect.top;

    /** Grow by the size of the OS border. */
    WindowWidth  += BorderRect.right - BorderRect.left;
    WindowHeight += BorderRect.bottom - BorderRect.top;

    HWND Handle = CreateWindowExA(
        WindowExStyle, "Lumora Window Class", ApplicationName,
        WindowStyle, WindowX, WindowY, WindowWidth, WindowHeight,
        0, 0, InternalState->Instance, 0
    );

    if (Handle == 0)
    {
        MessageBoxA(NULL, "Window creation failed.", "Error", MB_ICONEXCLAMATION | MB_OK);
        LUMORA_FATAL("Window creation failed.")
        return FALSE;
    }

    InternalState->Hwnd = Handle;

    /** Show the window */
    bool32 ShouldActivate = 1;
    int32 ShowWindowCommandFlags = ShouldActivate ?
        SW_SHOW : SW_SHOWNOACTIVATE;

    /** 
     * If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
     * If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE;
     */
    ShowWindow(InternalState->Hwnd, ShowWindowCommandFlags);

    /** Clock setup */
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    GClockFrequency = 1.0 / (float64)Frequency.QuadPart;
    QueryPerformanceCounter(&GStartTime);

    return TRUE;
}

void PlatformShutdown(FPlatformState* PlatformState)
{
    /** Simply cold-cast to the known type. */
    FInternalState* InternalState = (FInternalState*)PlatformState->InternalState;
    
    if (InternalState->Hwnd)
    {
        DestroyWindow(InternalState->Hwnd);
        InternalState->Hwnd = 0;
    }
}

bool8 PlatformPumpMessage(FPlatformState* PlatformState)
{
    MSG Message = { 0, };
    while (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }

    return TRUE;
}

void* PlatformAllocate(uint64 AllocSize, bool8 bAligned)
{
    UNREFERENCED_PARAMETER(bAligned);
    return malloc(AllocSize);
}

void PlatformFree(void* Block, bool8 bAligned)
{
    UNREFERENCED_PARAMETER(bAligned);
    free(Block);    
}

void* PlatformZeroMemory(void* Block, uint64 AllocSize)
{
    return memset(Block, 0, AllocSize);
}

void* PlatformCopyMemory(void* Dest, const void* Src, uint64 AllocSize)
{
    return memcpy(Dest, Src, AllocSize);
}

void* PlatformSetMemory(void* Dest, int32 Value, uint64 AllocSize)
{
    return memset(Dest, Value, AllocSize);
}

void PlatformConsoleWrite(const char* Message, uint8 Color)
{
    PlatformConsoleWriteImpl(Message, Color, GetStdHandle(STD_OUTPUT_HANDLE));
}

void PlatformConsoleWriteError(const char* Message, uint8 Color)
{
    PlatformConsoleWriteImpl(Message, Color, GetStdHandle(STD_ERROR_HANDLE));
}

static FORCEINLINE void PlatformConsoleWriteImpl(const char* Message, uint8 Color, HANDLE ConsoleHandle)
{
    STATIC_ASSERT(LOG_LEVEL_FATAL == 0, "LOG_LEVEL_FATAL must be 0; check ELogLevel.");
    STATIC_ASSERT(LOG_LEVEL_ERROR == 1, "LOG_LEVEL_ERROR must be 1; check ELogLevel.");
    STATIC_ASSERT(LOG_LEVEL_WARN  == 2, "LOG_LEVEL_WARN must be 2; check ELogLevel.");
    STATIC_ASSERT(LOG_LEVEL_INFO  == 3, "LOG_LEVEL_INFO must be 3; check ELogLevel.");
    STATIC_ASSERT(LOG_LEVEL_DEBUG == 4, "LOG_LEVEL_DEBUG must be 4; check ELogLevel.");
    STATIC_ASSERT(LOG_LEVEL_TRACE == 5, "LOG_LEVEL_TRACE must be 5; check ELogLevel.");

    LUMORA_ASSERT_MSG(Color < LOG_LEVEL_COUNT, "Invalid log level");
    
    /** FATAL, ERROR, WARN, INFO, DEBUG, TRACE */
    static uint8 Levels[] = { 
        BACKGROUND_RED,                     // 64
        FOREGROUND_RED,                     // 4
        FOREGROUND_GREEN | FOREGROUND_RED,  // 6
        FOREGROUND_GREEN,                   // 2
        FOREGROUND_BLUE,                    // 1
        FOREGROUND_INTENSITY                // 8
    };
    SetConsoleTextAttribute(ConsoleHandle, Levels[Color]);

    OutputDebugStringA(Message);

    uint64 Length = strlen(Message);
    LPDWORD NumberWritten = 0;
    WriteConsoleA(ConsoleHandle, Message, (DWORD)Length, NumberWritten, 0);
}

float64 PlatformGetAbsoluteTime(void)
{
    LARGE_INTEGER CurrentTime = { 0 };
    QueryPerformanceCounter(&CurrentTime);
    return (float64)CurrentTime.QuadPart * GClockFrequency;
}

void PlatformSleep(uint64 MilliSecond)
{
    Sleep(MilliSecond);
}

void PlatformGetRequiredExtensionNames(const char*** CArrayNames)
{
    const char* Win32Surface = "VK_KHR_win32_surface";
    CArrayPush(*CArrayNames, &Win32Surface);
}

/** Surface createion for Vulkan. */
bool8 PlatformCreateVulkanSurface(struct FPlatformState* PlatformState, struct FVulkanContext* VulkanContext)
{
    /** Siply cold-cast to the known type. */
    FInternalState* InternalState = (FInternalState*)PlatformState->InternalState;
    
    VkWin32SurfaceCreateInfoKHR CreateInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    CreateInfo.hinstance = InternalState->Instance;
    CreateInfo.hwnd = InternalState->Hwnd;

    VkResult Result = vkCreateWin32SurfaceKHR(VulkanContext->Instance, &CreateInfo, VulkanContext->Allocator, &InternalState->Surface);
    if (Result != VK_SUCCESS)
    {
        LUMORA_FATAL("Vulkan surface createion failed.");
        return FALSE;
    }

    VulkanContext->Surface = InternalState->Surface;
    return TRUE;
}

LRESULT CALLBACK Win32ProcessMessage(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
    case WM_ERASEBKGND:
        /** Notify the OS that erasing will be handled by the application to prevent flicker. */
        return 1;
    case WM_CLOSE:
        /** TODO: Fire an event for the application to quit. */
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE: 
    {
        /** Get the updated size. */
        // RECT Rect = {};
        // GetClientRect(Hwnd, &Rect);
        // uint32 Width  = Rect.right - Rect.left;
        // uint32 Height = Rect.bottom - Rect.top;

        /** TODO: Fire an event for window resize. */
    } 
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: 
        ProcessKeyInputImpl(Hwnd, Message, wParam, lParam);
        break;
    case WM_MOUSEMOVE: 
    {
        /** Mouse move */
        const int32 PositionX = GET_X_LPARAM(lParam);
        const int32 PositionY = GET_Y_LPARAM(lParam);

        /** Pass over to the input subsystem. */
        ProcessInputMouseMove(PositionX, PositionY);
    }
        break;
    case WM_MOUSEWHEEL:
    {
        int32 DeltaZ = GET_WHEEL_DELTA_WPARAM(wParam);
        if (DeltaZ != 0) {
            /** Flatten the input to an OS-independent (-1, 1) */
            DeltaZ = (DeltaZ < 0) ? -1 : 1;
            ProcessInputMouseWheel(DeltaZ);            
        }
    }
        break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        bool8 bPressed = Message == WM_LBUTTONDOWN || Message == WM_RBUTTONDOWN || Message == WM_MBUTTONDOWN;
        EButtons MouseButton = BUTTON_MAX_BUTTONS;
        switch (Message)
        {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            MouseButton = BUTTON_LEFT;
            break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            MouseButton = BUTTON_MIDDLE;
            break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
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
    default:
        break;
    }

    return DefWindowProcA(Hwnd, Message, wParam, lParam);
}

/**
 * NOTE: Input Handling Strategy
 *
 * GetAsyncKeyState() performs discrete-time polling of the current
 * asynchronous key state at the exact moment the function is called.
 *
 * If a key is pressed and released entirely between two input samples,
 * the engine may observe the key as released at both sampling points and
 * therefore fail to detect the state transition. In other words, short-lived
 * input events can be missed when only the instantaneous key state is polled.
 *
 * WM_KEYDOWN and WM_KEYUP, on the other hand, are event-based. Windows
 * places keyboard messages into the owning thread's message queue, and the
 * application retrieves and dispatches those messages through its message
 * pump (e.g. PeekMessage/GetMessage -> DispatchMessage -> WndProc).
 *
 * Because the key-down and key-up transitions are represented as separate
 * queued messages, input transitions that occur between simulation frames
 * can still be processed when the message queue is pumped.
 *
 * Keyboard messages are also associated with the window/thread that owns
 * keyboard focus, which naturally follows the normal Windows focus model.
 *
 * For this reason, the platform layer uses WM_KEYDOWN/WM_KEYUP to record
 * input transitions and updates the engine's internal input state from those
 * events, rather than polling every key with GetAsyncKeyState().
 */
static FORCEINLINE void ProcessKeyInputImpl(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    /** Key pressed/released */
    bool8 bPressed = (Message == WM_KEYDOWN || Message == WM_SYSKEYDOWN);
    EKeys Key = (uint16)wParam;

    /** Pass to the input subsystem for processing. */
    ProcessInputKey(Key, bPressed);
}

#endif