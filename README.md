# autohubpp
C++ Home Automation Hub - Insteon PLM Support

This is the initial push to github. 
Documentation will be made available.

Intention of Project

 To create a fully customizable home automation hub.
 This server software is written in C++. It was designed to provide TCP/IP support to Insteon PLM.
 PLM's supported: Serial / USB
 Additional support for Insteon Hub is available but has not been pushed to my github repo. 
 <b>NO</b> Insteon accounts are required.
 
 Requirements
 - Boost
 - websocketpp
 - jsoncpp
 - restbed

 Links/Sub repos to required libraries will be added to this repo.
 
 I have decided to open source this project for various reasons. 
  - To increase support for Insteon hardware in the open source community.
  - Software development is vary time consuming. I simply do not have the spare time to support a project of this size by myself.
  - To attract C++ developers to the project increasing support for the project itself.
  - To learn from other C++ developers and help other C++ developers learn.

This is the backend server software only. I currently run it on a Beaglebone Black. The front end used with this project is 
Home Assistant. https://github.com/home-assistant/home-assistant 
Any frontend GUI supporting restapi or websockets will work with this server.

A very simple python library was written for homeassistant to communicate with this server software. Autohub components were also written for use with with Home Assistant.
 - Python library for autohub: https://github.com/gitaaronc/pyautohub
Autohubpp has only recently been modified to work with Home Assistant. The python libraries are young and my python skills are limited. If there are any python developers willing to take part, please contact me.

This is the initial push to the repo, additional documentation will follow.

Server supports restapi and websockets. Rest and Websockets are used for controlling devices and receiving updates.
TODO: 
 - Documentation
 - configuration class to hold settings such as serial port, binding ip address, device retries, timeouts, etc. As there is no configuration/settings object in the project you can set your serial port on line 79 of MessageProcessor.cpp

Sample json output for a device:
```json
{
   "device_address_" : 2608693,
   "device_name_" : "0x0027ce35",
   "event" : "deviceUpdate",
   "properties_" : {
      "button_on_level" : 255,
      "button_on_ramp_rate" : 31,
      "device_category" : 2,
      "device_engine_version" : 2,
      "device_firmware_version" : 70,
      "device_subcategory" : 55,
      "enable_blink_on_traffic" : 0,
      "enable_led" : 0,
      "enable_load_sense" : 0,
      "enable_programming_lock" : 1,
      "enable_resume_dim" : 0,
      "light_status" : 0,
      "link_database_delta" : 1,
      "signal_to_noise_threshold" : 127,
      "x10_house_code" : 32,
      "x10_unit_code" : 32
   }
}
```
If you are interested in helping with the development of this project please contact me.

More C++ and Python developers required!!

Thanks.
