RIOFILES_DIR="/riofiles"
DAEMON_BINARY_NAME=rioboard_d
DAEMON_BINARY_INSTALL_PATH=/usr/sbin
DAEMON_STARTUP_SCRIPT=rioboard_d.service
DAEMON_STARTUP_SCRIPT_INSTALL_PATH=/etc/systemd/system
INSTALL_ARCHIVE_NAME=RioBoardInstallFiles.zip
INSTALL_URL=http://www.roboteq.com/riofiles

WIRING_PI_SHARED_LIB=libwiringPi.so.1.0 
WIRING_PI_LINK_NAME=libwiringPi.so.1
WIRING_PI_INSTALL_PATH=/usr/lib

RIOBOARD_FIRMWARE_ARCHIVE=raspioF3-update.zip

echo ----- Rio Board installation script for Arch Linux

echo ----- Creating directory for Rio Board files ... 
mkdir -p $RIOFILES_DIR
if [ $? -ne 0 ]
then
   echo ERROR: "Can't create directory for Rio Board files. Please check your file system access privileges."
   exit
fi

cd $RIOFILES_DIR

echo ----- Downloading installation files for Rio Board ...
wget -nv $INSTALL_URL/$INSTALL_ARCHIVE_NAME
if [ $? -ne 0 ]
then
   echo ERROR: "Can't download installation file. Please check your internet connection or file system access privileges."
   exit
fi

echo ----- Changing file attributes for installation archive ...
chmod 755 $RIOFILES_DIR/$INSTALL_ARCHIVE_NAME
if [ $? -ne 0 ]
then
   echo ERROR: "Can't change archive file attributes. Please check your file system access privileges."
   exit
fi

echo ----- Unpacking RioBoard installation files ...
unzip -o $RIOFILES_DIR/$INSTALL_ARCHIVE_NAME
if [ $? -ne 0 ]
then
   echo ERROR: "Can't unpack installation files. Please check your file system access privileges."
   exit
fi 

echo ----- Setting file attributes for Rio Board files ...
chmod -R 755 $RIOFILES_DIR/*
if [ $? -ne 0 ]
then
   echo ERROR: "Can't Rio Board test/sample file attributes. Please check your file system access privileges."
   exit
fi

echo ----- Checking for previous version of Rio Board daemon ...
if [ ! -f $DAEMON_INSTALL_PATH/$DAEMON_BINARY_NAME ]
then
   echo ----- Removing the previous version of Rio Board daemon ...
   rm -f $DAEMON_INSTALL_PATH/$DAEMON_BINARY_NAME
   systemctl stop $DAEMON_STARTUP_SCRIPT
   systemctl disable $DAEMON_STARTUP_SCRIPT
fi

echo ----- Installing RioBoard daemon ...
chown `id -u`.`id -g` /dev/spidev0.*
cp $RIOFILES_DIR/$DAEMON_BINARY_NAME $DAEMON_BINARY_INSTALL_PATH
chmod 755 $DAEMON_BINARY_INSTALL_PATH/$DAEMON_BINARY_NAME
cp $RIOFILES_DIR/$DAEMON_STARTUP_SCRIPT $DAEMON_STARTUP_SCRIPT_INSTALL_PATH
chmod 755 $DAEMON_STARTUP_SCRIPT_INSTALL_PATH/$DAEMON_STARTUP_SCRIPT
systemctl enable $DAEMON_STARTUP_SCRIPT
systemctl start $DAEMON_STARTUP_SCRIPT

echo ----- Installing wiringPi library
rm -f /usr/local/lib/$WIRING_PI_SHARED_LIB
rm -f /usr/local/lib/$WIRING_PI_LINK_NAME
cp ./$WIRING_PI_SHARED_LIB /usr/local/lib
if [ $? -ne 0 ]
then
   echo "ERROR: Can't copy WirigPi library file. Please check yout file system access privileges"
   exit
fi

chmod 755 /usr/local/lib/$WIRING_PI_SHARED_LIB
if [ $? -ne 0 ]
then
   echo "ERROR: Can't set file attributes to WiringPi library file. Please check yout file system access privileges"
   exit
fi

ln /usr/local/lib/$WIRING_PI_SHARED_LIB /usr/local/lib/$WIRING_PI_LINK_NAME
if [ $? -ne 0 ]
then
   echo "ERROR: Can't create symbolic link to WiringPi library file. Please check yout file system access privileges"
   exit
fi

ldconfig
if [ $? -ne 0 ]
then
   echo "ERROR: Can't update shared library subsystem. Make sure to run this script with 'root' user privileges"
   exit
fi

echo ----- Fetching the recent firmware for RioBoard
wget -nv $INSTALL_URL/$RIOBOARD_FIRMWARE_ARCHIVE
if [ $? -ne 0 ]
then
   echo ERROR: "Can't download Rio Board installaion file. Please check your internet connection or file system access privileges."
   exit
fi

unzip -o $RIOFILES_DIR/$RIOBOARD_FIRMWARE_ARCHIVE
if [ $? -ne 0 ]
then
   echo ERROR: "Can't unpack installation Rio Board firmware update files. Please check your file system access privileges."
   exit
fi

cd ~

echo ----- Removing installation archive ...
rm -Rf $INSTALL_ARCHIVE_NAME
rm -Rf $RIOBOARD_FIRMWARE_ARCHIVE 

echo ----- Checking if minicom is installed ...
if which minicom >/dev/null; then
	echo minicom located
else 
	echo minicom was not found - installing ...
	pacman -S minicom
fi

echo ----- Asigning the alias riocom for minicom with pre-set parameters for RioBoard ...
echo 'alias riocom="minicom -b 115200 -o -D /dev/ttyAMA0"' >> /etc/bash.bashrc
if [ $? -ne 0 ]
then
   echo ERROR: "Can't modify /etc/bash.bashrc. Please check your file system access privileges."
   exit
fi

echo ----- Disabling the boot over serial port ...
cp -f /boot/cmdline.txt /riofiles/cmdline.txt.copy
if [ $? -ne 0 ]
then
   echo ERROR: "Can't copy /boot/cmdline.txt. Please check your file system access privileges."
   exit
fi

sed -e 's/console\=ttyAMA0\,115200 kgdboc=ttyAMA0\,115200//g' /boot/cmdline.txt >> /boot/cmdline.txt

systemctl stop serial-getty@ttyAMA0.service
systemctl disable serial-getty@ttyAMA0.service
systemctl mask serial-getty@ttyAMA0.service

echo Please reboot now.
