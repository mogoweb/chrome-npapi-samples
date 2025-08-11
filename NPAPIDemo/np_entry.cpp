// np_entry.cpp
#include "pch.h"
#include <windows.h>
#include "npapi.h"
#include "npfunctions.h"
#include "npruntime.h"
#include "log.h"

// 保存浏览器的函数表指针
NPNetscapeFuncs* g_browserFuncs = nullptr;

// 插件对外提供的 MIME 类型描述
// 语法: "MIME类型:文件扩展名:插件描述"
static const char* kMimeDescription = "application/x-npapi-demo:.npd:NPAPI Demo Plugin";

// 获取插件入口点
extern "C" NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pluginFuncs) {
    if (!pluginFuncs) return NPERR_INVALID_FUNCTABLE_ERROR;

    // 初始化函数指针（只示例几个）
    pluginFuncs->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
    pluginFuncs->newp = NPP_New;
    pluginFuncs->destroy = NPP_Destroy;
    pluginFuncs->setwindow = NPP_SetWindow;
    pluginFuncs->newstream = NPP_NewStream;
    pluginFuncs->destroystream = NPP_DestroyStream;
    pluginFuncs->asfile = NPP_StreamAsFile;
    pluginFuncs->writeready = NPP_WriteReady;
    pluginFuncs->write = NPP_Write;
    pluginFuncs->event = NPP_HandleEvent;
    pluginFuncs->urlnotify = NPP_URLNotify;
    pluginFuncs->getvalue = NPP_GetValue;
    pluginFuncs->setvalue = NPP_SetValue;
    return NPERR_NO_ERROR;
}

// 插件初始化
extern "C"  NPError OSCALL NP_Initialize(NPNetscapeFuncs* browserFuncs) {
    // 在 DllMain 的入口处调用
    WriteDebugLog("NPAPIDemo: NP_Initialize called.\n");

    if (!browserFuncs) return NPERR_INVALID_FUNCTABLE_ERROR;
    g_browserFuncs = browserFuncs;
    return NPERR_NO_ERROR;
}

// 插件卸载
extern "C"  NPError OSCALL NP_Shutdown(void) {
    g_browserFuncs = nullptr;
    return NPERR_NO_ERROR;
}

// 返回插件支持的 MIME 描述
extern "C"  const char* NP_GetMIMEDescription(void) {
    return kMimeDescription;
}
