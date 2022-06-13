/*
  Copyright (C) 2019-2022 oZone
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "StdAfx.h"
#include "PluginTranslucentRM.h"

// Note: GetString, ExecuteBang and an unnamed function for use as a section variable
// have been commented out. Uncomment any functions as needed.
// For more information, see the SDK docs: https://docs.rainmeter.net/developers/plugin/cpp/

static bool validWinVersion = false;
static bool isWin11 = false;
static bool isWin11Mica = false;
static HMODULE hUser32 = nullptr;
static std::vector<ParentMeasure*> g_ParentMeasures;

inline bool IsAtLeastWin10Build(DWORD buildNumber)
{
    if (!IsWindows10OrGreater()) {
        return false;
    }

    const auto mask = VerSetConditionMask(0, VER_BUILDNUMBER, VER_GREATER_EQUAL);

    OSVERSIONINFOEXW osvi{};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwBuildNumber = buildNumber;
    return VerifyVersionInfo(&osvi, VER_BUILDNUMBER, mask) != FALSE;
}

void SetWindowAccent(const struct ChildMeasure* measure, HWND hWnd)
{
    const bool useMica = (measure->mica != DWMSBT_NONE) && isWin11Mica;
    if (validWinVersion &&
        !measure->parent->errorUser32 &&
        !useMica &&
        (!measure->parent->taskbar ||
            measure->accentState != AccentTypes::ACCENT_DISABLED))
    {
        using SWCA = bool (WINAPI*)(HWND hWnd, WINCOMPATTRDATA* wcaData);
        const auto _SetWindowCompositionAttribute = reinterpret_cast<SWCA>(GetProcAddress(hUser32, "SetWindowCompositionAttribute"));

        if (_SetWindowCompositionAttribute != nullptr) {
            ACCENTPOLICY policy = { measure->accentState, measure->flags, measure->color, 0 };
            WINCOMPATTRDATA data = { WCA_ACCENT_POLICY, &policy, sizeof(ACCENTPOLICY) };
            _SetWindowCompositionAttribute(hWnd, &data);
        }
    }
}

BOOL CALLBACK MonitorEnumProc(HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT /*lprcMonitor*/, LPARAM dwData)
{
    auto count = reinterpret_cast<int*>(dwData);
    (*count)++;
    return TRUE;
}

int CountMonitor(void* rm)
{
    int count = 0;
    if (EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&count)) != FALSE) {
        return count;
    }

    RmLogF(rm, LOG_DEBUG, L"Could not count monitors. Will default to %ld.", DEFAULT_MONITOR_COUNT);
    return DEFAULT_MONITOR_COUNT;
}

void EnumTaskbars(struct ParentMeasure* parentMeasure, void* rm)
{
    HWND tmpTaskbar = FindWindow(L"Shell_SecondaryTrayWnd", nullptr);
    if (tmpTaskbar == nullptr) {
        return;
    }

    parentMeasure->monitorCount = CountMonitor(rm);
    parentMeasure->taskbars.push_back(tmpTaskbar);
    int guardCounter = 0;
    while ((tmpTaskbar = FindWindowEx(nullptr, tmpTaskbar, L"Shell_SecondaryTrayWnd", L"")) != nullptr) {
        if (guardCounter >= (parentMeasure->monitorCount - 1)) {
            break;
        }
        parentMeasure->taskbars.push_back(tmpTaskbar);
        guardCounter++;
    }
}

void SetTaskbars(struct ParentMeasure* parentMeasure, bool enableAccent)
{
    for (auto taskbar : parentMeasure->taskbars) {
        if (enableAccent) {
            SetWindowAccent(parentMeasure->ownerChild, taskbar);
        }
        else {
            SendMessage(taskbar, WM_THEMECHANGED, NULL, NULL);
        }
    }
}

