#!/bin/sh

UTILITIES_PATH=device-download-mode-changer

MYTMPDIR=`mktemp -d`
trap "rm -rf $MYTMPDIR" EXIT

DEVICE="/dev/cdc-wdm0"
DEVICE_VENDOR="0x03f0"
FIRMWARE_VARIANT="GC"
ACTIVE_CONFIG_ID="software,CEE6D2D6E1508D2A9B0AE45811A0DAFC7D14B27E"

usage () {
    echo "Usage copy-firmware.sh [-v] FirmwareDir"
    echo "  FirmwareDir usually on Windows partition."
    echo "  E.g. /mnt/Windows/Program Files (x86)/HP lt4120 Snapdragon X5 LTE/Image"
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
    fastboot flash -i "$DEVICE_VENDOR" "$PARTITION" "$TMP_NAME"
}

flash_step() {
    DBG "Switching modem to flash mode..."
    qmicli -p -d "$DEVICE" --dms-hp-change-device-mode=fastboot
    # It usually fails because client closing commands send after device became unavailable

    RETRIES=0
    while [ -z "$DEVS" -a $RETRIES -le 30 ]; do
        sleep 1
        DBG "Checking if fastboot device available - attempt $RETRIES"
        DEVS=`fastboot devices -i "$DEVICE_VENDOR"`
        RETRIES=`expr $RETRIES + 1`
    done

    if [ $RETRIES -ge 30 ]; then
        FATAL "Device not found by fastboot. Something went wrong?"
        exit 1
    fi

    flash mibib "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/partition.mbn
    flash sbl "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/sbl1.mbn
    flash sdi "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/sdi.mbn
    flash tz "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/tz.mbn
    flash mba "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/mba.mbn
    flash rpm "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/rpm.mbn
    flash qdsp "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/qdsp6sw.mbn
    flash aboot "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/appsboot.mbn
    flash boot "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/mdm9625-boot.img
    flash system "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/mdm9625-sysfs.yaffs2
    flash userdata "$IMAGE_DIR"/"$FIRMWARE_VARIANT"/*/mdm9625-usrfs.yaffs2
    fastboot -i "$DEVICE_VENDOR" erase efs2
    fastboot -i "$DEVICE_VENDOR" reboot
    DBG "Waiting while device booted..."

    RETRIES=0
    while [ ! -e "$DEVICE" -a $RETRIES -le 30 ]; do
        sleep 1
        DBG "Checking if modem device available - attempt $RETRIES"
        RETRIES=`expr $RETRIES + 1`
    done
    if [ $RETRIES -ge 30 ]; then
        FATAL "Device not found. Something went wrong?"
        exit 1
    fi
}

config_upload_step() {
    for CONFIG in `ls -1 $IMAGE_DIR/*/MCFG*/mcfg_sw.mbn`; do
        DBG "Uploading config $CONFIG"
        TMP_NAME="$MYTMPDIR/mcfg_sw.mbn"
        tail -c +13 <"$CONFIG" >"$TMP_NAME"
        qmicli -p -d "$DEVICE" --pdc-load-config="$TMP_NAME"
    done
    qmicli -p -d "$DEVICE" --pdc-list-configs=software
}

select_config_step() {
    qmicli -p -d "$DEVICE" --pdc-activate-config="$ACTIVE_CONFIG_ID"
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

flash_step
config_upload_step
select_config_step
