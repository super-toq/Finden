## finden
A small Gnome desktop application to find items on your computer that you are looking for.

![finden](finden_img1.png?raw=true) 


### Preconditions and dev. depentencies:

#### Ubuntu/Debian:
`sudo apt-get install libglib2.0-dev libgtk-4-dev libadwaita-1-dev`

#### Fedora:
`sudo dnf install glib2-devel gtk4-devel libadwaita-devel`

#### Arch Linux:
`sudo pacman -S glib2 gtk4 libadwaita`


### Installation:
1. `git clone https://github.com/super-toq/finden.git`
2. `cd finden`
3. `meson setup finden`
4. `ninja -C finden`
5. `sudo ninja -C finden install`

### Uninstall:
1. `cd finden` ### in the folder from git clone
2. `./uninstall.sh`


**Please note**:
***This code is part of a learning project. Use of the code and execution of the application is at your own risk;
I accept no liability!***
