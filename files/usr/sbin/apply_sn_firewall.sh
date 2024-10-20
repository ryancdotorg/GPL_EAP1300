TARGET_CONFFILE="/etc/firewall.user"
CONFFILE_DIR="/etc/firewall.rules"
PREFIX="firewall."
SUFFIX=".insert"

#list="dos alg"

### DoS : portscan + syncflood + icmp ###
dos_list="portscan syncflood icmp"

alg_list="alg passthrough"

urlblock_list="urlblock"

### TODO: use array
#list=(${dos_list} ${alg_list})

FW_DOS_INSERT=${CONFFILE_DIR}/${PREFIX}"dos"${SUFFIX}
FW_DOS=${CONFFILE_DIR}/${PREFIX}"dos"

FW_ALG_INSERT=${CONFFILE_DIR}/${PREFIX}"natalg"${SUFFIX}
FW_ALG=${CONFFILE_DIR}/${PREFIX}"natalg"

FW_URLBLOCK_INSERT=${CONFFILE_DIR}/${PREFIX}"urlblock"${SUFFIX}
FW_URLBLOCK=${CONFFILE_DIR}/${PREFIX}"urlblock"

reverse_order(){
	[ -e $1 ] && sed -n -i '1!G;h;$p' $1
}

concatenate_insert() {
	[ -e ${CONFFILE_DIR}/${PREFIX}$1${SUFFIX} ] && cat ${CONFFILE_DIR}/${PREFIX}$1${SUFFIX} >> $2
}

concatenate() {
	[ -e ${CONFFILE_DIR}/${PREFIX}$1 ] && cat ${CONFFILE_DIR}/${PREFIX}$1 >> $2 
}

prepareDosRules() {
	# clean old files
	rm -f $FW_DOS $FW_DOS_INSERT
}

setDosRules() {
	for name in ${dos_list}; do
		concatenate_insert $name $FW_DOS_INSERT
		concatenate $name $FW_DOS
	done;
	reverse_order $FW_DOS
}

prepareAlgRules() {
	rm -f $FW_ALG $FW_ALG_INSERT
}

setAlgRules() {
	for name in ${alg_list}; do
		concatenate_insert $name $FW_ALG_INSERT
		concatenate $name $FW_ALG
	done;
	reverse_order $FW_ALG
}

prepareURLBlock() {
	rm -f $FW_URLBLOCK $FW_URLBLOCK_INSERT
}
setURLBlock() {
#	for name in ${urlblock_list}; do
#		concatenate_insert $name $FW_URLBLOCK_INSERT
#		concatenate $name $FW_URLBLOCK
#	done;
	reverse_order $FW_URLBLOCK
}

prepareDosRules
setDosRules

prepareAlgRules
setAlgRules

#prepareURLBlock
setURLBlock

# recovery firewall.user
rm -f ${TARGET_CONFFILE}
if [ 1 = $(uci get sn_firewall.firewall.enable) ]; then
	cp -f /rom/${TARGET_CONFFILE} ${TARGET_CONFFILE}
else
	echo "" > ${TARGET_CONFFILE}
	exit 0
fi

### count the files for appending rules below the exsiting rules.
lines=$(wc -l ${TARGET_CONFFILE}|cut -d ' ' -f 1)

### execute a files contained chain rules
### we reversed the order of rules, so we need to apply the chain rules in the beginning.
echo "sh /tmp/firewall.chain" >> ${TARGET_CONFFILE}

echo "### ALG ###" >> ${TARGET_CONFFILE}
cat $FW_ALG >> ${TARGET_CONFFILE}

echo "### DOS ###" >> ${TARGET_CONFFILE}
cat $FW_DOS >> ${TARGET_CONFFILE}

echo "### URL Block ###" >> ${TARGET_CONFFILE}
cat $FW_URLBLOCK >> ${TARGET_CONFFILE}

echo "### URL Block INSERT ###" >> ${TARGET_CONFFILE}
cat $FW_URLBLOCK_INSERT >> ${TARGET_CONFFILE}

echo "### DOS INSERT ###" >> ${TARGET_CONFFILE}
cat $FW_DOS_INSERT >> ${TARGET_CONFFILE}

echo "### ALG INSERT ###" >> ${TARGET_CONFFILE}
cat $FW_ALG_INSERT >> ${TARGET_CONFFILE}

### create the files contained chain rules
grep -w '\-N' ${TARGET_CONFFILE} > /tmp/firewall.chain
#sed -i "${lines}i $(grep -w '\-N' ${TARGET_CONFFILE})" ${TARGET_CONFFILE}

### check natalg modules
### TODO: the firewall main page doesen't reload the nat alg
RMMOD=rmmod 2>&1 > /dev/null
INSMOD=insmod 2>&1 > /dev/null
check_modules() {
	if [ 0 == $(uci get sn_firewall.alg.$1) ]; then
		rmmod $3
		rmmod $2
	else
		insmod $2
		insmod $3
	fi
}

check_modules "h323" "nf_conntrack_h323" "nf_nat_h323"
check_modules "tftp" "nf_conntrack_tftp" "nf_nat_tftp"
check_modules "irc" "nf_conntrack_irc" "nf_nat_irc"
check_modules "amanda" "nf_conntrack_amanda" "nf_nat_amanda"
check_modules "ftp" "nf_conntrack_ftp" "nf_nat_ftp"
check_modules "pptp" "nf_conntrack_pptp" "nf_nat_pptp"
