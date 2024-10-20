#!/bin/sh
#
# Copyright (c) 2015 The Linux Foundation. All rights reserved.
# Copyright (C) 2011 OpenWrt.org

. /lib/ipq806x.sh

do_load_ipq4019_board_bin()
{
    local board=$(ipq806x_board_name)
    local mtdblock=$(find_mtd_part 0:ART)

    local apdk="/tmp"

    HK_BD_FILENAME=/lib/firmware/IPQ8074/bdwlan.bin

    # SENAO link BDF
    local soc_version_major=1
    HK01_BD_FILENAME=/lib/firmware/IPQ8074/WIFI_FW/bdwlan.b20
    HK07_BD_FILENAME=/lib/firmware/IPQ8074/WIFI_FW/bdwlan.b70
    AC01_BD_FILENAME=/lib/firmware/IPQ8074/WIFI_FW/bdwlan.b91
    [ -f /sys/firmware/devicetree/base/soc_version_major ] && {
            soc_version_major="$(hexdump -n 1 -e '"%1d"' /sys/firmware/devicetree/base/soc_version_major)"
    }

    if [ -z "$mtdblock" ]; then
        # read from mmc
        mtdblock=$(find_mmc_part 0:ART)
    fi

    [ -n "$mtdblock" ] || return

    # SENAO link BDF
    if [ $soc_version_major = 1 ]; then
        case "$board" in
                ap-hk01-*)
    		rm ${HK_BD_FILENAME}
                    ln -s ${HK01_BD_FILENAME} ${HK_BD_FILENAME}
                ;;
                ap-hk07)
    		rm ${HK_BD_FILENAME}
                    ln -s ${HK07_BD_FILENAME} ${HK_BD_FILENAME}
                ;;
                ap-ac01*)
    		rm ${HK_BD_FILENAME}
                    ln -s ${AC01_BD_FILENAME} ${HK_BD_FILENAME}
                ;;
        esac
    fi

    # load board.bin
    case "$board" in
            ap-dk0*)
                    mkdir -p ${apdk}
                    dd if=${mtdblock} of=${apdk}/wifi0.caldata bs=32 count=377 skip=128
                    dd if=${mtdblock} of=${apdk}/wifi1.caldata bs=32 count=377 skip=640
            ;;
            ap16* | ap148*)
                    mkdir -p ${apdk}
                    dd if=${mtdblock} of=${apdk}/wifi0.caldata bs=32 count=377 skip=128
                    dd if=${mtdblock} of=${apdk}/wifi1.caldata bs=32 count=377 skip=640
                    dd if=${mtdblock} of=${apdk}/wifi2.caldata bs=32 count=377 skip=1152
            ;;
            ap-hk01-*)
                    mkdir -p ${apdk}/IPQ8074
                    if [ -f "$HK_BD_FILENAME" ]; then
                        FILESIZE=$(stat -Lc%s "$HK_BD_FILENAME")
                    else
                        FILESIZE=131072
                    fi
                    dd if=${mtdblock} of=${apdk}/IPQ8074/caldata.bin bs=1 count=$FILESIZE skip=4096
                    [ -L /lib/firmware/IPQ8074/caldata.bin ] || \
                    ln -s ${apdk}/IPQ8074/caldata.bin /lib/firmware/IPQ8074/caldata.bin

                    PCIE_0_DEVICE_ID="$(cat /sys/bus/pci/devices/0000\:01\:00.0/device)"
                    PCIE_1_DEVICE_ID="$(cat /sys/bus/pci/devices/0001\:01\:00.0/device)"

                    if [ "${PCIE_0_DEVICE_ID}" == "0x1104" ]; then  # QCN9000_DEVICE_ID = 0x1104
                        mkdir -p ${apdk}/qcn9000
                        dd if=${mtdblock} of=${apdk}/qcn9000/caldata_1.bin bs=1 count=$FILESIZE skip=157696
                        ln -s ${apdk}/qcn9000/caldata_1.bin /lib/firmware/qcn9000/caldata_1.bin
                    elif [ "${PCIE_0_DEVICE_ID}" == "0x003c" -o "${PCIE_0_DEVICE_ID}" == "0x0050" ]; then # AR9888_DEVICE_ID="0x003c"  AR9887_DEVICE_ID="0x0050"
                        dd if=${mtdblock} of=${apdk}/wifi1.caldata bs=1 count=2116 skip=208896
                    fi

                    if [ "${PCIE_1_DEVICE_ID}" == "0x1104" ]; then  # QCN9000_DEVICE_ID = 0x1104
                        mkdir -p ${apdk}/qcn9000
                        dd if=${mtdblock} of=${apdk}/qcn9000/caldata_2.bin bs=1 count=$FILESIZE skip=311296
                        ln -s ${apdk}/qcn9000/caldata_2.bin /lib/firmware/qcn9000/caldata_2.bin
                    elif [ "${PCIE_1_DEVICE_ID}" == "0x003c" -o "${PCIE_1_DEVICE_ID}" == "0x0050" ]; then # AR9888_DEVICE_ID="0x003c"  AR9887_DEVICE_ID="0x0050"
                        dd if=${mtdblock} of=${apdk}/wifi2.caldata bs=1 count=2116 skip=311296
                    fi
            ;;
            ap-hk* | ap-ac* | ap-oa*)
                    mkdir -p ${apdk}/IPQ8074
                    dd if=${mtdblock} of=${apdk}/wifi1.caldata bs=1 count=12064 skip=208896
                    if [ -f "$HK_BD_FILENAME" ]; then
                        FILESIZE=$(stat -Lc%s "$HK_BD_FILENAME")
                    else
                        FILESIZE=131072
                    fi
                    dd if=${mtdblock} of=${apdk}/IPQ8074/caldata.bin bs=1 count=$FILESIZE skip=4096
                    [ -L /lib/firmware/IPQ8074/caldata.bin ] || \
                    ln -s ${apdk}/IPQ8074/caldata.bin /lib/firmware/IPQ8074/caldata.bin

                    PCIE_0_DEVICE_ID="$(cat /sys/bus/pci/devices/0000\:01\:00.0/device)"
                    PCIE_1_DEVICE_ID="$(cat /sys/bus/pci/devices/0001\:01\:00.0/device)"

                    if [ "${PCIE_0_DEVICE_ID}" == "0x1104" ]; then  # QCN9000_DEVICE_ID = 0x1104
                        mkdir -p ${apdk}/qcn9000
                        dd if=${mtdblock} of=${apdk}/qcn9000/caldata_1.bin bs=1 count=$FILESIZE skip=157696
                        ln -s ${apdk}/qcn9000/caldata_1.bin /lib/firmware/qcn9000/caldata_1.bin
                    elif [ "${PCIE_0_DEVICE_ID}" == "0x003c" -o "${PCIE_0_DEVICE_ID}" == "0x0050" ]; then # AR9888_DEVICE_ID="0x003c"  AR9887_DEVICE_ID="0x0050" 
                        dd if=${mtdblock} of=${apdk}/wifi1.caldata bs=1 count=2116 skip=208896
                    fi

                    if [ "${PCIE_1_DEVICE_ID}" == "0x1104" ]; then  # QCN9000_DEVICE_ID = 0x1104
                        mkdir -p ${apdk}/qcn9000
                        dd if=${mtdblock} of=${apdk}/qcn9000/caldata_2.bin bs=1 count=$FILESIZE skip=311296
                        ln -s ${apdk}/qcn9000/caldata_2.bin /lib/firmware/qcn9000/caldata_2.bin
                    elif [ "${PCIE_1_DEVICE_ID}" == "0x003c" -o "${PCIE_1_DEVICE_ID}" == "0x0050" ]; then # AR9888_DEVICE_ID="0x003c"  AR9887_DEVICE_ID="0x0050" 
                        dd if=${mtdblock} of=${apdk}/wifi2.caldata bs=1 count=2116 skip=311296
                    fi
            ;;
            ap-cp*)
                    CP_BD_FILENAME=/lib/firmware/IPQ6018/bdwlan.bin
                    mkdir -p ${apdk}/IPQ6018
                    dd if=${mtdblock} of=${apdk}/wifi1.caldata bs=1 count=12064 skip=208896
                    if [ -f "$CP_BD_FILENAME" ]; then
                        FILESIZE=$(stat -Lc%s "$CP_BD_FILENAME")
                    else
                        FILESIZE=65536
                    fi
                    dd if=${mtdblock} of=${apdk}/IPQ6018/caldata.bin bs=1 count=$FILESIZE skip=4096
                    [ -L /lib/firmware/IPQ6018/caldata.bin ] || \
                    ln -s ${apdk}/IPQ6018/caldata.bin /lib/firmware/IPQ6018/caldata.bin
            ;;
   esac
}

