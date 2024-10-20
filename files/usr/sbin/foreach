#!/bin/sh
# Copyright (C) 2006 OpenWrt.org
. /lib/functions.sh

config=$1
sectiontype=$2
option=$3
equalvalue=$4

get_section_name(){
	local section=$1
	local option=$2
	local optionvalue
	config_get optionvalue "$section" "$option"
	[ "${optionvalue}" = "$equalvalue" ] && echo ${section}

}
	config_load "$1"
		config_foreach get_section_name "$2" "$3"
