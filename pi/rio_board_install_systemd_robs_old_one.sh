#This a "systemd-ised" version of rio_board_install.sh, needed for Arch Linux.
#Original lines are left in, commented out.

echo "BE SURE to run this with root privileges (sudo) from the directory that this script and rioboard_d.service is in"
echo "Press a key to continue..."
read -n 1

RIOFILES_DIR="/riofiles"

DAEMON_BINARY_NAME=rioboard_d
DAEMON_BINARY_INSTALL_PATH=/usr/sbin
#DAEMON_STARTUP_SCRIPT=rioboard_d
DAEMON_STARTUP_SCRIPT=rioboard_d.service
DAEMON_STARTUP_SCRIPT_INSTALL_PATH=/etc/systemd/system
INSTALL_ARCHIVE_NAME=RioBoardInstallFiles.zip
INSTALL_URL=http://www.roboteq.com/riofiles

WIRING_PI_SHARED_LIB=libwiringPi.so.2.25 
WIRING_PI_LINK_NAME=libwiringPi.so.2.25
#systemd doesn't look in /usr/local/lib so need to make this usr/lib
WIRING_PI_INSTALL_PATH=/usr/lib

RIOBOARD_FIRMWARE_ARCHIVE=raspioF3-update.zip

mkdir -p $RIOFILES_DIR
pushd $RIOFILES_DIR

wget -nv $INSTALL_URL/$INSTALL_ARCHIVE_NAME

echo ----- Unpacking RioBoard installation archive ...
unzip -o $RIOFILES_DIR/$INSTALL_ARCHIVE_NAME

echo ----- Trying to uninstall an old version of the daemon ...
rm -f $DAEMON_BINARY_INSTALL_PATH/$DAEMON_BINARY_NAME
#update-rc.d -f rioboard_d remove
systemctl stop $DAEMON_STARTUP_SCRIPT
systemctl disable $DAEMON_STARTUP_SCRIPT

echo ----- Installing wiringPi library
cp $WIRING_PI_SHARED_LIB $WIRING_PI_INSTALL_PATH
chmod 755 $WIRING_PI_INSTALL_PATH/$WIRING_PI_SHARED_LIB
pushd $WIRING_PI_INSTALL_PATH
ln $WIRING_PI_SHARED_LIB $WIRING_PI_LINK_NAME
ldconfig
popd

echo ----- Installing RioBoard daemon ...
#This used to be in the demon script but you can't use the same quoted construct in systemd, hence moving it to here and hoping.
chown `id -u`.`id -g` /dev/spidev0.*
cp $DAEMON_BINARY_NAME $DAEMON_BINARY_INSTALL_PATH
chmod 755 $DAEMON_BINARY_INSTALL_PATH/$DAEMON_BINARY_NAME
#cp ./startup_script/$DAEMON_STARTUP_SCRIPT /etc/init.d
#chmod 755 /etc/init.d/$DAEMON_STARTUP_SCRIPT
#update-rc.d rioboard_d defaults
popd
cp $DAEMON_STARTUP_SCRIPT $DAEMON_STARTUP_SCRIPT_INSTALL_PATH
pushd $RIOFILES_DIR
chmod 755 $DAEMON_STARTUP_SCRIPT_INSTALL_PATH/$DAEMON_STARTUP_SCRIPT
systemctl enable $DAEMON_STARTUP_SCRIPT
systemctl start $DAEMON_STARTUP_SCRIPT

echo ----- Fetching the recent firmware for RioBoard
wget -nv $INSTALL_URL/$RIOBOARD_FIRMWARE_ARCHIVE
unzip -o $RIOFILES_DIR/$RIOBOARD_FIRMWARE_ARCHIVE

echo ----- Removing installation archive ...
rm -Rf $INSTALL_ARCHIVE_NAME
rm -Rf $RIOBOARD_FIRMWARE_ARCHIVE 

popd

echo ----- Checking if minicom is installed ...
if which minicom >/dev/null; then
	echo minicom located
else 
	echo minicom was not found - installing ...
#	apt-get install minicom
	pacman -S minicom
fi

echo ----- Assigning the alias riocom for minicom with pre-set parameters for RioBoard ...
chmod 777 /etc/bash.bashrc
echo 'alias riocom="minicom -b 115200 -o -D /dev/ttyAMA0"' >> /etc/bash.bashrc

echo ----- Disabling the boot over serial port ...
cp /boot/cmdline.txt /riofiles/cmdline.txt.copy
echo "Now edit /boot/cmdline.txt to remove the bit where it says:"
echo "console=ttyAMA0,115200 kgdboc=ttyAMA0,115200"
echo "Then reboot."
#echo "dwc_otg.lpm_enable=0 console=tty1 root=/dev/mmcblk0p2 rootfstype=ext4 elevator=deadline rootwait" > /boot/cmdline.txt
#cp /etc/inittab /riofiles/inittab.copy
#sed -e 's/^T0\:23\:respawn\:\/sbin\/getty \-L ttyAMA0 115200 vt100/#&/g' /riofiles/inittab.copy > /etc/inittab
#Getty service on the serial port that _I_ want for myself, how shall I stop thee?  Let me count the ways:
systemctl stop serial-getty@ttyAMA0.service
systemctl disable serial-getty@ttyAMA0.service
systemctl mask serial-getty@ttyAMA0.service

echo ----- Rebooting the system ...
#reboot -f
echo Please reboot now.