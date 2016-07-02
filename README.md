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

A very simple python library was written to communicate with this server software. Autohub components were also written for us with
with Home Assistant.
 - Python library for autohub: https://github.com/gitaaronc/pyautohub

This is the initial push to the repo, additional documentation will follow.

If you are interested in helping with the development of this project please contact me.

Thanks.
