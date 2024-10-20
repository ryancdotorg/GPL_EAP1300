#!/bin/sh

cd /usr/lib/
rm -rf libnetsnmp*.so.*
tftp -g -r libnetsnmpagent.so.15.1.2 $1
tftp -g -r libnetsnmphelpers.so.15.1.2 $1
tftp -g -r libnetsnmp.so.15.1.2 $1
tftp -g -r libnetsnmpmibs.so.15.1.2 $1
tftp -g -r libnetsnmptrapd.so.15.1.2 $1
chmod 755 libnetsnmp*.so.15.1.2
ln -sf libnetsnmpagent.so.15.1.2 libnetsnmpagent.so.15
ln -sf libnetsnmpagent.so.15 libnetsnmpagent.so
ln -sf libnetsnmphelpers.so.15.1.2 libnetsnmphelpers.so.15
ln -sf libnetsnmphelpers.so.15 libnetsnmphelpers.so
ln -sf libnetsnmp.so.15.1.2 libnetsnmp.so.15
ln -sf libnetsnmp.so.15 libnetsnmp.so
ln -sf libnetsnmpmibs.so.15.1.2 libnetsnmpmibs.so.15
ln -sf libnetsnmpmibs.so.15 libnetsnmpmibs.so
ln -sf libnetsnmptrapd.so.15.1.2 libnetsnmptrapd.so.15
ln -sf libnetsnmptrapd.so.15 libnetsnmptrapd.so

