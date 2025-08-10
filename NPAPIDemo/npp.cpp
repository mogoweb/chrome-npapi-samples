// npp.cpp
#include "pch.h"
#include "npapi.h"
#include "npfunctions.h"
#include "npruntime.h"

#include <cstdlib>
#include <cstring>
#include <windows.h>

extern NPNetscapeFuncs* g_browserFuncs;

struct PluginInstanceData {
    int dummy;
    bool showHello;
    NPWindow* window;
    PluginInstanceData() : dummy(0), showHello(false), window(nullptr) {}
};

struct HelloNPObject : NPObject {
    NPP npp;
    PluginInstanceData* pdata;
};

// Forward
static void DrawPlugin(PluginInstanceData* pdata);

// ---------- NPObject lifecycle ----------
static NPObject* Hello_Allocate(NPP npp, NPClass* aClass) {
    if (!g_browserFuncs) return nullptr;
    HelloNPObject* obj = (HelloNPObject*)g_browserFuncs->memalloc(sizeof(HelloNPObject));
    if (!obj) return nullptr;
    std::memset(obj, 0, sizeof(HelloNPObject));
    return reinterpret_cast<NPObject*>(obj);
}

static void Hello_Deallocate(NPObject* obj) {
    if (!obj || !g_browserFuncs) return;
    // ������� HelloNPObject ������Դ���������ͷ�
    g_browserFuncs->memfree(obj);
}

static void Hello_Invalidate(NPObject* obj) {
    (void)obj;
}

static bool Hello_HasMethod(NPObject* obj, NPIdentifier name);
static bool Hello_Invoke(NPObject* obj, NPIdentifier name,
    const NPVariant* args, uint32_t argCount, NPVariant* result);

