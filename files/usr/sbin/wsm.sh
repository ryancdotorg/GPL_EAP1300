#!/bin/sh

SIGNAL_PID_FILE=/var/run/wtp.signal.pid
SIGNAL_INPUT_DIR=/tmp/wtpsignal
SIGNAL_OUTPUT_DIR=/tmp
SIGNAL_FILE_NAME=wtp_sig.$$
SIGNAL_INPUT_FILE=$SIGNAL_INPUT_DIR/$SIGNAL_FILE_NAME
SIGNAL_OUTPUT_FILE=$SIGNAL_OUTPUT_DIR/$SIGNAL_FILE_NAME

opt_json=0

#parse the parameter
for cmdlist
do
	case ${cmdlist} in
	"-j")
		opt_json=1
	;;
	esac
done

if [ -f $SIGNAL_PID_FILE ]; then
	pid=`cat $SIGNAL_PID_FILE`
else
	echo "Error: failed to get pid file"
	exit 1
fi

if [ ! -d $SIGNAL_INPUT_DIR ]; then
	mkdir -p $SIGNAL_INPUT_DIR
fi

case $1 in
"status")
	echo "wtp_all_status" > $SIGNAL_INPUT_FILE
	echo "json=$opt_json" >> $SIGNAL_INPUT_FILE
	;;
"mem")
	echo "wtp_mem_info" > $SIGNAL_INPUT_FILE
	;;
"detectdfs")
	echo "wtp_detect_dfs" > $SIGNAL_INPUT_FILE
	echo $2 >> $SIGNAL_INPUT_FILE
	echo $3 >> $SIGNAL_INPUT_FILE
	;;
*)
	echo "Usage: ${0} <command>"
	echo "Available command:"
	echo "status                       show all of status"
	echo "mem                          show memory allocation info"
	echo "detectdfs {oldch} {newCh}    show memory allocation info"
	exit 1
esac

kill -SIGUSR2 $pid
for cnt in 1 2 3 4 5 6 7 8 9 10
do
	if [ -f $SIGNAL_OUTPUT_FILE ]; then
		cat $SIGNAL_OUTPUT_FILE
		rm -f $SIGNAL_OUTPUT_FILE
		exit 0
	else
		if [ $cnt -eq 10 ]; then
			echo "Error: failed to get output file"
			exit 1
		else
			usleep 100000
		fi
	fi
done
