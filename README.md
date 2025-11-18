## Finden
A small Gnome desktop application to find items on your computer that you are looking for.  

![finden](finden_img1.png?raw=true)  


### Preconditions and Dev Depentencies:

#### Ubuntu/Debian:  
`sudo apt-get install libglib2.0-dev libgtk-4-dev libadwaita-1-dev ninja-build meson`  

#### Fedora:  
`sudo dnf install glib2-devel gtk4-devel libadwaita-devel ninja-build meson`  

#### Arch Linux:  
`sudo pacman -S glib2 gtk4 libadwaita ninja meson`  


### Installation:
1. `git clone https://github.com/super-toq/Finden.git`
2. `cd Finden`
3. `meson setup _build`
4. `ninja -C _build`
5. `sudo ninja -C _build install`

### Uninstall:  
Go to the download folder from git clone and run the script:  
`./uninstall.sh`  


**Please note**:
***This code is part of my learning project. Use of the code and execution of the application is at your own risk; 
I accept no liability!***
