// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

// 在 DllMain 外部定义一个辅助函数，避免在 DllMain 中做字符串拼接
void WriteDebugLog(const char* message) {
    OutputDebugStringA(message);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    // 在 DllMain 的入口处调用
    WriteDebugLog("NPAPIDemo: DllMain called.\n");

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
