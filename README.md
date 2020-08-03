# Battmon
### A lightweight battery monitor in C++.

by Manuel Olguin (a.k.a. Arachnid92)

Battmon is a battery monitor for people like me who use minimalistic tiling WMs
or who simply don't want to clutter down their system tray with unnecessary icons.
The program runs in the background, polling the battery state at /sys/class/power_supply/ 
and using desktop notifications to notify the user at user-defined intervals 
(by default, it notifies every 10%, and at 15% for low battery and 5% for critical battery), 
and DOES NOT HAVE A SYSTEM TRAY ICON. I might add it in the future, but it will definitely be an optional feature.

The polling interval can be adjusted using the --interval (-i) command line option. Default is one second, but it can definitely be set
to a greater number to minimize system perfomance impact (although it should already be extremely low, since it is written in C++ and is
very simple and lightweight).

As of the release version (0.5beta1), there are two default icons packs (for the notifications) to choose from: light and dark. 
By default the program uses the light ones.

This is my first C++ project, so the code is probably horrible and too C-like. Pull-requests and bug reports are welcome! :)  

The icons are by Icon Works
http://www.flaticon.com/packs/icon-works/3

This software is released under a GNU GPL v2 License.

## Installation:

### Dependencies:
- cmake
- GDK Pixbuf
- LibNotify
- GLIB
- GTK2
- Boost

#### Manual Install:
```
$git clone git@github.com:arachnid92/battmon.git
$cd battmon
$cmake ./
$sudo make install
``` 
### Arch Linux:
An AUR package exists at https://aur4.archlinux.org/packages/battmon/:
```
$git clone https://aur4.archlinux.org/battmon.git/
$cd battmon
$makepkg
$sudo pacman -U <battmon_package>.tar.gz
```
Or if you're lazy like me and like to use Yaourt:
```
$yaourt -S battmon
```

## Basic Usage:

For example, to run the program with a a polling rate of 2 seconds, notifying every 3%, low battery level set at 10%,
critical at 3% and using dark icons:

```
battmon --interval 2 --delta 3 --low 10 --critical 3 --dark
```

### TODO

[x] Add support for BAT1

[ ] Add support for custom battery (if not in the proper path)

[ ] Fix it for dual battery system 

