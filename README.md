# PiSoftware
The SW in this repository is the Raspberry Pi portion of the SW for RoboOne and its charger, described in full here:

http://www.meades.org/robots/robots.html

The Raspberry Pi is expected to be running ARCH Linux, for which the installation instructions and specific installation files are kept in the pi subdirectory of this folder.

The SW consists of a number of separate executables communicating via messages sent over a sockets interface.  The executables are built under GCC and makefiles plus Eclipse project files are provided for a Windows environment where a GCC-to-Pi cross-compiler is required (one has been built under crosstool-ng).

The general structure of the SW is as follows:

RoboOne - main application that starts all the other executables and displays a monitor/dashboard on a TTY.
RoboOneHardware - message server that handles interfacing with all of the hardware on RoboOne, using OneWire and OneWireLibs amongst other things, and handling communication with the Orangutan board that drives RoboOne's motors and handles additional sensors.
RoboOneBatteryManager - message server that monitors the charging of the batteries on RoboOne.
RoboOneStateMachine - message server that runs the state machine defined in RoboOne.uml (stored in the RoboOne directory).
RoboOneTaskHandler - message server that holds the task queue of things for RoboOne to do.

The server executables each link generic message send/receive libraries (MessagingClient/MessagingServer) and write a .log file of their activities.  The protocol is that every message that is sent will receive a message in return (which may be empty and may be ignored).  Taking RoboOneStateMachine as an example, a message log would show something like:

BM client sent w,
SM Server received x,
HW client sent y,
SM Server received z.

This should be interpreted as:

RoboOneStateMachine's Battery Manager client function sent message w to the Battery Manager server,
RoboOneStateMachine's received message x back from the Batter Manager server,
RoboOneStateMachine's Hardware client function sent message y to the Hardware server,
RoboOneStateMachine's received message z back from the Hardware server.

Common utilities and types are kept in the shared directory.

Charger and PiIo are solely used for the RoboOne charger side.

OneWireServer, OneWireTestClient, HelloClient, HelloServer and HelloWorld are no longer in active use.

Rob Meades
