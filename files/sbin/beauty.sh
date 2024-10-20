#!/bin/bash

#red
recho() {
    if [ "${1}" == "-n" ];then
        shift
        echo -e -n "\033[31m$*\033[0m"
    else
        echo -e "\033[31m$*\033[0m"
    fi
}
#green
gecho() {
    if [ "${1}" == "-n" ];then
        shift
        echo -e -n "\033[32m$*\033[0m"
    else
        echo -e "\033[32m$*\033[0m"
    fi
}
#blue
becho() {
    if [ "${1}" == "-n" ];then
        shift
        echo -e -n "\033[36m$*\033[0m"
    else
        echo -e "\033[36m$*\033[0m"
    fi
}
#yellow
yecho() {
    if [ "${1}" == "-n" ];then
        shift
        echo -e -n "\033[33m$*\033[0m"
    else
        echo -e "\033[33m$*\033[0m"
    fi
}
# echo -e "\033[31m 紅色字\033[0m"
# echo -e "\033[34m 黃色字\033[0m"
# echo -e "\033[41;33m 紅底黃字\033[0m"
# echo -e "\033[41;37m 紅底白字\033[0m"
# echo -e "\033[30m 黑色字\033[0m"
# echo -e "\033[31m 紅色字\033[0m"
# echo -e "\033[32m 綠色字\033[0m"
# echo -e "\033[33m 黃色字\033[0m"
# echo -e "\033[34m 藍色字\033[0m"
# echo -e "\033[35m 紫色字\033[0m"
# echo -e "\033[36m 天藍字\033[0m"
# echo -e "\033[37m 白色字\033[0m"
# echo -e "\033[40;37m 黑底白字\033[0m"
# echo -e "\033[41;37m 紅底白字\033[0m"
# echo -e "\033[42;37m 綠底白字\033[0m"
# echo -e "\033[43;37m 黃底白字\033[0m"
# echo -e "\033[44;37m 藍底白字\033[0m"
# echo -e "\033[45;37m 紫底白字\033[0m"
# echo -e "\033[46;37m 天藍底白字\033[0m"
# echo -e "\033[47;30m 白底黑字\033[0m"
#custom echo
cecho() {
    local bcolor=$1
    local fcolor=$2
    shift
    shift
    echo -e "\033[${bcolor};${fcolor}m$*\033[0m"
}


