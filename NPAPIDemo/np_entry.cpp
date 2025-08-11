// np_entry.cpp
#include "pch.h"
#include <windows.h>
#include "npapi.h"
#include "npfunctions.h"
#include "npruntime.h"
#include "log.h"

// ����������ĺ�����ָ��
NPNetscapeFuncs* g_browserFuncs = nullptr;

// ��������ṩ�� MIME ��������
// �﷨: "MIME����:�ļ���չ��:�������"
static const char* kMimeDescription = "application/x-npapi-demo:.npd:NPAPI Demo Plugin";

// ��ȡ�����ڵ�
extern "C" NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pluginFuncs) {
    if (!pluginFuncs) return NPERR_INVALID_FUNCTABLE_ERROR;

    // ��ʼ������ָ�루ֻʾ��������
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

// �����ʼ��
extern "C"  NPError OSCALL NP_Initialize(NPNetscapeFuncs* browserFuncs) {
    // �� DllMain ����ڴ�����
    WriteDebugLog("NPAPIDemo: NP_Initialize called.\n");

    if (!browserFuncs) return NPERR_INVALID_FUNCTABLE_ERROR;
    g_browserFuncs = browserFuncs;
    return NPERR_NO_ERROR;
}

// ���ж��
extern "C"  NPError OSCALL NP_Shutdown(void) {
    g_browserFuncs = nullptr;
    return NPERR_NO_ERROR;
}

// ���ز��֧�ֵ� MIME ����
extern "C"  const char* NP_GetMIMEDescription(void) {
    return kMimeDescription;
}
