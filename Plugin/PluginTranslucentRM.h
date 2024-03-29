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

#pragma once

// Won't support outdated insider builds
//constexpr DWORD MIN_FLUENT_BUILD = 17063;

constexpr DWORD BUILD_1803 = 17134; // Windows 10 1803 (April 2018 Update)
constexpr DWORD BUILD_1903 = 18362; // Windows 10 1903 (May 2019 Update)

constexpr DWORD BUILD_WIN11 = 22000; // Windows 11 first "stable" build
constexpr DWORD BUILD_22H2 = 22621; // Windows 11 22H2 first to support mica properly

constexpr uint32_t MIN_TRANPARENCY = 0x01000000;

constexpr int32_t BORDER = 0x1E0; //0x20U | 0x40U | 0x80U | 0x100U;

constexpr uint32_t WCA_ACCENT_POLICY = 19;

constexpr uint32_t DWMWA_MICA_EFFECT = 1029; // Windows 11 Mica

constexpr int DEFAULT_MONITOR_COUNT = 5;

enum class AccentTypes : int {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_ENABLE_TRANSPARENT = 6
};

struct ACCENTPOLICY {
    AccentTypes nAccentState = AccentTypes::ACCENT_DISABLED;
    int32_t nFlags = 0;
    uint32_t nColor = 0;
    int32_t nAnimationId = 0;
};

struct WINCOMPATTRDATA {
    uint32_t nAttribute = 0;
    void* pData = nullptr;
    uint32_t ulDataSize = 0;
};

struct DWMCOLORIZATIONPARAMS {
    COLORREF dmwColor = 0;
    COLORREF dmwAfterglow = 0;
    DWORD dmwColorBalance = 0;
    DWORD dmwAfterglowBalance = 0;
    DWORD dmwBlurBalance = 0;
    DWORD dmwGlassReflectionIntensity = 0;
    BOOL dmwOpaqueBlend = FALSE;
};

struct Measure {
    AccentTypes accentState = AccentTypes::ACCENT_DISABLED;
    AccentTypes usedState = AccentTypes::ACCENT_DISABLED;
    HWND handle = nullptr;
    uint32_t color = MIN_TRANPARENCY;
    std::wstring hexColor = L"";
    int32_t flags = 0;
    bool border = false;
    bool accentColor = false;
    bool altInfo = false;
    bool warn = false;
    bool errorDwmapi = false;

    DWM_SYSTEMBACKDROP_TYPE mica = DWMSBT_NONE;

    uint32_t colorBorder = 0x002B2B2B;
    DWM_WINDOW_CORNER_PREFERENCE corner = DWMWCP_DONOTROUND;
};

struct ChildMeasure;

struct ParentMeasure {
    void* skin = nullptr;
    LPCWSTR name = nullptr;
    ChildMeasure* ownerChild = nullptr;

    std::vector<HWND> taskbars;
    int monitorCount = 0;
    bool taskbar = false;
    bool sndMonitor = false;
    bool sameStyle = true;
    bool errorUser32 = false;
};

struct ChildMeasure : public Measure {
    ParentMeasure* parent = nullptr;
    int taskbarIdx = 0;
};

inline bool IsAtLeastWin10Build(DWORD buildNumber);

void SetWindowAccent(const struct ChildMeasure* measure, HWND hWnd);

inline BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
inline int CountMonitor(void* rm);
void EnumTaskbars(struct ParentMeasure* parentMeasure, void* rm);
void SetTaskbars(struct ParentMeasure* parentMeasure, bool enableAccent);

void SetColor(struct Measure* measure, void* rm, bool isBorderColor);
inline void GetAccentColor(struct Measure* measure, bool isBorderColor);
inline void InitColor(struct Measure* measure, void* rm);
inline void SetType(struct Measure* measure, void* rm);
inline void SetMinTransparency(struct Measure* measure);
inline void SetBorder(struct Measure* measure);
void SetBorderColor(struct Measure* measure, void* rm);
void SetCorner(struct Measure* measure, void* rm);
void InitMica(struct Measure* measure, void* rm);
void SetMica(struct Measure* measure, HWND hwnd);

void InitParentMeasure(struct ParentMeasure* parentMeasure, void* rm);
void InitChildMeasure(struct ChildMeasure* childMeasure, void* rm);

void CheckFeaturesSupport(struct Measure* measure, void* rm);
void CheckErrors(const struct ChildMeasure* childMeasure);