void SetColor(struct Measure* measure, void* rm, bool isBorderColor)
{
    auto* color = isBorderColor ? &measure->colorBorder : &measure->color;
    *color = MIN_TRANPARENCY;

    auto optionStr = isBorderColor ? L"BorderColor" : L"Color";
    std::wstring colorStr = RmReadString(rm, optionStr, L"00000001", TRUE);
    if (_wcsnicmp(colorStr.c_str(), L"-", 1) == 0U) {
        colorStr = colorStr.substr(1, colorStr.size());
    }

    if (_wcsnicmp(colorStr.c_str(), L"0x", 2) == 0U) {
        colorStr = colorStr.substr(2, colorStr.size());
    }

    constexpr size_t RGBAStrSize = 8;
    constexpr size_t RGBStrSize = 6;
    const size_t strSize = colorStr.size();

    if (strSize > RGBAStrSize || strSize < RGBStrSize) {
        measure->warn = true;
        *color = MIN_TRANPARENCY;
        return;
    }

    try {
        constexpr int hexBase = 16;
        *color = std::stoul(colorStr, nullptr, hexBase);
    }
    catch (const std::invalid_argument&) {
        measure->warn = true;
        *color = MIN_TRANPARENCY;
        return;
    }

    constexpr uint32_t bitwiseShift4 = 4;
    constexpr uint32_t bitwiseShift8 = 8;
    constexpr uint32_t bitwiseShift24 = 24;

    constexpr uint32_t maskAA = 0xFF;
    constexpr uint32_t maskBB = 0xFF00;
    constexpr uint32_t maskGG = 0xFF0000;
    constexpr uint32_t maskRR = 0xFF000000;

    if (strSize < RGBAStrSize) {
        *color <<= bitwiseShift4;
        if (strSize == RGBStrSize) {
            *color <<= bitwiseShift4;
        }
    }

    measure->accentColor = RmReadInt(rm, L"AccentColor", 0) > 0;
    if (measure->accentColor &&
        measure->accentState != AccentTypes::ACCENT_ENABLE_BLURBEHIND)
    {
        GetAccentColor(measure, isBorderColor);
    }

    wchar_t buffer[RGBStrSize + 1];
    swprintf_s(buffer, L"%06X", (*color >> bitwiseShift8));
    measure->hexColor = buffer;

    // Transform RRGGBBAA to AABBRRGG
    *color = (((*color & maskAA) << bitwiseShift24) |
        ((*color & maskRR) >> bitwiseShift24) |
        ((*color & maskBB) << bitwiseShift8) |
        ((*color & maskGG) >> bitwiseShift8));
}

void GetAccentColor(struct Measure* measure, bool isBorderColor)
{
    if (!measure->errorDwmapi && validWinVersion) {
        const HMODULE hDwmapi = LoadLibraryEx(L"dwmapi.dll", nullptr, LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32);

        if (hDwmapi == nullptr) {
            measure->errorDwmapi = true;
            return;
        }

        using DGCP = HRESULT (WINAPI*)(DWMCOLORIZATIONPARAMS * colorParams);
        const auto _DwmGetColorizationParameters = reinterpret_cast<DGCP>(GetProcAddress(hDwmapi, MAKEINTRESOURCEA(127)));

        if (_DwmGetColorizationParameters != nullptr) {
            struct DWMCOLORIZATIONPARAMS params;
            _DwmGetColorizationParameters(&params);

            constexpr uint32_t bitwiseShift8 = 8;
            constexpr uint32_t maskAA = 0xFF;
            constexpr uint32_t maskColor = 0xFFFFFF;
            constexpr uint32_t maskColor2 = 0xFFFFFF00;

            auto colorMeasure = isBorderColor ? &measure->colorBorder : &measure->color;

            //use user defined alpha, ignore it for border color
            //transform AARRGGBB format to RRGGBBAA
            auto colorTmp = (*colorMeasure & maskAA) | ((params.dmwColor & maskColor) << bitwiseShift8);
            *colorMeasure = isBorderColor ? (colorTmp & maskColor2) : colorTmp;
        }
        FreeLibrary(hDwmapi);
    }
}

void InitColor(struct Measure* measure, void* rm)
{
    SetColor(measure, rm, false);

    if (RmReadInt(rm, L"DisableColorWarning", 0) > 0) {
        measure->warn = false;
    }
    if (measure->warn) {
        RmLog(rm, LOG_WARNING, L"Wrong format for option color. Color is in hex format RRGGBBAA or RRGGBB.");
    }
}

