# autohubpp
C++ Home Automation Hub - Insteon PLM Support

<b>Intention of Project</b><br/>

 To create a fully customizable home automation hub.<br/>
 This server software is written in C++. It was designed to provide TCP/IP support to Insteon PLM.<br/>
 <b>PLM's supported:</b> Serial / USB<br/>
 Additional support for Insteon Hub is available but has not been pushed to my github repo. <br/>
 <b>NO</b> Insteon accounts are required.<br/>
 
 I have decided to open source this project for various reasons. <br/>
  - To increase support for Insteon hardware in the open source community.
  - Software development is vary time consuming. I simply don't have the spare time to support a project of this size by myself.
  - To attract C++ developers to the project increasing support for the project itself.
  - To learn from other C++ developers and help other C++ developers learn.

This is the backend server software only. I currently run it on a Beaglebone Black. <br/>
The front end used with this project is Home Assistant. https://github.com/home-assistant/home-assistant <br/>
Any frontend GUI supporting restapi or websockets will work with this server.<br/>

A very simple python library was written for homeassistant to communicate with this server software. <br/>
Autohub components were also written for use with with Home Assistant.
 - Python library for autohub: https://github.com/gitaaronc/pyautohub
 - If there are any python developers willing to take part, please contact me.
 - HomeAssistant integration can be found in my homeassistant repo, branch autohub.

This is the initial push to the repo, additional documentation will follow.

Server supports restapi and websockets. Rest and Websockets are used for controlling devices and receiving updates.
TODO: 
 - Documentation
 - configuration class to hold settings such as serial port. Currently settings are stored in "include/config.hpp"

You must have all the the required dependencies to compile autohubpp.<br/>
First ensure you have at least version 1.55 of the boost libraries installed.<br/>
<b>apt-get install libboost-all-dev</b><br/>

It's recommended to create a dev directory and clone the following repositories into it.
```
-dev
--restbed
--websocketpp
--yaml-cpp
--autohubpp
```
<b>RESTBED</b><br/>
git clone --recursive https://github.com/gitaaronc/restbed.git<br/>
Follow the instructions for compiling and installing restbed: https://github.com/gitaaronc/restbed<br/>

<b>WEBSOCKETPP</b><br/>
git clone https://github.com/gitaaronc/websocketpp.git<br/>

<b>YAML-CPP</b><br/>
https://github.com/gitaaronc/yaml-cpp.git
Follow the instructions for compiling provided in theyaml-cpp repo.<br/>

<b>AUTOHUBPP</b><br/>
Clone this repository.

<b>Compiling</b><br/>
 I compile and run everything in linux. All of this should compile and run under windows but it hasn't been tested.<br />
 I do use Netbeans as an IDE on windows, but all compiling is done remotely on linux.<br />

 The compiler must be able to find the header files included with the above dependencies.<br />
 I create a softlink inside of /usr/include<br />
 ln -s /{GIT_REPO_ROOT}/websocketpp websocketpp<br />
 ln -s /{GIT_REPO_ROOT}/restbed restbed<br />
 ln -s /{GIT_REPO_ROOT}/yaml-cpp yaml-cpp<br />
 Once you have the dependencies in place and the symbolic links created you can run make.<br />
 The libraries created by the above dependencies will require placement into your /usr/lib folder.</br>
 Rather than moving or copying the required libraries, I create symbolic links using the above method.<br/>
 
 If there are any masters of CMake out there, an automated process is needed.<br />
 If you are interested in helping with the development of this project please contact me.<br />

<b>Running/Configuration</b><br/>
A yaml configuration file must be specified on the command line.
ex: autohubpp /etc/configuration.yaml</br>
example yaml configuration file<br/>
```
# loggin mode
logging_mode: VERBOSE
# INSTEON PLM Information
PLM:
  serial_port: /dev/ttyUSB0
  baud_rate: 19200
# WEBSOCKET listening information
WEBSOCKETPP:
  listening_port: 9100
```
More C++ and Python developers required!!

Thanks.
