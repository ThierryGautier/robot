# Hikey project
hikey 96boards sources
- The bluetooth directory is a C project used to communicate with FRDM_KV31 board through bluetooth rfcomm connection.
- The joystick directory is a C project used to read gamepad information. The gamepad information drives robot.

# Hikey external software
BOOT-ROOTFS Reference platform 2016-06
-	download Debian For Hikey and apply the procedure to update boot and rootfs 
-	open terminal
-	set config keyboard to azerty  (setxkbmap fr)
-	edit and check /etc/apt/sources.list (jessie)
-	sudo apt-get update
-	do not do: sudo apt-get upgrade 30/06/217 problem update package freeze the board
-	do not do: sudo apt-get dist-upgrade 30/06/217 problem update package freeze the board

PACKAGES List of package required for the project
-	eclipse
//eclipse http://packages.ubuntu.com/search?keywords=eclipse
//IDE (gdb , compilation native OK)
sudo apt-get install eclipse
sudo apt-get install eclipse-cdt
sudo apt-get install eclipse-cdt-autotools
sudo apt-get install eclipse-anyedit

-	vlc (test audio-video)
//outil audio et video pour tester com bluetooth ave BT1300 philips
sudo apt-get install vlc

//list hardwardsup
sudo apt-get install lshw

//gestion disque partition permet de voir les partion de la flash emmc de la hikey board
// permet de faire de disque image de la flash
// https://packages.debian.org/fr/jessie/gnome-disk-utility
sudo apt-get install gnome-disk-utility
-	bluethooth

//bluethooth pour LX music
sudo apt-get install xmms2 xmms2-plugin-*

//bluethooth cmd:List of devices to get the MAC address of my device:bt-device -l
sudo apt-get install bluez-tools

-	open cv (open soucre vision stack)
https://github.com/milq/milq/blob/master/scripts/bash/install-opencv.sh
	# INSTALL OPENCV ON UBUNTU OR DEBIAN #
	######################################
	

	# |         THIS SCRIPT IS TESTED CORRECTLY ON         |
	# |----------------------------------------------------|
	# | OS             | OpenCV       | Test | Last test   |
	# |----------------|--------------|------|-------------|
	# | Ubuntu 16.04.2 | OpenCV 3.2.0 | OK   | 20 May 2017 |
	# | Debian 8.8     | OpenCV 3.2.0 | OK   | 20 May 2017 |
	# | Debian 9.0     | OpenCV 3.2.0 | OK   | 25 Jun 2017 |
	

	# 1. KEEP UBUNTU OR DEBIAN UP TO DATE
	
sudo apt-get -y update
#sudo apt-get -y upgrade
#sudo apt-get -y dist-upgrade
#sudo apt-get -y autoremove

	# 2. INSTALL THE DEPENDENCIES

	# Build tools:
sudo apt-get install -y build-essential cmake

	# GUI (if you want to use GTK instead of Qt, replace 'qt5-default' with 'libgtkglext1-dev' and remove '-DWITH_QT=ON' option in CMake):
sudo apt-get install -y qt5-default libvtk6-dev

	# Media I/O:
sudo apt-get install -y zlib1g-dev libjpeg-dev libwebp-dev libpng-dev libtiff5-dev libjasper-dev libopenexr-dev libgdal-dev

	# Video I/O:
sudo apt-get install -y libdc1394-22-dev libavcodec-dev libavformat-dev libswscale-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev yasm libopencore-amrnb-dev libopencore-amrwb-dev libv4l-dev libxine2-dev

	# Parallelism and linear algebra libraries:
sudo apt-get install -y libtbb-dev libeigen3-dev

	# Python:
sudo apt-get install -y python-dev python-tk python-numpy python3-dev python3-tk python3-numpy

	# Java:
sudo apt-get install -y ant default-jdk

	# Documentation:
sudo apt-get install -y doxygen

  #++ T.GAUTIER
sudo apt-get install FFmpeg
	
	# 3. INSTALL THE LIBRARY (YOU CAN CHANGE '3.2.0' FOR THE LAST STABLE VERSION)
