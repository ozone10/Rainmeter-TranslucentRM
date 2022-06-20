# Rainmeter-TranslucentRM

[![Build status](https://img.shields.io/github/workflow/status/ozone10/Rainmeter-TranslucentRM/Build/master?logo=Github)](https://github.com/ozone10/Rainmeter-TranslucentRM)
[![Build status](https://img.shields.io/appveyor/ci/ozone10/Rainmeter-TranslucentRM/master?logo=Appveyor)](https://ci.appveyor.com/project/ozone10/rainmeter-translucentrm)
[![Latest release](https://img.shields.io/github/v/release/ozone10/Rainmeter-TranslucentRM?include_prereleases)](https://github.com/ozone10/Rainmeter-TranslucentRM/releases/latest)
[![Total downloads](https://img.shields.io/github/downloads/ozone10/Rainmeter-TranslucentRM/total.svg)](https://github.com/ozone10/Rainmeter-TranslucentRM/releases)
[![Licence](https://img.shields.io/github/license/ozone10/Rainmeter-TranslucentRM?color=9cf)](https://www.gnu.org/licenses/gpl-3.0.en.html)

[Rainmeter](https://www.rainmeter.net) plugin that allows to create a blur/acrylic/transparent effect with color on the skin window or taskbar. This plugin has basic support for multimonitor setup.  
Minimal requirements is Windows 10. For acrylic option Windows 10 build 10.0.17134, 1803 (April 2018 update). For corner and border color options Windows 11 build 22000. For mica effects Windows 11 build 22621.

For more information, check the [Rainmeter forum](https://forum.rainmeter.net/viewtopic.php?f=128&p=165921).

Check my other Rainmeter plugin [DarkContextMenu](https://github.com/ozone10/Rainmeter-DarkContextMenu).

* * *

<p align="center">
<img src="https://i.imgur.com/aaIuWs1.png">
  <img src="https://i.imgur.com/jyUrTar.png">
  <img src="https://i.imgur.com/zYt37SA.png">
  <img src="https://i.imgur.com/EJGMumj.png">
  <img src="https://i.imgur.com/Qj7ePGZ.png">
</p>

* * *

## Options

This plugin has two measure mode one for taskbar and other for skin.

### General Options

Works in both modes.

- **Taskbar** - Option to choose plugin mode.

  - Value **0** - skin mode, default value.
  - Value **1** - taskbar mode.

- **Type** - Option to choose accent state.

  - Value **0** - disabled state, default value.
  - Value **1** - opaque, transparency is ignored.
  - Value **2** - transparent, border option is ignored in Windows 10.
  - Value **3** - blur, border option will disable color option in Windows 10. _Note 1_
  - Value **4** - acrylic, border option will disable color option in Windows 10. _Note 2_
  - Value **6** - transparent, color option is ignored.

- **Color** - Color in hex format RRGGBBAA or RRGGBB. Default value is 00000001.

- **AccentColor** - will use Windows 10 accent color, with transparency from color option (last 2 number - AA). In Windows 11 also affect border color.

**Note 1:** blur state is somehow broken in Windows 11 build 22000. This is Windows 11 bug.  
**Note 2:** acrylic state is broken in Windows 10 build 10.0.18362, 1903 (May 2019 Update) and later. This is Windows 10 bug. In Windows 11 build 22000 it seems to be fixed.

### Skin Options

Must have option `Taskbar=0`.

- **Border** - Option to choose border around skin.
  - Value **0** - disable border, default value.
  - Value **1** - enable border.

#### Windows 11 Options

- **Corner** - Option to choose corner type, also enable border around skin.
  - Value **0** - let system decide, usually not rounded corners.
  - Value **1** - don't round corners, default value.
  - Value **2** - round corners.
  - Value **3** - small round corners.

- **BorderColor** - Color in hex format RRGGBB. Corner option must have value 2 or 3.

- **Mica** - Option to choose backdrop material (mica) effect type, will disable `Type` option. _Note 3_
  - Value **0** - don't use mica, won't disable `Type` option, , default value.
  - Value **1** - let system decide, usually transparent background (no mica effect).
  - Value **2** - mica effect.
  - Value **3** - acrylic effect.
  - Value **4** - mica effect (found in tabbed apps).

**Note 3:** mica effects require window (here skin) to have focus, if there is no focus mica will use solid color. Current workaround is not perfect, and skin can start or revert back to solid color state.  
Mica is also affected by dark mode, which can be controled by my other Rainmeter plugin [DarkContextMenu](https://github.com/ozone10/Rainmeter-DarkContextMenu).

### Taskbar Options

Must have option `Taskbar=1`.  
In taskbar mode plugin is working in parent-child measure style.

#### Parent Measure

- **Info** - Option to choose which information will be shown in log (Skin Tab).

  - Value **0** - show used accent state and plugin mode, default value.
  - Value **1** - show monitor count and used color in hex format RRGGBB.

- **SecondMonitor** - Option to enable support for 2+ monitor setup and allow to use child measures.
  - Value **0** - use only main monitor, default value.
  - Value **1** - enable support for 2+ monitor setup. _Note 2_

#### Child Measure

- **ParentName** - Name of parent measure. Parent measure must have option `SecondMonitor=1`.

- **Index** - Option to select second monitor.
  - Value **0** - disable measure, default value.
  - Value **_N_** - index of second monitors, it is in range from 0 to (number of monitor - 1).  

**Note 2:** if child measure is not used, style from parent measure will be applied on all taskbars.

* * *

## Examples

- **Example 1:**
    Transparent skin with accent color.

```ini
[TranslucentSkin]
Measure=Plugin
Plugin=TranslucentRM
Type=2
Color=00000050
AccentColor=1
```

- **Example 2:**
    Measure count monitor and accent color without changing skin and taskbar.

```ini
[Info]
Measure=Plugin
Plugin=TranslucentRM
Taskbar=1
AccentColor=1
Info=1
```

- **Example 3:**
    Set only main taskbar.

```ini
[MainTaskbar]
Measure=Plugin
Plugin=TranslucentRM
Taskbar=1
Type=2
Color=6311B250
```

- **Example 4:**
    Set different style to taskbars.

```ini
[MainTaskbar]
Measure=Plugin
Plugin=TranslucentRM
Taskbar=1
SecondMonitor=1
Type=4
Color=0063B1A0

[SecondTaskbar]
Measure=Plugin
Plugin=TranslucentRM
ParentName=MainTaskbar
Index=1
Type=1
Color=A70013
```

- **Example 5:**
    Set taskbars to have same style. In this case transparent style.

```ini
[Taskbars]
Measure=Plugin
Plugin=TranslucentRM
Taskbar=1
SecondMonitor=1
Type=6
```

- **Example 6:**
    Acrylic skin with default color and transparency, round corners and gold border.

```ini
[RoundCorners]
Measure=Plugin
Plugin=TranslucentRM
Type=4
Corner=2
BorderColor=FFD700
```

- **Example 7:**
    Skin with standard mica effect, small round corners and silver blue border.

```ini
[Mica]
Measure=Plugin
Plugin=TranslucentRM
Mica=2
Corner=3
BorderColor=C4D4E0
```
