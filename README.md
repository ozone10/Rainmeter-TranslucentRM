# Rainmeter-TranslucentRM 
[![Build status](https://img.shields.io/github/workflow/status/ozone10/Rainmeter-TranslucentRM/Build/master?logo=Github)](https://github.com/ozone10/Rainmeter-TranslucentRM)
[![Build status](https://img.shields.io/appveyor/ci/ozone10/Rainmeter-TranslucentRM/master?logo=Appveyor)](https://ci.appveyor.com/project/ozone10/rainmeter-translucentrm)
[![Codacy Badge](https://img.shields.io/codacy/grade/d6ef2575cd244ad3b3b2dff4c9de3499?logo=Codacy)](https://www.codacy.com/manual/ozone10/Rainmeter-TranslucentRM?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=ozone10/Rainmeter-TranslucentRM&amp;utm_campaign=Badge_Grade)
[![Latest release](https://img.shields.io/github/v/release/ozone10/Rainmeter-TranslucentRM?include_prereleases)](https://github.com/ozone10/Rainmeter-TranslucentRM/releases/latest)
[![Total downloads](https://img.shields.io/github/downloads/ozone10/Rainmeter-TranslucentRM/total.svg)](https://github.com/ozone10/Rainmeter-TranslucentRM/releases)
[![Licence](https://img.shields.io/github/license/ozone10/Rainmeter-TranslucentRM?color=9cf)](https://www.gnu.org/licenses/gpl-3.0.en.html)

[Rainmeter](https://www.rainmeter.net) plugin that allows to create a blur/acrylic/transparent effect with color on the skin window or taskbar. This plugin has basic support for multimonitor setup. Minimal requirements is Windows 10. For acrylic option Windows 10 build 10.0.17134, 1803 (April 2018 update).

For more information, check the [Rainmeter forum](https://forum.rainmeter.net/viewtopic.php?f=128&p=165921).

---

## Options
This plugin has two measure mode one for taskbar and other for skin.

### General Options
Works in both modes.

* **Taskbar** - Option to choose plugin mode.
  * Value **0** - skin mode, default value.
  * Value **1** - taskbar mode.
* **Type** - Option to choose accent state.
  * Value **0** - disabled state, default value.
  * Value **1** - opaque, transparency is ignored.
  * Value **2** - transparent, border option is ignored.
  * Value **3** - blur, border option will disable color option.
  * Value **4** - acrylic, border option will disable color option. *Note 1*
  * Value **6** - transparent, color option is ignored.
* **Color** - Color in hex format RRGGBBAA or RRGGBB. Default value is 00000001.
* **AccentColor** - will use Windows 10 accent color, with transparency from color option (last 2 number - AA)

**Note 1:** acrylic state is broken in Windows 10 build 10.0.18362, 1903 (May 2019 Update) and later. This is Windows 10 bug.

### Skin Options
Must have option `Taskbar=0`.

* **Border** - Option to choose border around skin.
  * Value **0** - disable border, default value.
  * Value **1** - enable border.

### Taskbar Options
Must have option `Taskbar=1`
In taskbar mode plugin is working in parent-child measure style.

#### Parent Measure
* **Info** - Option to choose which information will be shown in log (Skin Tab).
  * Value **0** - show used accent state and plugin mode, default value.
  * Value **1** - show monitor count and used color in hex format RRGGBB.
* **SecondMonitor** - Option to enable support for 2+ monitor setup and allow to use child measures.
  * Value **0** - use only main monitor, default value.
  * Value **1** - enable support for 2+ monitor setup.

#### Child Measure
* **ParentName** - Name of parent measure. Parent measure must have option `SecondMonitor=1`.
* **Index** - Option to select second monitor.
  * Value **0** - disable measure, default value.
  * Value ***N*** - index of second monitors, it is in range from 0 to (number of monitor - 1).
  
---
## Examples
* **Example 1:**
Transparent skin with accent color.

```ini
[TranslucentSkin]
Measure=Plugin
Plugin=TranslucentRM
Type=2
Color=00000050
AccentColor=1
```


* **Example 2:**
Measure count monitor and accent color without changing skin and taskbar.

```ini
[Info]
Measure=Plugin
Plugin=TranslucentRM
Taskbar=1
AccentColor=1
Info=1
```

* **Example 3:**
Set only main taskbar.

```ini
[MainTaskbar]
Measure=Plugin
Plugin=TranslucentRM
Taskbar=1
Type=2
Color=6311B250
```


* **Example 4:**
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
ParentName=Taskbar
Index=1
Type=1
Color=A70013
```