void SetType(struct Measure* measure, void* rm)
{
    const int type = RmReadInt(rm, L"Type", 0);
    switch (type) {
    case 1:
        measure->accentState = AccentTypes::ACCENT_ENABLE_GRADIENT;
        break;
    case 2:
        measure->accentState = AccentTypes::ACCENT_ENABLE_TRANSPARENTGRADIENT;
        break;
    case 3:
        measure->accentState = AccentTypes::ACCENT_ENABLE_BLURBEHIND;
        SetMinTransparency(measure);
        break;
    case 4:
        measure->accentState = AccentTypes::ACCENT_ENABLE_ACRYLICBLURBEHIND;
        SetMinTransparency(measure);
        break;
    case 5:
        measure->accentState = AccentTypes::ACCENT_ENABLE_HOSTBACKDROP;
        break;
    case 6:
        measure->accentState = AccentTypes::ACCENT_ENABLE_TRANSPARENT;
        break;
    default:
        measure->accentState = AccentTypes::ACCENT_DISABLED;
    }

    measure->usedState = measure->accentState;
}

void SetMinTransparency(struct Measure* measure)
{
    constexpr uint32_t onlyColor = 0xFFFFFF;
    if (measure->color <= onlyColor) {
        measure->color += MIN_TRANPARENCY;
    }
}

void SetBorder(struct Measure* measure)
{
    if ((measure->border &&
        measure->accentState != AccentTypes::ACCENT_ENABLE_TRANSPARENTGRADIENT) ||
        (isWin11 &&
            (measure->corner == DWMWCP_ROUND ||
                measure->corner == DWMWCP_ROUNDSMALL)))
    {
        measure->flags = BORDER;
    }
    else {
        measure->flags = 2;
    }
}

void SetBorderColor(struct Measure* measure, void* rm)
{
    if (isWin11 &&
    (measure->corner == DWMWCP_ROUND ||
        measure->corner == DWMWCP_ROUNDSMALL))
        {
            SetColor(measure, rm, true);
            const COLORREF color = measure->colorBorder;
            DwmSetWindowAttribute(RmGetSkinWindow(rm), DWMWA_BORDER_COLOR, &color, sizeof(color));
        }
}

void SetCorner(struct Measure* measure, void* rm)
{
    const int cornerType = RmReadInt(rm, L"Corner", 1);
    if (isWin11) {
        switch (cornerType) {
        case 0:
            measure->corner = DWMWCP_DEFAULT;
            break;
        case 2:
            measure->corner = DWMWCP_ROUND;
            measure->flags = BORDER;
            break;
        case 3:
            measure->corner = DWMWCP_ROUNDSMALL;
            measure->flags = BORDER;
            break;
        default:
            measure->corner = DWMWCP_DONOTROUND;
        }

        DwmSetWindowAttribute(RmGetSkinWindow(rm), DWMWA_WINDOW_CORNER_PREFERENCE, &measure->corner, sizeof(measure->corner));
    }
}

void SetMica(struct Measure* measure, void* rm)
{
    const int type = RmReadInt(rm, L"Mica", 0);
    if (isWin11Mica) {

        switch (type) {
        case 1:
            measure->mica = DWMSBT_AUTO;
            measure->accentState = AccentTypes::ACCENT_DISABLED;
            break;
        case 2:
            measure->mica = DWMSBT_MAINWINDOW;
            measure->accentState = AccentTypes::ACCENT_DISABLED;
            break;
        case 3:
            measure->mica = DWMSBT_TRANSIENTWINDOW;
            measure->accentState = AccentTypes::ACCENT_DISABLED;
            break;
        case 4:
            measure->mica = DWMSBT_TABBEDWINDOW;
            measure->accentState = AccentTypes::ACCENT_DISABLED;
            break;
        default:
            measure->mica = DWMSBT_NONE;
        }

        const MARGINS margins = { -1, -1, -1, -1};
        auto hwnd = RmGetSkinWindow(rm);
        DwmExtendFrameIntoClientArea(hwnd, &margins);
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &measure->mica, sizeof(measure->mica));
    }
    else if (isWin11) {
        
        const MARGINS margins = { -1, -1, -1, -1 };
        auto hwnd = RmGetSkinWindow(rm);
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        BOOL useMica = type > 0 ? TRUE : FALSE;
        DwmSetWindowAttribute(RmGetSkinWindow(rm), DWMWA_MICA_EFFECT, &useMica, sizeof(useMica));
    }
}

