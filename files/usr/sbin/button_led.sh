# fork by /etc/hotpulg.d/button/001-timer
#
# When time's up, check the syslogd log message.
# If button $ACTION = "released" in the log message, user released the button during the sleep.
# $1: timer

iTimer=$(($1+1)) # if timer = 10, then blink the power led at 11th sec
sleep $iTimer

iCheck=$(grep "released" /tmp/button_log)

if [ -z $iCheck ]; then
	echo 1 > /sys/devices/platform/leds-gpio/leds/power/brightness
	echo timer > /sys/devices/platform/leds-gpio/leds/power/trigger
	echo 200 > /sys/devices/platform/leds-gpio/leds/power/delay_on
	echo 200 > /sys/devices/platform/leds-gpio/leds/power/delay_off
fi

ps|grep "logread -f"|grep -v "grep"|awk '{print $1}'|xargs kill
rm -f /tmp/button_log
