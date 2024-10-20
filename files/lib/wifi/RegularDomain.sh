#!/bin/sh

country_id="$1"
sku="$2"
green="$3"
outdoor="$4"
dfscertified="$5"
tri_band="$6"
fakeCertified="$7"

# display help
[ ! -n "$country_id" ] && {
echo RegularDomain.sh country domain obey outdoor dfsCertified check_wifi2
exit
}

# Get dfs certified
fccDfsCertified=$(uci -q get sysProductInfo.model.fccDfsCertified || echo -n "0")
etsiDfsCertified=$(uci -q get sysProductInfo.model.etsiDfsCertified || echo -n "0")
intDfsCertified=$(uci -q get sysProductInfo.model.intDfsCertified || echo -n "0")
defaultNetherlands=1

# check triband
[ -n "$tri_band" -a "$tri_band" != "0" ] && tri_band=1
[ ! -n "$tri_band" ] && tri_band=0

# extend
[ "$(uci -q get wifiprofile.snWifiConf.SUPPORT_FORCE_GREEN_MODE)" == "1" ] && green=1

# Get DFS certified from sysProductInfo
get_dfs_certified() {
	case ${sku} in
		0)  # FCC
			dfscertified=$fccDfsCertified
			;;
		1)  # ETSI/EU
			dfscertified=$etsiDfsCertified
			;;
		2|*)# INT/others
			case "$country_id" in
				00) #Default Country ID
					if [ "$defaultNetherlands" -eq 1 ]; then
						dfscertified=$etsiDfsCertified
					else
						dfscertified=$fccDfsCertified
					fi
					# replace to Netherlands
					country_id=528
					;;
				32|76|124|152|158|170|188|214|218|320|340|484|591|604|630|840|858|862) #FCC Country ID
					dfscertified=$fccDfsCertified
					;;
				8|40|56|100|191|203|208|233|246|250|276|300|348|352|372|380|398|428|440|442|492|528|578|616|620|642|643|703|705|724|752|756|804|807|826) #ETSI Country ID
					dfscertified=$etsiDfsCertified
					;;
				*)
					dfscertified=$intDfsCertified
					;;
			esac
			;;
	esac
}

# Check green mode / tri_band / outddor / dfs certified to decide entrylist
get_country_band() {
	band_list=""
	if [ $green -eq 0 ]; then
		band_list="$f3"
	elif [ $tri_band -eq 1 ]; then
		band_list="$f8"
	else
		case "$outdoor:$dfscertified" in
		0:0) band_list="$f6";;
		0:*) band_list="$f4";;
		1:0) band_list="$f7";;
		1:*) band_list="$f5";;
		*) band_list="1,2,3,4";;
		esac
	fi
	echo "$band_list"
}

# band list convert to bitmap
get_band_bit_map() {
	local band_list="$1"
	local band1=$(echo $band_list | grep 1  > /dev/null 2>&1 && echo 1 || echo 0)
	local band2=$(echo $band_list | grep 2  > /dev/null 2>&1 && echo 2 || echo 0)
	local band3=$(echo $band_list | grep 3  > /dev/null 2>&1 && echo 4 || echo 0)
        local band4=$(echo $band_list | grep 4  > /dev/null 2>&1 && echo 8 || echo 0)

	echo "$(($band4+$band3+$band2+$band1))"
}


# if fakeCertified = 1, use dfscertified from command
# if fakeCertified = 0 or null, use dfscertified from sysProductInfo
[ "$fakeCertified" != "1" ] && get_dfs_certified

# main function
while read f1 f2 f3 f4 f5 f6 f7 f8 f9
do
	if [ "$country_id" == "$f2" ]; then
		country_name="$f1"
		country_band="$(get_country_band)";
		band_bit_map="$(get_band_bit_map $country_band)";
		if [ "$f9" = "Y" ]; then
			echo -n "$band_bit_map" 1
		else
			echo -n "$band_bit_map" 0
		fi
	fi
done < /lib/wifi/RegularDomain.csv 