sudo apt-get install -y unzip wget
wget https://github.com/opencv/opencv/archive/3.2.0.zip
unzip 3.2.0.zip
wget -O opencv_contrib.zip https://github.com/Itseez/opencv_contrib/archive/3.2.0.zip
unzip opencv_contrib.zip


#### TODO clarify opencv_contrib-3.2.0

cd opencv-3.2.0
mkdir build
cd build
cmake -Wno-dev -D CMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-3.2.0/modules ../   >cmake_result.txt 

make -j4
sudo make install
sudo ldconfig

	# 4. EXECUTE SOME OPENCV EXAMPLES AND COMPILE A DEMONSTRATION


//bluetooth

http://www.96boards.org/documentation/ConsumerEdition/HiKey/Configuration/WirelessTIModule.md/

//installer package hwpack
//voir https://doc.ubuntu-fr.org/dpkg
//se placer dans le répertoire ou il ya tout les fichiers *.deb
sudo dpkg -i -R ./

//dpkg ne gère pas les dépendances, il faut ensuite, pour compléter l'installation d'une application possédant des dépendances, exécuter dans un terminal la commande:
sudo apt-get -f install

//sound http://www.alsa-project.org/alsa-doc/alsa-lib/examples.html
sudo apt-get install libasound2
sudo apt-get install libasound2-dev
sudo apt-get install libasound2-doc
//sudo apt-get install Alsa-base


-	synthèse vocale
//https://www.gnu.org/software/gnuspeech/
//https://tuxicoman.jesuislibre.net/2015/05/synthese-vocale-sous-linux.html
//http://espeak.sourceforge.net/mbrola.html not free
//http://www.pobot.org/Synthese-vocale-avec-espeak-et.html?lang=fr
//sudo apt-get install espeak mbrola mbrola-fr4
//mbrola et mbrola-fr4 non présent sur ARM debian voir next URL
//http://www.tcts.fms.ac.be/synthesis/mbrola/mbrcopybin.html
// festival (1:2.1~release-8)
//gstreamer0.10-pocketsphinx (0.8-5) Speech recognition tool - gstreamer plugin
//pocketsphinx (0.8-5) Speech recognition tool

//install de espeak
sudo apt-get install espeak
sudo apt-get install espeak-data
sudo apt-get install festival

//install de mbrola
wget http://tcts.fpms.ac.be/synthesis/mbrola/bin/armlinux/mbrola.rar
sudo apt-get install unrar
unrar e mbrola.rar
sudo mv mbrola /usr/bin
sudo chmod 777 /usr/bin/mbrola

//problem executuion de mbrola => test conversion dos vers uinx pas de changement!!!!
sudo apt-get install dos2unix
linaro@linaro-alip:/usr/bin$ sudo dos2unix mbrola


//install de langue française
sudo mkdir /usr/share/mbrola 
sudo mkdir /usr/share/mbrola/voices
wget http://tcts.fpms.ac.be/synthesis/mbrola/dba/fr1/fr1-990204.zip
unzip fr1*.zip
sudo mv fr1/fr1 /usr/share/mbrola/voices/

//test
# echo "salut les amis, c'est Pobot" > exemple.txt
# espeak -v mb/mb-fr1 -f exemple.txt | mbrola /usr/share/mbrola/voices/fr1 - - | aplay -r16000 -fS16


-	vision (stack de management vision)



http://www.pyimagesearch.com/2015/10/26/how-to-install-opencv-3-on-raspbian-jessie

sudo apt-get install build-essential git cmake pkg-config
sudo apt-get install libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt-get install libxvidcore-dev libx264-dev
sudo apt-get install libgtk2.0-dev
sudo apt-get install libatlas-base-dev gfortran
sudo apt-get install python2.7-dev python3-dev


Installing OpenCV 3 on Raspbian Jessie
Shell
1
2
3	$ cd ~
$ wget -O opencv.zip https://github.com/Itseez/opencv/archive/3.0.0.zip
$ unzip opencv.zip







	$ cd ~/opencv-3.0.0/
$ mkdir build
$ cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D INSTALL_C_EXAMPLES=ON -D OPENCV_EXTRA_MODULES_PATH=~/../opencv_contrib-3.0.0/modules ..