NPClass HelloNPClass = {
    NP_CLASS_STRUCT_VERSION,
    Hello_Allocate,
    Hello_Deallocate,
    Hello_Invalidate,
    Hello_HasMethod,
    Hello_Invoke,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

// ---------- Implementation ----------

static void DrawPlugin(PluginInstanceData* pdata) {
    if (!pdata || !pdata->window) return;
    // window->window �� Win32 ���� HWND
    HWND hwnd = (HWND)pdata->window->window;
    if (!hwnd) return;

    // ֱ�ӻ�ȡ DC �����ƣ��������� WM_PAINT��
    HDC hdc = GetDC(hwnd);
    if (!hdc) return;

    RECT rect;
    if (!GetClientRect(hwnd, &rect)) {
        ReleaseDC(hwnd, hdc);
        return;
    }

    // �����ɫ������Windows �����ɫ��
    HBRUSH brush = CreateSolidBrush(RGB(0, 120, 215));
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

    // �����Ҫ��ʾ�ı������
    if (pdata->showHello) {
        SetBkMode(hdc, TRANSPARENT);
        // ���־���
        DrawTextA(hdc, "Hello, NPAPI Plugin!", -1, &rect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    ReleaseDC(hwnd, hdc);
}

// sayHello ����ʵ��
static bool Hello_Invoke(NPObject* obj, NPIdentifier name,
    const NPVariant* args, uint32_t argCount, NPVariant* result) {
    if (!g_browserFuncs) return false;
    NPUTF8* methodName = g_browserFuncs->utf8fromidentifier(name);
    bool handled = false;
    if (methodName && std::strcmp((const char*)methodName, "sayHello") == 0) {
        HelloNPObject* helloObj = static_cast<HelloNPObject*>(obj);
        if (helloObj && helloObj->pdata) {
            helloObj->pdata->showHello = true;
            // �������ƣ������� WM_PAINT �� invalidaterect
            DrawPlugin(helloObj->pdata);
        }
        VOID_TO_NPVARIANT(*result);
        handled = true;
    }
    if (methodName) g_browserFuncs->memfree(methodName);
    return handled;
}

static bool Hello_HasMethod(NPObject* obj, NPIdentifier name) {
    if (!g_browserFuncs) return false;
    NPUTF8* methodName = g_browserFuncs->utf8fromidentifier(name);
    bool res = (methodName && std::strcmp((const char*)methodName, "sayHello") == 0);
    if (methodName) g_browserFuncs->memfree(methodName);
    return res;
}

// ---------------- NPP �ص� ----------------

NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode,
    int16_t argc, char* argn[], char* argv[], NPSavedData* saved) {
    if (!instance) return NPERR_GENERIC_ERROR;
    PluginInstanceData* d = (PluginInstanceData*)std::calloc(1, sizeof(PluginInstanceData));
    if (!d) return NPERR_OUT_OF_MEMORY_ERROR;
    // ��Ȼ calloc �Ѿ����㣬��������ʽ���ã���֤��ȷ��ʼֵ
    d->dummy = 0;
    d->showHello = false;
    d->window = nullptr;
    instance->pdata = d;
    return NPERR_NO_ERROR;
}

NPError NPP_Destroy(NPP instance, NPSavedData** save) {
    if (!instance) return NPERR_GENERIC_ERROR;
    if (instance->pdata) {
        PluginInstanceData* pdata = static_cast<PluginInstanceData*>(instance->pdata);
        std::free(pdata);
        instance->pdata = nullptr;
    }
    return NPERR_NO_ERROR;
}

// NPP_SetWindow: ���洰�ڲ������������ף��������ʱ�ῴ����ɫ������
NPError NPP_SetWindow(NPP instance, NPWindow* window) {
    if (!instance) return NPERR_GENERIC_ERROR;
    PluginInstanceData* pdata = static_cast<PluginInstanceData*>(instance->pdata);
    if (!pdata) return NPERR_GENERIC_ERROR;

    pdata->window = window;

    // �������Ʊ����������� WM_PAINT��
    DrawPlugin(pdata);

    return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) {
    return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason) {
    return NPERR_NO_ERROR;
}

int32_t NPP_WriteReady(NPP instance, NPStream* stream) {
    return 0x0FFFFFFF;
}

int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer) {
    return len;
}

void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname) {
    (void)instance; (void)stream; (void)fname;
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {
    (void)instance; (void)url; (void)reason; (void)notifyData;
}

// NPP_HandleEvent: ��Ȼʵ���Լ���ĳЩ�����������յ� WM_PAINT Ҳ����
int16_t NPP_HandleEvent(NPP instance, void* event) {
    if (!instance || !event) return 0;
    PluginInstanceData* pdata = static_cast<PluginInstanceData*>(instance->pdata);
    if (!pdata || !pdata->window) return 0;

    NPEvent* npEvent = static_cast<NPEvent*>(event);
    if (!npEvent) return 0;

    if (npEvent->event == WM_PAINT) {
        // �� WM_PAINT ��Ҳ���ƣ��� SetWindow/Invoke �еĻ��Ʊ���һ�£�
        PAINTSTRUCT ps;
        HWND hwnd = (HWND)pdata->window->window;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (hdc) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH brush = CreateSolidBrush(RGB(0, 120, 215));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            if (pdata->showHello) {
                SetBkMode(hdc, TRANSPARENT);
                DrawTextA(hdc, "Hello, NPAPI Plugin!", -1, &rect,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }
            EndPaint(hwnd, &ps);
            return 1;
        }
    }
    return 0;
}

// NPP_GetValue ���� NPObject
NPError NPP_GetValue(NPP instance, NPPVariable variable, void* value) {
    if (variable == NPPVpluginScriptableNPObject) {
        if (!instance || !g_browserFuncs) return NPERR_GENERIC_ERROR;
        PluginInstanceData* pdata = static_cast<PluginInstanceData*>(instance->pdata);
        NPObject* obj = g_browserFuncs->createobject(instance, &HelloNPClass);
        if (!obj) return NPERR_OUT_OF_MEMORY_ERROR;
        HelloNPObject* hello = static_cast<HelloNPObject*>(obj);
        hello->npp = instance;
        hello->pdata = pdata;
        *(NPObject**)value = obj;
        return NPERR_NO_ERROR;
    }
    return NPERR_GENERIC_ERROR;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void* value) {
    (void)instance; (void)variable; (void)value;
    return NPERR_GENERIC_ERROR;
}
