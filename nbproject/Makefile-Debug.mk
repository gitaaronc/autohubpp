#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/AutoResetEvent.o \
	${OBJECTDIR}/Autohub.o \
	${OBJECTDIR}/DynamicLibrary.o \
	${OBJECTDIR}/InsteonController.o \
	${OBJECTDIR}/InsteonDevice.o \
	${OBJECTDIR}/InsteonNetwork.o \
	${OBJECTDIR}/InsteonProtocol.o \
	${OBJECTDIR}/Logger.o \
	${OBJECTDIR}/MessageProcessor.o \
	${OBJECTDIR}/PropertyKeyNames.o \
	${OBJECTDIR}/SerialPort.o \
	${OBJECTDIR}/SocketPort.o \
	${OBJECTDIR}/autoapi.o \
	${OBJECTDIR}/config.o \
	${OBJECTDIR}/include/insteon/detail/InsteonDeviceImpl.o \
	${OBJECTDIR}/include/system/Timer.o \
	${OBJECTDIR}/include/utils/utils.o \
	${OBJECTDIR}/jsoncpp.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/local/lib -lboost_system -lboost_filesystem -lboost_log -lboost_thread -ldl -lrestbed -lcrypto -lssl -lyaml-cpp

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/autohubpp

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/autohubpp: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/autohubpp ${OBJECTFILES} ${LDLIBSOPTIONS} -pthread -rdynamic

${OBJECTDIR}/AutoResetEvent.o: nbproject/Makefile-${CND_CONF}.mk AutoResetEvent.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AutoResetEvent.o AutoResetEvent.cpp

${OBJECTDIR}/Autohub.o: nbproject/Makefile-${CND_CONF}.mk Autohub.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Autohub.o Autohub.cpp

${OBJECTDIR}/DynamicLibrary.o: nbproject/Makefile-${CND_CONF}.mk DynamicLibrary.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DynamicLibrary.o DynamicLibrary.cpp

${OBJECTDIR}/InsteonController.o: nbproject/Makefile-${CND_CONF}.mk InsteonController.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/InsteonController.o InsteonController.cpp

${OBJECTDIR}/InsteonDevice.o: nbproject/Makefile-${CND_CONF}.mk InsteonDevice.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/InsteonDevice.o InsteonDevice.cpp

${OBJECTDIR}/InsteonNetwork.o: nbproject/Makefile-${CND_CONF}.mk InsteonNetwork.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/InsteonNetwork.o InsteonNetwork.cpp

${OBJECTDIR}/InsteonProtocol.o: nbproject/Makefile-${CND_CONF}.mk InsteonProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/InsteonProtocol.o InsteonProtocol.cpp

${OBJECTDIR}/Logger.o: nbproject/Makefile-${CND_CONF}.mk Logger.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Logger.o Logger.cpp

${OBJECTDIR}/MessageProcessor.o: nbproject/Makefile-${CND_CONF}.mk MessageProcessor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MessageProcessor.o MessageProcessor.cpp

${OBJECTDIR}/PropertyKeyNames.o: nbproject/Makefile-${CND_CONF}.mk PropertyKeyNames.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PropertyKeyNames.o PropertyKeyNames.cpp

${OBJECTDIR}/SerialPort.o: nbproject/Makefile-${CND_CONF}.mk SerialPort.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SerialPort.o SerialPort.cpp

${OBJECTDIR}/SocketPort.o: nbproject/Makefile-${CND_CONF}.mk SocketPort.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SocketPort.o SocketPort.cpp

${OBJECTDIR}/autoapi.o: nbproject/Makefile-${CND_CONF}.mk autoapi.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/autoapi.o autoapi.cpp

${OBJECTDIR}/config.o: nbproject/Makefile-${CND_CONF}.mk config.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/config.o config.cpp

${OBJECTDIR}/include/insteon/detail/InsteonDeviceImpl.o: nbproject/Makefile-${CND_CONF}.mk include/insteon/detail/InsteonDeviceImpl.cpp 
	${MKDIR} -p ${OBJECTDIR}/include/insteon/detail
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/include/insteon/detail/InsteonDeviceImpl.o include/insteon/detail/InsteonDeviceImpl.cpp

${OBJECTDIR}/include/system/Timer.o: nbproject/Makefile-${CND_CONF}.mk include/system/Timer.cpp 
	${MKDIR} -p ${OBJECTDIR}/include/system
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/include/system/Timer.o include/system/Timer.cpp

${OBJECTDIR}/include/utils/utils.o: nbproject/Makefile-${CND_CONF}.mk include/utils/utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/include/utils
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/include/utils/utils.o include/utils/utils.cpp

${OBJECTDIR}/jsoncpp.o: nbproject/Makefile-${CND_CONF}.mk jsoncpp.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/jsoncpp.o jsoncpp.cpp

${OBJECTDIR}/main.o: nbproject/Makefile-${CND_CONF}.mk main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBOOST_FILESYSTEM_NO_DEPRECATED -DBOOST_LOG_DYN_LINK -I/usr/include/yaml-cpp -I/usr/include/websocketpp -I/usr/include/restbed -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/autohubpp

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
