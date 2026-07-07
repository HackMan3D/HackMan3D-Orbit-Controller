![License](https://img.shields.io/github/license/HackMan3D/HackMan3D-Orbit-Controller)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)
![Arduino](https://img.shields.io/badge/Arduino-Pro%20Micro-00979D?logo=arduino)
![Open Source](https://img.shields.io/badge/Open%20Source-Yes-brightgreen)
[![GitHub stars](https://img.shields.io/github/stars/HackMan3D/HackMan3D-Orbit-Controller?style=social)](https://github.com/HackMan3D/HackMan3D-Orbit-Controller)

# Hackman3D Orbit Controller

<p align="center">
  <img src="images/43.jpg" alt="Hackman3D Orbit Controller" width="900">
</p>

An open-source DIY 6-DOF navigation controller built with an Arduino Pro Micro and Hall Effect joysticks.

This repository contains everything needed to build your own controller, including the firmware, Bill of Materials, wiring diagrams, board files, and documentation.

> **This is my first large open-source hardware project after nearly four months of development. I've been using this controller daily for the past two months and now consider it ready to share with the maker community. Documentation and tutorials will continue to improve thanks to your feedback.**

---

## Features

* 6 Degrees of Freedom (6-DOF)
* Hall Effect joystick technology
* Arduino Pro Micro based firmware
* Adjustable speed profiles and response curve
* Configurable multi-axis movement filtering
* Optional slicer mouse emulation mode
* USB-C connectivity
* Fully 3D printable design
* Optional mechanical shortcut buttons
* Open-source hardware and firmware
* Compatible with Windows, macOS, and Linux

---

## Repository Contents

| Folder | Description |
|--------|-------------|
| Firmware | Arduino source code |
| BOM | Complete Bill of Materials |
| Wiring | Wiring diagrams |
| Documentation | Assembly and installation guides |

---

## Hardware Requirements

* Arduino Pro Micro (ATmega32U4) USB-C 5v/16mhz
* 4× Hall Effect joystick modules (JH16 joystick hall effecct)
* Optional mechanical keyboard switches (3 Mechanical Switchs)
* Dupont wires female to female 15cm
* Standard metric screws (M2 Countersunk and M3 Socket Head)
* 4x M2x10 / 6x M2x6 / 4x M3x6 / 2x M3x8 / 4x M3x10 / 1x M3x12
* USB-C cable (1m or 2m)
* 3D printed parts

A complete list of components is available in the **BOM** pdf.

---

## Software Requirements

* Arduino IDE
* Required board package
* 3Dconnexion Driver (Windows, macOS, Linux)

---

## Compatible Applications

Works with most software supporting 3Dconnexion devices, including:

- Fusion 360
- Blender
- SolidWorks
- FreeCAD
- Onshape
- Autodesk Inventor
- Rhino
- Bambu Studio
- Cura
- PrusaSlicer
- and many more.

For slicers with limited native 3D mouse support, the firmware also includes an optional mouse emulation mode.
This mode sends mouse drag, wheel zoom, and configurable keyboard shortcuts instead of 3Dconnexion movement reports.

---

## Slicer Mouse Mode

Hold buttons `2 + 3` together to switch between normal CAD mode and slicer mouse mode.

Default slicer shortcuts:

| Button | Short press | Long press |
|--------|-------------|------------|
| 1 | Tab | Cmd + Shift + G |
| 2 | N | L |
| 3 | Cmd + 0 | A |

Pressing all three buttons still changes the speed profile.

---

## Operating System Compatibility

| Operating System | Status |
|------------------|--------|
| Windows | ✅ Supported |
| macOS | ✅ Supported |
| Linux | ✅ Supported |

---

## Getting Started

1. Print all required parts.
2. Purchase the components listed in the BOM.
3. Assemble the controller following the documentation.
4. Upload the firmware to the Arduino Pro Micro.
5. Install the required driver.
6. Enjoy your new DIY 3D navigation controller!

---

## STL Files

The complete set of printable STL files is available on Creality Cloud and makerworld :

👉 **https://www.crealitycloud.com/model-detail/hackman3d-orbit-controller**

👉 **https://makerworld.com/fr/models/3009119**

If you enjoy the project, don't hesitate to leave a ❤️, download the files, and share your makes!

---

## Open Source Philosophy

This project is intended to be built, modified and improved by the community.

Feel free to fork it, adapt it to your needs, and share your own improvements.

Every contribution helps make the project better.

---

## Roadmap

- [x] Hardware design
- [x] Firmware
- [x] Complete documentation
- [x] Windows support
- [x] macOS support
- [x] Linux support
- [ ] Configuration utility
- [ ] Additional button layouts
- [ ] Community requested improvements

---

## License

The firmware contained in this repository is released under the **GNU General Public License v3.0 (GPL-3.0)**.

3D printable files may be distributed under a separate license. Please refer to the STL repository for licensing information.

---

## Contributing

Bug reports, improvements, and pull requests are always welcome.

If you build your own controller or improve the project, feel free to share it with the community!

---

## Support

If you have any questions, open an Issue or start a Discussion.

Feedback is always appreciated and helps improve the project.

---

## Disclaimer

This is an independent open-source DIY project and is not affiliated with or endorsed by 3Dconnexion.

---

## Credits

Designed by **HackMan3D**

Additional features, testing, and feedback by [Kitek](https://www.crealitycloud.com/user/7734397320).

This project is released free of charge for the maker community.

### Special Thanks

Special thanks to **NavCore** for developing and maintaining the custom Arduino board package that enables native 3Dconnexion compatibility on the Arduino Pro Micro.

Board package:

https://github.com/NavCoree/3D-controller-Board-package

Without this work, native 3Dconnexion support on the Arduino Pro Micro would not have been possible.

---

If you build one, I'd genuinely love to see it.

Don't hesitate to share your photos, remixes or improvements with the community!

⭐ If you like this project, consider starring this repository.
