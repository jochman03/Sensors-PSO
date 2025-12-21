#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#undef LoadImage

#include "winfile.h"

const char* OpenBMPFileDialog(void){
    static char filename[MAX_PATH] = "";
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)){
        return filename;
    }

    return NULL;
}

const char* SaveImageFileDialog(void){
    static char filename[MAX_PATH] = "output.png";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();

    ofn.lpstrFilter =
        "PNG Image (*.png)\0*.png\0"
        "Bitmap Image (*.bmp)\0*.bmp\0"
        "All Files (*.*)\0*.*\0";

    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    ofn.lpstrDefExt = "png";

    if (GetSaveFileNameA(&ofn))
        return filename;
    return NULL;
}

const char* SaveJSONFileDialog(void){
    static char filename[MAX_PATH] = "solution.json";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();

    ofn.lpstrFilter =
        "JSON File (*.json)\0*.json\0"
        "All Files (*.*)\0*.*\0";

    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    ofn.lpstrDefExt = "json";

    if (GetSaveFileNameA(&ofn)){
        return filename;
    }
        
    return NULL;
}

const char* OpenJSONFileDialog(void){
    static char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFilter =
        "JSON File (*.json)\0*.json\0"
        "All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileNameA(&ofn)) {
        return filename;
    }
    return NULL;
}

const char* SaveSettingsJSONFileDialog(void){
    static char filename[MAX_PATH] = "settings.json";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFilter =
        "JSON File (*.json)\0*.json\0"
        "All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    ofn.lpstrDefExt = "json";

    if (GetSaveFileNameA(&ofn)) {
        return filename;
    }
    return NULL;
}