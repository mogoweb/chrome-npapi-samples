// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "log.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        WriteDebugLog("NPAPIDemo: DLL_PROCESS_ATTACH called.\n");
        break;
    case DLL_THREAD_ATTACH:
        WriteDebugLog("NPAPIDemo: DLL_THREAD_ATTACH called.\n");
        break;
    case DLL_THREAD_DETACH:
        WriteDebugLog("NPAPIDemo: DLL_THREAD_DETACH called.\n");
        break;
    case DLL_PROCESS_DETACH:
        WriteDebugLog("NPAPIDemo: DLL_PROCESS_DETACH called.\n");
        break;
    }
    return TRUE;
}
