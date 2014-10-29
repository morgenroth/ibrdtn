#!/bin/bash -e
#

if [ $# -lt 2 ]; then
    echo "Usage: $0 <source-file> <target-name> [version]"
    echo "Example: $0 ic_launcher.svg ic_launcher v21"
    exit 0
fi

if [ $# -gt 2 ]; then
    PATHSUFFIX="-${3}"
fi

NAME=${2}

function createicon() {
  mkdir -p res/drawable-${4}
  inkscape --export-png res/drawable-${4}/${2}.png -w ${3} ${1}
}

# web: 512x512
inkscape --export-png ${NAME}-web${PATHSUFFIX}.png -w 512 ${1}

# xxxhdpi: 192x192
createicon ${1} ${NAME} 192 xxxhdpi${PATHSUFFIX}

# xxhdpi: 144x144
createicon ${1} ${NAME} 144 xxhdpi${PATHSUFFIX}

# xhdpi: 96x96
createicon ${1} ${NAME} 96 xhdpi${PATHSUFFIX}

# hdpi: 72x72
createicon ${1} ${NAME} 72 hdpi${PATHSUFFIX}

# mdpi: 48x48
createicon ${1} ${NAME} 48 mdpi${PATHSUFFIX}

