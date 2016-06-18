#!/bin/sh

UTILITIES_PATH=device-download-mode-changer

MYTMPDIR=`mktemp -d`
trap "rm -rf $MYTMPDIR" EXIT

usage () {
    echo "Usage copy-firmware.sh [-v] FirmwareDir"
    echo "  FirmwareDir usually on Windows partition."
    echo "  E.g. /media/alex/Windows/Program Files (x86)/HP lt4120 Snapdragon X5 LTE/Image"
}

DBG () {
    if [ -n "$VERBOSE" ] ; then
        time=`date -Iseconds`
        echo "[${time}] $1"
    fi
}

FATAL () {
    time=`date -Iseconds`
    echo "[${time}] FATAL: $1"
}

flash () {
    PARTITION="$1"
    SOURCE="$2"
    if [ ! -f "$SOURCE" ]; then
        FATAL "Can't find file to flash in $SOURCE"
        exit 1
    fi
    NAME=`basename "$SOURCE"`
    TMP_NAME="$MYTMPDIR/$NAME"
    tail -c +13 <"$SOURCE" >"$TMP_NAME"
    DBG "fastboot flash \"$PARTITION\" \"$TMP_NAME\""
    fastboot flash "$PARTITION" "$TMP_NAME"
}

if [ "$#" -lt 1 ] ; then
    usage
    exit 1
fi

if [ "$1" = '-v' ] ; then
    VERBOSE=1
    shift
fi

if [ "$#" != 1 ] ; then
    usage
    exit 1
fi

IMAGE_DIR="$1"

# Checking if it correct dir
DBG "Checking $IMAGE_DIR directory to contain firmware"
for SUB_DIR in AT CM DT GC OG OP SC TE TF VF VZ; do
    DBG "Checking if $IMAGE_DIR/$SUB_DIR presend"
    if [ ! -d "$IMAGE_DIR/$SUB_DIR" ] ; then
        FATAL "$IMAGE_DIR/$SUB_DIR not found!"
        exit 1
    fi
done

DBG "Switching modem to flash mode"
$UTILITIES_PATH/x5-change-download-mode -d /dev/cdc0 -m 5
if [ $? != 0 ]; then
    FATAL "Failed to change mode to Fastboot"
    #exit 1
fi

RETRIES=0
while [ -z "$DEVS" -a $RETRIES -le 30 ]; do
    sleep 1
    DBG "Checking if fastboot device available $RETRIES"
    DEVS=`fastboot devices -i 0x03F0`
    RETRIES=`expr $RETRIES + 1`
done

if [ $RETRIES -le 10 ]; then
    FATAL "Device not found by fastboot. Something went wrong?"
    #exit 1
fi

flash mibib "$IMAGE_DIR"/GC/*/partition.mbn
flash sbl "$IMAGE_DIR"/GC/*/sbl1.mbn
flash sdi "$IMAGE_DIR"/GC/*/sdi.mbn
flash tz "$IMAGE_DIR"/GC/*/tz.mbn
flash mba "$IMAGE_DIR"/GC/*/mba.mbn
flash rpm "$IMAGE_DIR"/GC/*/rpm.mbn
flash qdsp "$IMAGE_DIR"/GC/*/qdsp6sw.mbn
flash aboot "$IMAGE_DIR"/GC/*/appsboot.mbn
flash boot "$IMAGE_DIR"/GC/*/mdm9625-boot.img
flash system "$IMAGE_DIR"/GC/*/mdm9625-sysfs.yaffs2
flash userdata "$IMAGE_DIR"/GC/*/mdm9625-usrfs.yaffs2
fastboot erase efs2
fastboot reboot

