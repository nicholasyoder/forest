# Forest

Forest is a lightweight desktop environment for Linux built with C++ and Qt. It has a modular design and is fully themeable via QSS (Qt style sheets).

It can be used with various window managers and compositors, but xfwm4 is recommended.

![Forest desktop screenshot](docs/images/forest_0.7.9_desktop.jpg)

## Installation

Download the latest `.deb` package from the [releases page](https://github.com/nicholasyoder/forest/releases) and install it:

```sh
sudo apt install ./forest_*.deb
```

## Building from Source

**Dependencies Include:** CMake, Qt6, KF6WindowSystem, Qt6Xdg, X11/Xcb, ALSA (libasound2), libsensors

All dependencies should be available in the Debian 13 (Trixie) official repositories. Compatibility with the library versions on other distros is not guaranteed.

```sh
cmake -B build
cmake --build build
sudo cmake --install build
```
