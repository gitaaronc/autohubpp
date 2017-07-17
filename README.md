# autohubpp
C++ Home Automation Hub - Insteon PLM Support

<b>Intention of Project</b><br/>

 To create a fully customizable home automation hub.<br/>
 This server software is written in C++. It was designed to provide TCP/IP support to Insteon PLM.<br/>
 <b>PLM's supported:</b> Serial / USB and HUB<br/>
 If using a HUB it must support RAW Insteon Commands via TCP/IP. My hub is version 4.8A<br/>
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

This server supports restapi and websockets. Rest and Websockets are used for controlling devices and receiving updates.
TODO: 
 - Documentation
 
You must have all the the required dependencies to compile autohubpp.<br/>
First ensure you have at least version 1.55 of the boost libraries installed.<br/>
<b>apt-get install libboost-all-dev</b><br/>

You'll also need yaml-cpp<br/>
<b>apt-get install libyaml-cpp-dev</b><br/>

It's recommended to create a dev directory and clone the following repositories into it.
```
-dev
--restbed
--websocketpp
--autohubpp
```
<b>RESTBED</b><br/>
git clone --recursive https://github.com/gitaaronc/restbed.git<br/>
Follow the instructions for compiling and installing restbed: https://github.com/gitaaronc/restbed<br/>

<b>WEBSOCKETPP</b><br/>
git clone https://github.com/gitaaronc/websocketpp.git<br/>

<b>AUTOHUBPP</b><br/>
Clone this repository.

<b>Compiling</b><br/>
 I compile and run everything in linux on a Beaglebone. <br/>
 All of this should compile and run under windows but it hasn't been tested.<br />
 I do use Netbeans as an IDE on windows, but all compiling is done remotely on linux.<br />

 The compiler must be able to find the header files included with the above dependencies.<br />
 I create a softlink inside of /usr/include<br />
 ln -s /{GIT_REPO_ROOT}/websocketpp websocketpp<br />
 ln -s /{GIT_REPO_ROOT}/restbed/source restbed<br />
 You will also need to create a link to the librestbed.so if the file does not exist in your /usr/lib directory.</br>
 
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
worker_threads: 20
RESTBED:
  listening_port: 8000
INSTEON:
  command_delay: 1500
  DEVICES: # You don't need to populate the device section, it will autopopulate on discovery. You can modify/customize it.
    0x0026deeb:
      properties_:
        button_on_ramp_rate: 31
        button_on_level: 255
        device_category: 1
        device_disabled: 0
        device_engine_version: 2
        device_firmware_version: 65
        device_subcategory: 14
        enable_blink_on_traffic: 0
        enable_led: 0
        enable_load_sense: 0
        enable_programming_lock: 0
        enable_resume_dim: 0
        light_status: 0
        link_database_delta: 2
        signal_to_noise_threshold: 32
        x10_house_code: 32
        x10_unit_code: 0
      device_name_: Desk Lamp
    0x0029e6cd:
      properties_:
        device_engine_version: 2
        device_category: 1
        device_subcategory: 32
        device_firmware_version: 65
        device_disabled: 0
        enable_blink_on_traffic: 0
        enable_led: 0
        enable_load_sense: 0
        enable_programming_lock: 0
        enable_resume_dim: 0
        light_status: 0
        link_database_delta: 24
      device_name_: Ceiling Light
    0x002035f6:
      device_name_: 0x002035f6
      device_disabled: 1
    0x0027ce35:
      device_name_: 0x0027ce35
      device_disabled: 1
  PLM:
    enable_monitor_mode: false
    serial_port: /dev/ttyUSB0
    type: hub #can be hub or serial, only the older hub is support at this time.
    sync_device_status: true
    hub_ip: 192.168.4.147
    baud_rate: 19200
    load_aldb: false
    hub_port: 9761
WEBSOCKET:
  listening_port: 9000
logging_mode: VERBOSE

```

Websocket requests are expected by the server in a json format. Responses are delivered in a json format.<br/>
A sample request:<br/>
```
{
   "event" : "getDeviceList"
}
```
The sampled response:<br/>
```
{
   "devices" : [
      {
         "device_address_" : 2110966,
         "device_name_" : "0x002035f6",
         "properties_" : {
            "light_status" : 0
            "device_disabled" : 1
         }
      },
      {
         "device_address_" : 2547435,
         "device_name_" : "Desk Lamp",
         "properties_" : {
            "button_on_level" : 255,
            "button_on_ramp_rate" : 31,
            "device_category" : 1,
            "device_disabled" : 0,
            "device_engine_version" : 2,
            "device_firmware_version" : 65,
            "device_subcategory" : 14,
            "enable_blink_on_traffic" : 0,
            "enable_led" : 0,
            "enable_load_sense" : 0,
            "enable_programming_lock" : 0,
            "enable_resume_dim" : 0,
            "light_status" : 0,
            "link_database_delta" : 2,
            "signal_to_noise_threshold" : 32,
            "x10_house_code" : 32,
            "x10_unit_code" : 0
         }
      },
      {
         "device_address_" : 2548930,
         "device_name_" : "Bedside Lamp",
         "properties_" : {
            "device_category" : 1,
            "device_disabled" : 0,
            "device_engine_version" : 2,
            "device_firmware_version" : 65,
            "device_subcategory" : 14,
            "enable_blink_on_traffic" : 0,
            "enable_led" : 0,
            "enable_load_sense" : 0,
            "enable_programming_lock" : 0,
            "enable_resume_dim" : 0,
            "light_status" : 255,
            "link_database_delta" : 3
         }
      }
   ],
   "event" : "deviceList"
}
```
Sample request:<br/>
```
{
   "command" : "on",
   "command_two" : 255,
   "device_id" : 2547435,
   "event" : "device"
}
```
Sampled Response:<br/>
```
 {
   "device_address_" : 2547435,
   "device_name_" : "Desk Lamp",
   "event" : "deviceUpdate",
   "properties_" : {
      "button_on_level" : 255,
      "button_on_ramp_rate" : 31,
      "device_category" : 1,
      "device_disabled" : 0,
      "device_engine_version" : 2,
      "device_firmware_version" : 65,
      "device_subcategory" : 14,
      "enable_blink_on_traffic" : 0,
      "enable_led" : 0,
      "enable_load_sense" : 0,
      "enable_programming_lock" : 0,
      "enable_resume_dim" : 0,
      "light_status" : 255,
      "link_database_delta" : 2,
      "signal_to_noise_threshold" : 32,
      "x10_house_code" : 32,
      "x10_unit_code" : 0
   }
}

```
There is also a rest API with an events service to deliver realtime updates to your application. No polling of status required.<br/>
Documenation is still required.<br/>

More C++ and Python developers required!!<br/>

Thanks.
