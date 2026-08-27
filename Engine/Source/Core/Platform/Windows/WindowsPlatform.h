#if defined(_MSC_VER)
    #pragma comment(lib, "user32.lib")
#endif

#define CDECL       __cdecl
#define STDCALL     __stdcall
#define FASTCALL    __fastcall
#define NORETURN    __declspec(noreturn)
#define INLINE      inline
#define FORCEINLINE __forceinline

#if defined(__cplusplus) && __cplusplus >= 201703L
    #define LUMORA_NODISCARD [[nodiscard]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define LUMORA_NODISCARD [[nodiscard]]
#endif