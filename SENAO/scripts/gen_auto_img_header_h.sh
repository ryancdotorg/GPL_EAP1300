#!/bin/bash

type=$1
PKG_BUILD_DIR=$2
SN_CONFIGS_DIR=$3
TOPDIR=$4
SN_SCRIPTS_DIR=$5
PRODUCT_NAME=$6

echo "type="$type
echo "SN_CONFIGS_DIR"=$SN_CONFIGS_DIR
AUTO_IMG_HEADER_FILE=$PKG_BUILD_DIR/h/auto_img_header.h

if [ "$type" != "default" ];then
    #generate partitions_size.txt by Get_partitions_size.py
    cd $PKG_BUILD_DIR/PARTITION_FILES/
    rm -f partitions_size.txt
    python Get_partitions_size.py -t $type -B -F $SN_CONFIGS_DIR/ipq-image-config/boardconfig .
    /bin/cp -f partitions_size.txt $TOPDIR/tmp/
fi


#auto generate MAX_CODE_SIZE
case $type in
    default)
        # TODO: need to parsing sn_flash_map.mk
        echo "#define MAX_CODE_SIZE  0x4000000" > $AUTO_IMG_HEADER_FILE
        ;;
    nor)
        partitions=$(grep -sw "Size" partitions_size.txt | awk -F ":" '{print $2}')
        K_SIZE=$(echo $partitions | awk -F " " '{print $1}')
        FS_SIZE=$(echo $partitions | awk -F " " '{print $2}')
        TotalSize=$(($K_SIZE+$FS_SIZE))
        HexTotalSize=`echo "obase=16;ibase=10; $TotalSize" | bc`
        echo "#define MAX_CODE_SIZE  0x$HexTotalSize" > $AUTO_IMG_HEADER_FILE
        ;;
    norplusnand)
        HexTotalSize=$(grep -sw "Size" partitions_size.txt | awk -F ":" '{print $2}')
        echo "HexTotalSize="$HexTotalSize
        echo "#define MAX_CODE_SIZE $HexTotalSize" > $AUTO_IMG_HEADER_FILE
        ;;
    nand)
        HexTotalSize=$(grep -sw "Size" partitions_size.txt | awk -F ":" '{print $2}')
        echo "HexTotalSize="$HexTotalSize
        echo "#define MAX_CODE_SIZE $HexTotalSize" > $AUTO_IMG_HEADER_FILE
        ;;
    *)
        echo "sorry do not support type:"$type
        exit 1
        ;;
esac

date=`$SN_SCRIPTS_DIR/fw_version.sh $TOPDIR |grep date |cut -d: -f2|awk -F\- '{printf $1$2$3}'`
echo "#define FIRMWARE_DATE  ${date:2:6}" >> $AUTO_IMG_HEADER_FILE

firmware_ver=`$SN_SCRIPTS_DIR/fw_version.sh $TOP_DIR |grep version |awk -F ":" '{printf $2}'| awk -F "." '{printf $1"."$2"."$3}'`;
echo "#define FIRMWARE_VERSION  \"${firmware_ver}\"" >> $AUTO_IMG_HEADER_FILE

if [ -f $SN_CONFIGS_DIR/capwap.ver ]; then
    capwap_ver=`cat $SN_CONFIGS_DIR/capwap.ver`;
    echo "#define CAPWAP_VERSION  \"${capwap_ver}\"" >> $AUTO_IMG_HEADER_FILE
fi

echo "#define MODEL_NAME  \"${PRODUCT_NAME}\"" >> $AUTO_IMG_HEADER_FILE
