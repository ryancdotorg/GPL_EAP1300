#!/bin/bash

TOP="$1"
TARGET="$2"
VERSION_CONFIG="$TOP/SENAO/configs/version_config"

if [ ! -e $VERSION_CONFIG ]; then
	echo ""
	echo ""
	echo "$VERSION_CONFIG not exist !!"
	echo ""
	echo ""
	exit
fi

. $VERSION_CONFIG

COMMIT_VER="$(git log --oneline -n 1 | sed -e 's/ .*//g')"

BUILD_DATE="$(date +"%Y-%m-%d %H:%M")"
BUILD_DATE1="$(date +%y%m%d)"

BANNER="************************************************************************"

if [ -z $TARGET ];then
	echo "version:$MAJOR_VERSION.$MINOR_VERSION.$RELEASE_VERSION.$BUILD_VERSION"
	echo "date:${BUILD_DATE}"
else
	mkdir -p $(dirname $TARGET)
	echo -e "$MAJOR_VERSION.$MINOR_VERSION.$RELEASE_VERSION.$BUILD_VERSION build-$BUILD_DATE1 ($COMMIT_VER)\n$BANNER\nFirmware Version : $MAJOR_VERSION.$MINOR_VERSION.$RELEASE_VERSION.$BUILD_VERSION		Build Date : ${BUILD_DATE}\n$BANNER\n" > $TARGET
fi

