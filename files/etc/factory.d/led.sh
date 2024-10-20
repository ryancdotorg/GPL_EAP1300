#!/bin/sh

#
# Frank Shen Create @ 201803
#
# led control API script
#

########################################################
# Definition
########################################################
LED_DEF="/etc/factory.d/led.def"

if [ -e $LED_DEF ]; then
    . $LED_DEF
else
    echo "$LED_DEF is not exist!";
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
    echo "led.sh [on/off] [led_name]"
    echo "led.sh [status]"
    echo "led.sh [-h]"
}

#-----------------------------------------------------
# one_led_onoff
# $1=on/off; $2=led_name  
#-----------------------------------------------------
one_led_onoff() {
    led_ctl_file=$LED_SYSTEM_PATH$2/$BRIGHTNESS

    if [ -e $led_ctl_file ]; then
        echo none > $LED_SYSTEM_PATH$2/$TRIGGER;
        if [ $1 == "on" ];then
            echo 1 > $led_ctl_file
        else
            echo 0 > $led_ctl_file
        fi
    else
        echo "fail"
    fi
}

#-----------------------------------------------------
# all_led_onoff
# $1=on/off;
#-----------------------------------------------------
all_led_onoff() {
	for i in `ls $LED_SYSTEM_PATH`; do
        for j in $NOT_LED_LIST; do
			not_led_flag=0;
			if [ $i == $j ]; then
				not_led_flag=1;
				break;
			fi
		done
        if [ $not_led_flag == "0" ]; then
            echo none > $LED_SYSTEM_PATH$i/$TRIGGER;
            if [ $1 == "on" ]; then
                echo 1 > $LED_SYSTEM_PATH$i/$BRIGHTNESS;
            elif [ $1 == "off" ]; then
                echo 0 > $LED_SYSTEM_PATH$i/$BRIGHTNESS;
            else
                echo "fail";
            fi
        fi
    done
}

#-----------------------------------------------------
# show_led_status
# N/A
#-----------------------------------------------------
show_led_status() {
	for i in `ls $LED_SYSTEM_PATH`; do
		for j in $NOT_LED_LIST; do
			not_led_flag=0;
			if [ $i == $j ]; then
				not_led_flag=1;
				break;
			fi
		done
	[ $not_led_flag == "0" ] && echo $i `cat $LED_SYSTEM_PATH$i/$BRIGHTNESS`;
	done
}


########################################################
# MAIN
########################################################
case $1 in
    "on" | "off")
        if [ -n "$2" ]; then
            echo "turn $1 $2"; # turn xxx on/off
            one_led_onoff $1 $2;
        else
            echo "turn all led $1"; # turn all led on/off
            all_led_onoff $1;
        fi
        break;
        ;;
    "status")
		show_led_status;
        break;
        ;;
    *)
        usage;
        break;
        ;;
esac

