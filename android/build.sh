#!/bin/sh -e
#

BUILD_MODE="release"
WORKSPACE=$(pwd)

### BUILD IBR-DTN API FOR ANDROID ###

# update android project
android update project -p ibrdtn-api -n android-ibrdtn-api

# build IBR-DTN API
ant -f ibrdtn-api/build.xml clean ${BUILD_MODE}

### BUILD IBR-DTN DAEMON ###

# build JNI part of the daemon
cd ibrdtn/jni
./build.sh

# switch back to workspace
cd ${WORKSPACE}

# update android project
android update project -p ibrdtn -n ibrdtn

# build IBR-DTN
ant -f ibrdtn/build.xml clean ${BUILD_MODE}

### BUILD WHISPER ###

# update android project
android update project -p Whisper -n Whisper

# build project
ant -f Whisper/build.xml clean ${BUILD_MODE}

### BUILD TALKIE ###

# update android project
android update project -p Talkie -n Talkie

# build project
ant -f Talkie/build.xml clean ${BUILD_MODE}

### BUILD DTN example app ###

# update android project
android update project -p DTNExampleApp -n DTNExampleApp

# build project
ant -f DTNExampleApp/build.xml clean ${BUILD_MODE}

### BUILD Communicator ###

# update android project
android update project -p Communicator -n Communicator

# build project
ant -f Communicator/build.xml clean ${BUILD_MODE}
