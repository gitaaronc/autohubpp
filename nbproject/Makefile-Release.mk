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
CND_PLATFORM=None-Windows
CND_DLIB_EXT=dll
CND_CONF=Release
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
	${OBJECTDIR}/SerialPort.o \
	${OBJECTDIR}/SocketPort.o \
	${OBJECTDIR}/autoapi.o \
	${OBJECTDIR}/config.o \
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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/autohubpp.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/autohubpp.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/autohubpp ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/AutoResetEvent.o: AutoResetEvent.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AutoResetEvent.o AutoResetEvent.cpp

${OBJECTDIR}/Autohub.o: Autohub.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Autohub.o Autohub.cpp

${OBJECTDIR}/DynamicLibrary.o: DynamicLibrary.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DynamicLibrary.o DynamicLibrary.cpp

${OBJECTDIR}/InsteonController.o: InsteonController.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/InsteonController.o InsteonController.cpp

${OBJECTDIR}/InsteonDevice.o: InsteonDevice.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/InsteonDevice.o InsteonDevice.cpp

${OBJECTDIR}/InsteonNetwork.o: InsteonNetwork.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/InsteonNetwork.o InsteonNetwork.cpp

${OBJECTDIR}/InsteonProtocol.o: InsteonProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/InsteonProtocol.o InsteonProtocol.cpp

${OBJECTDIR}/Logger.o: Logger.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Logger.o Logger.cpp

${OBJECTDIR}/MessageProcessor.o: MessageProcessor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MessageProcessor.o MessageProcessor.cpp

${OBJECTDIR}/SerialPort.o: SerialPort.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SerialPort.o SerialPort.cpp

${OBJECTDIR}/SocketPort.o: SocketPort.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SocketPort.o SocketPort.cpp

${OBJECTDIR}/autoapi.o: autoapi.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/autoapi.o autoapi.cpp

${OBJECTDIR}/config.o: config.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/config.o config.cpp

${OBJECTDIR}/include/system/Timer.o: include/system/Timer.cpp 
	${MKDIR} -p ${OBJECTDIR}/include/system
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/include/system/Timer.o include/system/Timer.cpp

${OBJECTDIR}/include/utils/utils.o: include/utils/utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/include/utils
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/include/utils/utils.o include/utils/utils.cpp

${OBJECTDIR}/jsoncpp.o: jsoncpp.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/jsoncpp.o jsoncpp.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/autohubpp.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
