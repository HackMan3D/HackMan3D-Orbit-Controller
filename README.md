![License](https://img.shields.io/github/license/HackMan3D/HackMan3D-Orbit-Controller)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)
![Arduino](https://img.shields.io/badge/Arduino-Pro%20Micro-00979D?logo=arduino)
![Open Source](https://img.shields.io/badge/Open%20Source-Yes-brightgreen)
# Hackman3D Orbit Controller

<p align="center">
  <img src="images/43.jpg" alt="Hackman3D Orbit Controller" width="900">
</p>

An open-source DIY 6-DOF navigation controller built with an Arduino Pro Micro and Hall Effect joysticks.

This repository contains everything needed to build your own controller, including the firmware, Bill of Materials, wiring diagrams, board files, and documentation.

---

## Features

* 6 Degrees of Freedom (6-DOF)
* Hall Effect joystick technology
* Arduino Pro Micro based firmware
* USB-C connectivity
* Fully 3D printable design
* Optional mechanical shortcut buttons
* Open-source hardware and firmware
* Compatible with Windows, macOS, and Linux

---

## Repository Contents

```
Firmware          Arduino source code
BOM               Complete Bill of Materials
Wiring            Wiring diagrams
Documentation    Installation and setup guides
```

---

## Hardware Requirements

* Arduino Pro Micro (ATmega32U4)
* 4× Hall Effect joystick modules
* Optional mechanical keyboard switches
* Standard metric screws
* USB-C cable
* 3D printed parts

A complete list of components is available in the **BOM** folder.

---

## Software Requirements

* Arduino IDE
* Required board package
* 3Dconnexion Driver (Windows, macOS, Linux)

---

## Operating System Compatibility

| Operating System | Status      |
| ---------------- | ----------- |
| Windows          | ✅ Supported |
| macOS            | ✅ Supported |
| Linux            | ✅ Supported |

---

## Getting Started

1. Print all required parts.
2. Purchase the components listed in the BOM.
3. Assemble the controller following the documentation.
4. Upload the firmware to the Arduino Pro Micro.
5. Install the required driver.
6. Enjoy your new DIY 3D navigation controller!

---

## License

The firmware contained in this repository is released under the **GNU General Public License v3.0 (GPL-3.0)**.

3D printable files may be distributed under a separate license. Please refer to the STL repository for licensing information.

---

## Contributing

Bug reports, improvements, and pull requests are always welcome.

If you build your own controller or improve the project, feel free to share it with the community!

---

## Disclaimer

This is an independent open-source DIY project and is not affiliated with or endorsed by 3Dconnexion.

---

## Credits

Designed by **Hackman3D**

This project is released free of charge for the maker community.

Credits

Special thanks to NavCore for developing and maintaining the custom Arduino board package that enables native 3Dconnexion compatibility.

This project uses NavCore's board package. If you'd like to learn more or support their work, please visit:

https://github.com/NavCoree/3D-controller-Board-package

Without this work, native 3Dconnexion support on the Arduino Pro Micro would not have been possible.

If you like this project, don't forget to ⭐ star the repository!
