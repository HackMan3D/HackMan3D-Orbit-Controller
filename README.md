# HackMan3D Orbit Controller

[![Latest release](https://img.shields.io/github/v/release/HackMan3D/HackMan3D-Orbit-Controller?style=flat-square&label=Release&color=0A84FF)](https://github.com/HackMan3D/HackMan3D-Orbit-Controller/releases/latest)
[![License: AGPL-3.0](https://img.shields.io/badge/License-AGPL--3.0-EA6D2F?style=flat-square)](LICENSE)
![Platforms](https://img.shields.io/badge/Platforms-Windows%20%7C%20macOS%20%7C%20Linux-0A84FF?style=flat-square)
![Hardware](https://img.shields.io/badge/Hardware-Arduino%20Pro%20Micro-00979D?style=flat-square&logo=arduino&logoColor=white)
![Controller](https://img.shields.io/badge/Controller-6--DOF-7B61FF?style=flat-square)
![Sensors](https://img.shields.io/badge/Joysticks-Hall%20Effect-00A6A6?style=flat-square)
![Open source](https://img.shields.io/badge/Open%20Source-Yes-39A845?style=flat-square)
[![GitHub stars](https://img.shields.io/github/stars/HackMan3D/HackMan3D-Orbit-Controller?style=flat-square&logo=github&label=Stars)](https://github.com/HackMan3D/HackMan3D-Orbit-Controller/stargazers)
[![Downloads](https://img.shields.io/github/downloads/HackMan3D/HackMan3D-Orbit-Controller/total?style=flat-square&logo=github&label=Downloads&color=0A84FF&cacheSeconds=300)](https://github.com/HackMan3D/HackMan3D-Orbit-Controller/releases)
[![Views](https://hits.sh/github.com/HackMan3D/HackMan3D-Orbit-Controller.svg?style=flat-square&label=Views&color=0A84FF)](https://github.com/HackMan3D/HackMan3D-Orbit-Controller)

<p align="center">
  <img src="images/43.jpg" alt="HackMan3D Orbit Controller" width="900">
</p>

An open-source, fully 3D-printable 6-DOF navigation controller built around an Arduino Pro Micro and four Hall-effect joysticks. Orbit provides native 3D-controller input for CAD applications and an optional mouse-emulation mode for slicers.

## Start here

1. Download the ready-to-use package from the [latest release](https://github.com/HackMan3D/HackMan3D-Orbit-Controller/releases/latest).
2. Print the parts from [Creality Cloud](https://www.crealitycloud.com/model-detail/hackman3d-orbit-controller) or [MakerWorld](https://makerworld.com/en/models/3009119).
3. Order the components listed in the [Bill of Materials](Documentation/HackMan3D_Orbit_Controller_BOM.pdf).
4. Follow the [Assembly Guide](Documentation/HackMan3D_Orbit_Controller_Assembly_Guide.pdf) and [wiring diagram](Wiring/Orbit_Controller_Wiring_Diagram.png).
5. Install the [NavCore 3D Controller board package](https://github.com/NavCoree/3D-controller-Board-package) in Arduino IDE.
6. Open `Firmware/Hackman3D_Orbit_Controller/Hackman3D_Orbit_Controller.ino`, select the Arduino Pro Micro target, and upload it.
7. Keep the controller untouched for one second after connecting it while the joysticks calibrate.

The complete assembly video is available on [YouTube](https://www.youtube.com/watch?v=l8wjwXb5LGA).

## Features

- Six degrees of freedom: translation and rotation on three axes
- Four Hall-effect joysticks for contactless position sensing
- Three adjustable speed profiles
- Configurable dead zones, gain, smoothing, response curves, and axis inversion
- Multi-axis input with optional dominant-axis filtering
- Three configurable shortcut buttons
- Optional slicer mouse-emulation mode
- Native USB HID operation; no Orbit application is required
- Windows, macOS, and Linux support

## Compatibility

Orbit works with applications that support 3Dconnexion-style HID controllers, including:

| Category | Tested or commonly supported applications |
|---|---|
| CAD and 3D | Fusion 360, Blender, SolidWorks, FreeCAD, Onshape, Autodesk Inventor, Rhino |
| Slicers | Bambu Studio, Cura, PrusaSlicer |
| Operating systems | Windows, macOS, Linux |

Application support can vary by version and operating system. Slicers without native 3D-mouse support can use Orbit's optional mouse-emulation mode.

## Hardware

- 1× Arduino Pro Micro, ATmega32U4, 5 V / 16 MHz, USB-C
- 4× [JH16 Hall-effect joystick modules](https://www.aliexpress.com/item/1005013045830245.html)
- 3× optional mechanical keyboard switches
- Female-to-female Dupont wires, approximately 15 cm
- USB-C data cable
- 3D-printed parts
- Fasteners:
  - 4× M2×10 countersunk
  - 6× M2×6 countersunk
  - 4× M3×6 socket head
  - 2× M3×8 socket head
  - 4× M3×10 socket head
  - 1× M3×12 socket head

Check the [Bill of Materials](Documentation/HackMan3D_Orbit_Controller_BOM.pdf) before ordering.

## Controls

### CAD mode

- Move and twist the knob for translation and rotation.
- Press all three buttons together to cycle through the speed profiles.

### Slicer mouse mode

Hold buttons 2 and 3 together to switch between CAD mode and slicer mouse mode.

| Button | Short press | Long press |
|---|---|---|
| 1 | Tab | Command + Shift + G |
| 2 | N | L |
| 3 | Command + 0 | A |

The default shortcuts target macOS workflows and can be changed in the firmware. Pressing all three buttons still changes the speed profile.

## Tuning and troubleshooting

The controller calibrates itself every time it starts. Place it on a stable surface, do not touch the knob, and wait about one second after connecting it.

If an axis drifts, feels too sensitive, moves in the wrong direction, or needs a different response curve, use the [Tuning Guide](TUNING_GUIDE.md). All user-adjustable values are grouped near the top of the firmware.

For problems:

1. Review the [open issues](https://github.com/HackMan3D/HackMan3D-Orbit-Controller/issues).
2. Confirm that the USB cable supports data.
3. Confirm the selected Arduino board and port.
4. Open a [bug report](https://github.com/HackMan3D/HackMan3D-Orbit-Controller/issues/new?template=bug_report.yml) with the requested diagnostic information.

## Repository contents

| Path | Contents |
|---|---|
| `Firmware/` | Arduino source code |
| `Documentation/` | Assembly guide and Bill of Materials |
| `Wiring/` | Pinout and wiring diagrams |
| `images/` | Project media |
| `TUNING_GUIDE.md` | Calibration and response settings |
| `CHANGELOG.md` | Published version history |

## Contributing

Bug fixes, documentation improvements, hardware feedback, and tested enhancements are welcome. Read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request.

Please report security-sensitive problems according to [SECURITY.md](SECURITY.md), rather than in a public issue.

## License

The repository is licensed under the [GNU Affero General Public License v3.0](LICENSE). Printable model platforms may provide their files under separate terms; check the license shown on the relevant model page before redistributing them.

Orbit is an independent community project and is not affiliated with or endorsed by 3Dconnexion.

## Credits and support

Designed and maintained by **HackMan3D**.

Additional features, testing, and feedback by [Kitek](https://www.crealitycloud.com/user/7734397320).

Special thanks to [NavCore](https://github.com/NavCoree/3D-controller-Board-package) for the Arduino board package that enables native 3D-controller compatibility on the Pro Micro.

If Orbit is useful to you, star the repository, share your build, and help other makers by reporting your tested hardware and software combinations.
