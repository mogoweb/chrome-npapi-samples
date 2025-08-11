#include "pch.h"
#include <windows.h>
#include <string>
#include <sstream>

void WriteDebugLog(const char* message) {
    std::ostringstream oss;
    oss << "[NPAPIDemo] " << message;
    OutputDebugStringA(oss.str().c_str());
}
