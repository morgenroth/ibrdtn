#!/bin/sh -e
#

BUILD_MODE="release"
WORKSPACE=$(pwd)

### BUILD IBR-DTN API FOR ANDROID ###

# update android project
android update project -p ibrdtn-api -n android-ibrdtn-api

# build IBR-DTN API
ant -f ibrdtn-api/build.xml ${BUILD_MODE}

# copy API .jar to all DTN apps
cp ibrdtn-api/bin/classes.jar ibrdtn/libs/android-ibrdtn-api.jar
cp ibrdtn-api/bin/classes.jar Whisper/libs/android-ibrdtn-api.jar
cp ibrdtn-api/bin/classes.jar Talkie/libs/android-ibrdtn-api.jar

### BUILD IBR-DTN DAEMON ###

# build JNI part of the daemon
cd ibrdtn/jni
./build.sh

# switch back to workspace
cd ${WORKSPACE}

# update android project
android update project -p ibrdtn -n ibrdtn

# build IBR-DTN API
ant -f ibrdtn/build.xml ${BUILD_MODE}

### BUILD WHISPER ###

# update android project
android update project -p Whisper -n Whisper

# build IBR-DTN API
ant -f Whisper/build.xml ${BUILD_MODE}

### BUILD TALKIE ###

# update android project
android update project -p Talkie -n Talkie

# build IBR-DTN API
ant -f Talkie/build.xml ${BUILD_MODE}

