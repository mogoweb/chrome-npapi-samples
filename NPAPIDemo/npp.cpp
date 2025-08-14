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

// 子窗口的窗口类名
const wchar_t CHILD_WINDOW_CLASS_NAME[] = L"NPAPI_ChildWindowClass";

struct PluginInstanceData {
    bool showHello;
    NPWindow* window;
    HWND m_childHwnd; // 用于保存子窗口句柄

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

// 子窗口的窗口过程函数
LRESULT CALLBACK ChildWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (hdc) {
            RECT rect;
            GetClientRect(hWnd, &rect);

            // 1. 填充白色背景
            HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &rect, whiteBrush);
            DeleteObject(whiteBrush);

            // 2. 绘制 "Hello Childwindow" 文字
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0)); // 黑色文字
            DrawTextA(hdc, "Hello Childwindow", -1, &rect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            EndPaint(hWnd, &ps);
        }
        return 0;
    }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


// 主插件窗口绘制函数（只绘制背景）
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

    // 填充蓝色背景
    HBRUSH brush = CreateSolidBrush(RGB(0, 120, 215));
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

    ReleaseDC(hwnd, hdc);
}

// sayHello 方法实现
static bool Hello_Invoke(NPObject* obj, NPIdentifier name,
    const NPVariant* args, uint32_t argCount, NPVariant* result) {
    if (!g_browserFuncs) return false;
    NPUTF8* methodName = g_browserFuncs->utf8fromidentifier(name);
    bool handled = false;
    if (methodName && std::strcmp((const char*)methodName, "sayHello") == 0) {
        HelloNPObject* helloObj = static_cast<HelloNPObject*>(obj);
        if (helloObj && helloObj->pdata) {
            helloObj->pdata->showHello = true;
            // 更新主窗口
            DrawPlugin(helloObj->pdata);

            // 让子窗口也重绘
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

// ---------------- NPP 回调 ----------------

NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode,
    int16_t argc, char* argn[], char* argv[], NPSavedData* saved) {
    if (!instance) return NPERR_GENERIC_ERROR;

    // 为子窗口注册窗口类
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ChildWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL); // 获取当前模块句柄
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = CHILD_WINDOW_CLASS_NAME;
    wcex.hIcon = NULL;
    wcex.hIconSm = NULL;

    // 只需要注册一次
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

        // 销毁子窗口
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
            // 创建子窗口
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
            // 如果窗口已存在，仅调整大小和位置
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
    // 事件现在主要由子窗口自己的 WindowProc 处理，
    // 但我们保留父窗口的 WM_PAINT 处理以重绘蓝色背景。
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
    return 0; // 其他事件我们不处理
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