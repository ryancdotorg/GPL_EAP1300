TYPE=$1
IP=$2
SOURCE=$3
RESULT=/tmp/$TYPE.Resutl.$$
MODEL=`tr '[a-z]' '[A-Z]' < /etc/modelname`
if [ -z $3 ]; then
SOURCE=$MODEL/wtp
fi

echo $IP
echo $SOURCE
echo $RESULT
echo $TYPE

#get wtp from server
if [ "$TYPE" == "wget" ]; then
	`cd /tmp/;wget $IP -O /tmp/wtp 2>$RESULT`
	ERROR=`cat $RESULT | grep "100%" -c`
	if [ "$ERROR" == "0" ]; then
		ERROR="1"
	else
		ERROR="0"
	fi
	echo "weget@@@@"
else
	`cd /tmp/;tftp -gr $SOURCE $IP 2>$RESULT`
	ERROR=`cat $RESULT | grep tftp -c`
	echo "tftp ####"
fi



if [ "$ERROR" == "0" ]; then
	`killall -9 wtp `
	echo 'killall wtp'
	`chmod 777 /tmp/wtp` 
	echo "chmod"
	`cp /tmp/wtp /usr/bin/wtp`
	echo "copy to /usr/bin"
	echo 'reboot'
	`reboot`
	exit
else
	echo "error:failed to download with $TYPE"
fi
echo `cat $RESULT`
`rm $RESULT`
