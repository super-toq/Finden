## Finden
A small Gnome desktop application to find items on your computer that you are looking for.  

![finden](finden_img1.png?raw=true)  
  
![finden](finden_img3.png?raw=true)  

  
**Finden** \[ˈfɪn.dən] is a very simple frontend for the Linux command “find”. It was one of my first programming attempts, incorporating the functionality into an Libadwaita window and also, for the first time, creating a Meson configuration for installation. My App Finden is far from finished and continues to present new challenges for me to expand it. This isn’t about developing a professional app; it’s simply about enjoying the process of building applications.  

### Preconditions and Dev Depentencies:

#### Ubuntu/Debian:  
```
sudo apt-get install libglib2.0-dev libgtk-4-dev libadwaita-1-dev ninja-build meson 
```  

#### Fedora:  
```
sudo dnf install glib2-devel gtk4-devel libadwaita-devel ninja-build meson 
```  

#### Arch Linux:  
```
sudo pacman -S glib2 gtk4 libadwaita ninja meson 
```  
  

### Installation:
```
git clone https://github.com/super-toq/Finden.git 
``` 
```
cd Finden 
``` 
```
meson setup _build 
``` 
```
ninja -C _build 
``` 
```
sudo ninja -C _build install 
```  

### Uninstall:  
Go to the download folder from git clone and run the script:  
```
./uninstall.sh 
```
  
  
**Please note**:  
***This code is part of my learning project. Use of the code and execution of the application is at your own risk; I accept no liability!***
  
