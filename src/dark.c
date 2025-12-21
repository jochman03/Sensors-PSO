#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>

void enableDarkTitlebar(const char* windowTitle){
    HWND hwnd = FindWindowA(NULL, windowTitle);
    if (!hwnd) return;

    BOOL on = TRUE;
    int attr = 20;
    if (FAILED(DwmSetWindowAttribute(hwnd, attr, &on, sizeof(on)))) {
        attr = 19;
        DwmSetWindowAttribute(hwnd, attr, &on, sizeof(on));
    }
}