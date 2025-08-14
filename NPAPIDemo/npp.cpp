// npp.cpp
#include "pch.h"
#include "npapi.h"
#include "npfunctions.h"
#include "npruntime.h"

#include <cstdlib>
#include <cstring>
#include <windows.h>

#include "log.h"

extern NPNetscapeFuncs* g_browserFuncs;

// �Ӵ��ڵĴ�������
const wchar_t CHILD_WINDOW_CLASS_NAME[] = L"NPAPI_ChildWindowClass";

struct PluginInstanceData {
    bool showHello;
    NPWindow* window;
    HWND m_childHwnd; // ���ڱ����Ӵ��ھ��

    PluginInstanceData() : showHello(false), window(nullptr), m_childHwnd(nullptr) {}
};

struct HelloNPObject : NPObject {
    NPP npp;
    PluginInstanceData* pdata;
};

// Forward
static void DrawPlugin(PluginInstanceData* pdata);
static LRESULT CALLBACK ChildWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
    g_browserFuncs->memfree(obj);
}

static void Hello_Invalidate(NPObject* obj) {
    (void)obj;
}

static bool Hello_HasMethod(NPObject* obj, NPIdentifier name);
static bool Hello_Invoke(NPObject* obj, NPIdentifier name,
    const NPVariant* args, uint32_t argCount, NPVariant* result);
static bool Hello_HasProperty(NPObject* obj, NPIdentifier name);

NPClass HelloNPClass = {
    NP_CLASS_STRUCT_VERSION,
    Hello_Allocate,
    Hello_Deallocate,
    Hello_Invalidate,
    Hello_HasMethod,
    Hello_Invoke,
    nullptr,
    Hello_HasProperty,
    nullptr,
    nullptr,
    nullptr
};

// ---------- Implementation ----------

// �Ӵ��ڵĴ��ڹ��̺���
LRESULT CALLBACK ChildWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (hdc) {
            RECT rect;
            GetClientRect(hWnd, &rect);

            // 1. ����ɫ����
            HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &rect, whiteBrush);
            DeleteObject(whiteBrush);

            // 2. ���� "Hello Childwindow" ����
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0)); // ��ɫ����
            DrawTextA(hdc, "Hello Childwindow", -1, &rect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            EndPaint(hWnd, &ps);
        }
        return 0;
    }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


// ��������ڻ��ƺ�����ֻ���Ʊ�����
static void DrawPlugin(PluginInstanceData* pdata) {
    if (!pdata || !pdata->window) return;
    HWND hwnd = (HWND)pdata->window->window;
    if (!hwnd) return;

    HDC hdc = GetDC(hwnd);
    if (!hdc) return;

    RECT rect;
    if (!GetClientRect(hwnd, &rect)) {
        ReleaseDC(hwnd, hdc);
        return;
    }

    // �����ɫ����
    HBRUSH brush = CreateSolidBrush(RGB(0, 120, 215));
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

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
            // ����������
            DrawPlugin(helloObj->pdata);

            // ���Ӵ���Ҳ�ػ�
            if (helloObj->pdata->m_childHwnd) {
                InvalidateRect(helloObj->pdata->m_childHwnd, NULL, TRUE);
                UpdateWindow(helloObj->pdata->m_childHwnd);
            }
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

static bool Hello_HasProperty(NPObject* obj, NPIdentifier name) {
    return false;
}

// ---------------- NPP �ص� ----------------

NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode,
    int16_t argc, char* argn[], char* argv[], NPSavedData* saved) {
    if (!instance) return NPERR_GENERIC_ERROR;

    // Ϊ�Ӵ���ע�ᴰ����
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ChildWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL); // ��ȡ��ǰģ����
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = CHILD_WINDOW_CLASS_NAME;
    wcex.hIcon = NULL;
    wcex.hIconSm = NULL;

    // ֻ��Ҫע��һ��
    if (!GetClassInfoExW(wcex.hInstance, CHILD_WINDOW_CLASS_NAME, &wcex)) {
         RegisterClassExW(&wcex);
    }

    PluginInstanceData* d = new PluginInstanceData();
    if (!d) return NPERR_OUT_OF_MEMORY_ERROR;
    instance->pdata = d;
    return NPERR_NO_ERROR;
}

NPError NPP_Destroy(NPP instance, NPSavedData** save) {
    if (!instance) return NPERR_GENERIC_ERROR;
    if (instance->pdata) {
        PluginInstanceData* pdata = static_cast<PluginInstanceData*>(instance->pdata);

        // �����Ӵ���
        if (pdata->m_childHwnd) {
            DestroyWindow(pdata->m_childHwnd);
            pdata->m_childHwnd = nullptr;
        }

        delete pdata;
        instance->pdata = nullptr;
    }
    return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP instance, NPWindow* window) {
    if (!instance) return NPERR_GENERIC_ERROR;
    PluginInstanceData* pdata = static_cast<PluginInstanceData*>(instance->pdata);
    if (!pdata) return NPERR_GENERIC_ERROR;

    pdata->window = window;
    HWND parentHwnd = (HWND)window->window;

    if (parentHwnd) {
        if (pdata->m_childHwnd == nullptr) {
            // �����Ӵ���
            pdata->m_childHwnd = CreateWindowExW(
                0,                              // Optional window styles.
                CHILD_WINDOW_CLASS_NAME,        // Window class
                L"My Child Window",             // Window text
                WS_CHILD | WS_VISIBLE,          // Window style

                // Position and size (e.g., inset by 10 pixels)
                10, 10, window->width - 20, window->height - 20,

                parentHwnd,                     // Parent window
                NULL,                           // Menu
                GetModuleHandle(NULL),          // Instance handle
                NULL                            // Additional application data
            );
        } else {
            // ��������Ѵ��ڣ���������С��λ��
            MoveWindow(pdata->m_childHwnd, 10, 10, window->width - 20, window->height - 20, TRUE);
        }

        InvalidateRect(parentHwnd, NULL, TRUE);
    }

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
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {
}

int16_t NPP_HandleEvent(NPP instance, void* event) {
    // �¼�������Ҫ���Ӵ����Լ��� WindowProc ����
    // �����Ǳ��������ڵ� WM_PAINT �������ػ���ɫ������
    if (!instance) return 0;
    PluginInstanceData* pdata = static_cast<PluginInstanceData*>(instance->pdata);
    if (!pdata || !pdata->window) return 0;

    NPEvent* npEvent = static_cast<NPEvent*>(event);
    if (!npEvent) return 0;

    if (npEvent->event == WM_PAINT) {
        HWND hwnd = (HWND)pdata->window->window;
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (hdc) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH brush = CreateSolidBrush(RGB(0, 120, 215));
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            EndPaint(hwnd, &ps);
            return 1;
        }
    }
    return 0; // �����¼����ǲ�����
}

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
    return NPERR_GENERIC_ERROR;
}