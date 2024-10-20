#!/bin/sh

#
# Frank Shen Create @ 201803
#
# button control API script
#

########################################################
# Definition
########################################################
BTN_CFG="/etc/factory.d/btn.cfg"

if [ -e $BTN_CFG ]; then
    . $BTN_CFG
else
    echo "$BTN_CFG is not exist!";
    exit 1;
fi

BTN_DEF="/etc/factory.d/btn.def"

if [ -e $BTN_DEF ]; then
    . $BTN_DEF
else
    echo "$BTN_DEF is not exist!";
    exit 1;
fi

########################################################
# sub-Functions
########################################################
#-----------------------------------------------------
# usage
#-----------------------------------------------------
usage() {
    echo "Usage:"
    echo "btn.sh [status]"
    echo "btn.sh [-h]"
}

#-----------------------------------------------------
# show_btn_status
# N/A
#-----------------------------------------------------
show_btn_status() {
    index=0;
	for btn_name in $BTN_NAME; do
        index=$(($index + 1));
        gpio=`echo $BTN_GPIO|cut -d " " -f $index`
        echo "$btn_name `cat $BTN_GPIO_PATH/gpio$gpio/value`"
    done
}


########################################################
# MAIN
########################################################
case $1 in
    "status")
		show_btn_status;
        break;
        ;;
    *)
        usage;
        break;
        ;;
esac

