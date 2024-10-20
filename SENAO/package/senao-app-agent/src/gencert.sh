#!/bin/sh
PATH=$PATH:/sbin:/bin:/usr/sbin/:/usr/bin
export PATH

if [ ! "$#" = 4 ]; then
	echo "Usage: $0 <PRODUCT_NAME> <OPENSSL_CONFIG> <KEY_FILE> <ORGANIZATION_NAME>"
	exit 1
fi

#
# Parameter
#
PRODUCT_NAME=$1
OPENSSL_CFG=$2
KEYFILE=$3
ORGANIZATION_NAME=$4

# temporarily file
KEY=key
CSR=csr
CRT=crt
TMPCFG=tmpcfg
TMPCFG2=tmpcfg2

sed -e "s/DEFAULT_COMMON_NAME/${PRODUCT_NAME}/" $OPENSSL_CFG > $TMPCFG
if [ $? != 0 -o ! -e $TMPCFG ]; then
	echo "ERROR: openssl config file!"
	exit 1
fi

sed -e "s/DEFAULT_ORGANIZATION_NAME/${ORGANIZATION_NAME}/" $TMPCFG > $TMPCFG2
if [ $? != 0 -o ! -e $TMPCFG2 ]; then
	echo "ERROR: openssl config file!"
	exit 1
fi
mv -f $TMPCFG2 $TMPCFG

openssl genrsa 2048 > $KEY
if [ $? != 0 -o ! -e $KEY ]; then
	echo "ERROR: generate rsa key!"
	exit 1
fi

openssl req -new -key $KEY -out $CSR -config $TMPCFG -batch
if [ $? != 0 -o ! -e $CSR ]; then
	echo "ERROR: generate new request!"
	exit 1
fi

openssl req -x509 -key $KEY -in $CSR -days 3650 -out $CRT
if [ $? != 0 -o ! -e $CRT ]; then
	echo "ERROR: generate x509 certificate!"
	exit 1
fi

cat $KEY $CRT > $KEYFILE
echo "Certificate file $KEYFILE generated."

# remove temporarily files
rm -f $KEY $CSR $CRT $TMPCFG
echo "done."
