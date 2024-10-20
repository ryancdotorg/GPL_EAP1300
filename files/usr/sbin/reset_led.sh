# fork by /etc/hotpulg.d/button/001-timer
#
# When time's up, check the syslogd log message.
# If button $ACTION = "released" in the log message, user released the button during the sleep.
# $1: timer

iTimer=$(($1+1)) # if timer = 10, then blink the power led at 11th sec
sleep $iTimer

iCheck=$(grep "pressed" /tmp/button_log)

power_led=$(uci get system.power_led.sysfs)

if [ -n "$iCheck" ]; then
	echo 1 > /sys/class/leds/$power_led/brightness
	echo timer > /sys/class/leds/$power_led/trigger
	echo 200 > /sys/class/leds/$power_led/delay_on
	echo 200 > /sys/class/leds/$power_led/delay_off
fi

ps|grep "logread -f"|grep -v "grep"|awk '{print $1}'|xargs kill