void InitParentMeasure(struct ParentMeasure* parentMeasure, void* rm)
{
    auto parent = parentMeasure->ownerChild;
    hUser32 = LoadLibraryEx(L"user32.dll", nullptr, LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUser32 == nullptr) {
        parentMeasure->errorUser32 = true;
        return;
    }

    InitColor(parent, rm);
    SetType(parent, rm);

    parentMeasure->taskbar = RmReadInt(rm, L"Taskbar", 0) > 0;
    if (parentMeasure->taskbar) {
        parent->altInfo = RmReadInt(rm, L"Info", 0) > 0;
        parentMeasure->taskbars.push_back(FindWindowW(L"Shell_TrayWnd", nullptr));
        parentMeasure->sndMonitor = RmReadInt(rm, L"SecondMonitor", 0) > 0;
        EnumTaskbars(parentMeasure, rm);
        parent->flags = 2;

    }
    else {
        parent->handle = RmGetSkinWindow(rm);
        parent->border = RmReadInt(rm, L"Border", 0) > 0;
        SetBorder(parent);
        SetCorner(parent, rm);
        SetMica(parent, rm);
        SetWindowAccent(parent, parent->handle);
        SetBorderColor(parent, rm);
    }

    CheckFeaturesSupport(parent, rm);
}

void InitChildMeasure(struct ChildMeasure* childMeasure, void* rm)
{
    auto parent = childMeasure->parent;

    if (parent != nullptr &&
        parent->ownerChild != childMeasure &&
        parent->taskbar &&
        parent->sndMonitor)
    {
        childMeasure->taskbarIdx = RmReadInt(rm, L"Index", 0);
        if (childMeasure->taskbarIdx < 0 || childMeasure->taskbarIdx >= childMeasure->parent->monitorCount) {
            childMeasure->taskbarIdx = 0;
            RmLogF(rm, LOG_WARNING, L"Could not get taskbar. Index should be in range from 0 to %ld.", (childMeasure->parent->monitorCount - 1));
        }

        if (childMeasure->taskbarIdx == 0) {
            childMeasure->handle = nullptr;
        }
        else {
            childMeasure->handle = parent->taskbars.at(childMeasure->taskbarIdx);
            SetType(childMeasure, rm);
        }
    }
}

void CheckFeaturesSupport(struct Measure* measure, void* rm)
{
    const bool useAcrylic = measure->accentState == AccentTypes::ACCENT_ENABLE_ACRYLICBLURBEHIND;

    if (useAcrylic) {
        if (IsAtLeastWin10Build(BUILD_1903) && !isWin11) {
            RmLog(rm, LOG_DEBUG, L"On Windows 10 1903 (May 2019 update, 10.0.18362) and later when using acrylic (Type=4), dragging skin will be slow. Fixed in Windows 11 build 22000?");
        }

        if (!IsAtLeastWin10Build(BUILD_1803)) {
            measure->accentState = AccentTypes::ACCENT_ENABLE_BLURBEHIND;
            RmLog(rm, LOG_WARNING, L"Acrylic is not supported, need at least Windows 10 1803 (April 2018 update, 10.0.17134). Will use blur instead.");
        }
    }
    else if (!isWin11Mica) {
        if (RmReadInt(rm, L"Mica", 0) > 0) {
            RmLog(rm, LOG_DEBUG, L"Mica transparency is properly supported only on Windows 11 build 22621 and later.");
        }
    }
    else if (isWin11) {
        if (measure->accentState == AccentTypes::ACCENT_ENABLE_BLURBEHIND) {
            RmLog(rm, LOG_DEBUG, L"On Windows 11 build 22000 when using blur (Type=3), dragging skin can be slow.");
        }
    }
    else {
        if (measure->corner == DWMWCP_ROUND || measure->corner == DWMWCP_ROUNDSMALL) {
            RmLog(rm, LOG_DEBUG, L"Rounded corners are supported only on Windows 11 build 22000 and later.");
        }
    }
}

