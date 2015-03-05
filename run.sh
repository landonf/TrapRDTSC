#!/bin/sh

TARGET=$1
NAME="TrapRDTSC"
BUNDLE="org.landonf.$NAME"

if [ -z "${TARGET}" ]; then
    echo "Specify a remote target or 'localhost'"
    exit 1
fi

if [ "${TARGET}" = "localhost" ]; then
    LOCAL="1"
    RSYNC_TARGET="./"
else
    RSYNC_TARGET="${TARGET}:"
fi

function remote () {
    if [ ! -z "${LOCAL}" ]; then
        $*
    else
        ssh "${TARGET}" $*
    fi 
}

function unload_kext () {
    # Check if loaded
    remote sudo kextstat -b "${BUNDLE}" | grep "${BUNDLE}" >/dev/null
    if [ $? != 0 ]; then
        return;
    fi

    remote sudo kextunload "${NAME}.kext"
    if [ $? != 0 ]; then
        echo "Failed to unload, trying again"
        unload_kext
    fi
}

xcodebuild -configuration Debug -target ${NAME}
if [ "$?" != 0 ]; then
    exit 1
fi

# If the kext is loaded, try to unload it
unload_kext

remote sudo rm -rf "${NAME}.kext"
rsync -avz "build/Debug/${NAME}.kext" ${RSYNC_TARGET} &&
    remote sudo chown -R root:wheel "${NAME}.kext" &&
    remote sudo kextload "${NAME}.kext" &&
    echo "kext loaded"