void CheckErrors(const struct ChildMeasure* childMeasure)
{
    if (childMeasure->parent == nullptr) {
        RmLog(LOG_ERROR, L"Invalid \"ParentName\"");
        return;
    }

    if (childMeasure->parent->errorUser32) {
        RmLog(LOG_ERROR, L"TranslucentRM plugin could not load library user32.dll.");
    }

    if (childMeasure->errorDwmapi) {
        RmLog(LOG_WARNING, L"TranslucentRM plugin could not load library dwmapi.dll. Accent color and rounded corners will not be applied.");
    }
}

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
    if (!IsWindows10OrGreater()) {
        return;
    }
    validWinVersion = true;

    isWin11Mica = IsAtLeastWin10Build(BUILD_22H2);
    isWin11 = isWin11Mica ? true : IsAtLeastWin10Build(BUILD_WIN11);

    auto child = new ChildMeasure;
    *data = child;

    // Initialize Parent measure or Set Child measure
    void* skin = RmGetSkin(rm);
    LPCWSTR parentName = RmReadString(rm, L"ParentName", L"", 1);
    if (*parentName == 0U) {
        child->parent = new ParentMeasure;
        child->parent->name = RmGetMeasureName(rm);
        child->parent->skin = skin;
        child->parent->ownerChild = child;
        g_ParentMeasures.push_back(child->parent);
        InitParentMeasure(child->parent, rm);
    }
    else {
        // Find parent using name AND the skin handle to be sure that it's the right one
        for (auto parentMeasure : g_ParentMeasures) {
            if (_wcsicmp(parentMeasure->name, parentName) == 0 &&
                parentMeasure->skin == skin &&
                parentMeasure->taskbar &&
                parentMeasure->sndMonitor)
            {
                child->parent = parentMeasure;
                child->parent->sameStyle = false;
                child->flags = 2;
                child->altInfo = RmReadInt(rm, L"Info", 0) > 0;
                break;
            }
        }
        if (child->parent == nullptr) {
            RmLog(rm, LOG_ERROR, L"Invalid \"ParentName\"");
            return;
        }
    }
    InitChildMeasure(child, rm);
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* /*maxValue*/)
{
    if (!validWinVersion || hUser32 == nullptr) {
        return;
    }

    auto child = static_cast<ChildMeasure*>(data);
    auto parent = child->parent;

    if (parent != nullptr && parent->taskbar) {
        InitColor(child, rm);
    }
}

PLUGIN_EXPORT double Update(void* data)
{
    if (!validWinVersion) {
        return -1;
    }

    auto child = static_cast<ChildMeasure*>(data);
    auto parent = child->parent;

    if (parent == nullptr) {
        return -1;
    }

    if (parent->ownerChild == child) {
        if (parent->taskbar) {
            if (parent->sndMonitor && parent->sameStyle) {
                SetTaskbars(parent, true);
            }
            else {
                SetWindowAccent(child, parent->taskbars.at(0));
            }
        }
    }
    else {
        if (parent->sndMonitor) {
            SetWindowAccent(child, child->handle);
        }
    }

    if (child->altInfo) {
        return static_cast<double>(parent->monitorCount);
    }
    return static_cast<double>(child->accentState);
}

PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
    if (!validWinVersion) {
        return L"Usupported.";
    }

    auto child = static_cast<ChildMeasure*>(data);
    auto parent = child->parent;

    if (parent == nullptr) {
        return L"Error";
    }

    if (parent->ownerChild == child) {
        if (child->altInfo) {
            return child->hexColor.c_str();
        }
        if (parent->taskbar) {
            return L"Taskbar";
        }
        return L"Skin";
    }

    if (child->altInfo) {
        return child->hexColor.c_str();
    }
    if (parent->sndMonitor) {
        return L"SecondaryTaskbar";
    }
    return L"SecondaryTaskbar (Disabled)";
}

//PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
//{
//    Measure* measure = static_cast<Measure*>(data);
//}

//PLUGIN_EXPORT LPCWSTR (void* data, const int argc, const WCHAR* argv[])
//{
//    Measure* measure = static_cast<Measure*>(data);
//    return nullptr;
//}

PLUGIN_EXPORT void Finalize(void* data)
{
    if (!validWinVersion) {
        RmLog(LOG_WARNING, L"TranslucentRM plugin is supported only on Windows 10 and 11.");
        return;
    }

    auto child = static_cast<ChildMeasure*>(data);
    auto parent = child->parent;

    if (parent != nullptr &&
        parent->ownerChild == child)
    {
        if (!child->parent->errorUser32) {
            child->accentState = AccentTypes::ACCENT_DISABLED;
            child->flags = 0;

            if (parent->taskbar) {
                if (parent->sndMonitor) {
                    SetTaskbars(parent, false);
                }
                else {
                    SendMessage(parent->taskbars.at(0), WM_THEMECHANGED, NULL, NULL);
                }
                std::vector<HWND>().swap(parent->taskbars);
            }
            else {
                SetWindowAccent(child, child->handle);
            }
            FreeLibrary(hUser32);
        }
        hUser32 = nullptr;
        std::vector<ParentMeasure*>().swap(g_ParentMeasures);
        delete parent;
    }

    CheckErrors(child);
    delete child;
}
