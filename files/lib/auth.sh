#!/bin/sh

restfulAPI_account=/etc/senao-openapi-server/senao-openapi-server.dav

debug(){
[ -e "/tmp/CLI_DEBUG" ] && echo "$1" "$2" "$3" "$4" "$5" "$6" "$7" 1>&2- > /dev/ttyS0
}

#rewrite get_username , get_password and psencry on WEBServer changes
CLI_escchar(){
	value="$1"
	value=$(echo "$value"|sed -e 's/\\/\\\\/g')
	value=$(echo "$value"|sed -e 's/"/\\"/g')
	value=$(echo "$value"|sed -e 's/`/\\`/g')
	value=$(echo "$value"|sed -e 's/[$]/\\$/g')
	printf "$value"
}

escape_character(){
local sour="$1"
local dest=
local n=1
local character=$(expr substr "$sour" "$n" 1)
	while [ -n "$character" ] ; do
	    case ${character} in
		'$')
		    dest=$dest"\$";
		;;
		'`')
		    dest=$dest"\`";
		;;
		'*')
		    dest=$dest"\*";
		;;
		*)
		    dest=$dest$character;
		;;
	    esac
	    n=$((n+1))
	    character=$(expr substr "$sour" "$n" 1)
	done

	echo "$dest"
}

psencry(){
	password="$1"
	echo "$password" | md5sum | cut -c -32
	debug password="$password" > /dev/ttyS0
}
get_username(){
grep ":" /etc/webpasswd|cut -d : -f 1
}
get_password(){
user_name=$(get_username)
user_len=${#user_name}
cat /etc/webpasswd|cut -c $(($user_len+2))-
}
login_auth(){
	username="$1"
	password="$2"
	password_hash=$(psencry "$password")
	uname_fail=0
	upass_fail=0
	if [ $(get_username) = "$username" ]; then
		debug "$username" OK.
	else
		uname_fail=1;debug err_user:"$username";
	fi
	if [ $(get_password) = "$password_hash" ];  then
		debug "$password_hash" OK.
	else
		upass_fail=1;debug err_pass:"$password_hash";
	fi	
	printf "$uname_fail,$upass_fail\n"
}
set_restfulAPI_auth(){
	if [ ! -s ${restfulAPI_account} ]; then
		cp /rom/${restfulAPI_account} ${restfulAPI_account}
	fi
	username="$1"
	password_hash="$2"
	username_unescape=$(echo "$username"|sed -e 's/%%/%/g')
	acct_line=$(grep -n -e ".*:.*:admin:[01]" ${restfulAPI_account} | head -1 | awk -F ':' '{print $1}')
	group=$(grep -n -e ".*:.*:admin:[01]" ${restfulAPI_account} | head -1 | awk -F ':' '{print $4}')
	enable=$(grep -n -e ".*:.*:admin:[01]" ${restfulAPI_account} | head -1 | awk -F ':' '{print $5}')
	sed -i ${acct_line}c"${username_unescape}:${password_hash}:${group}:${enable}" $restfulAPI_account
	sync
}
check_restfulAPI_acct(){
	if [ ! -s ${restfulAPI_account} ]; then
		cp /rom/${restfulAPI_account} ${restfulAPI_account}
		sync
	fi
	username="$1"
	password_hash="$2"
	same_acct=$(grep "${username}:${password_hash}" ${restfulAPI_account} | wc -l)
	echo ${same_acct:-0}
}
set_auth(){
	username="$1"
	password="$2"
	password_hash=$(psencry "$password")
	printf "$username:$password_hash\n" > /etc/webpasswd	
	[ -e $restfulAPI_account ] && {
		set_restfulAPI_auth "$username" "$password_hash"
	}
}
set_auth_md5(){
	username="$1"
	password="$2"
	printf "$username:$password\n" > /etc/webpasswd
	[ -e $restfulAPI_account ] && {
		set_restfulAPI_auth "$username" "$password"
	}
}
set_usrname(){
    username="$1"
    [ -n "$username" ] && {
	oldusername=$(get_username)
	oldusername=$(escape_character $oldusername)
        [ -n "$oldusername" ] &&
            sed -i "s/$oldusername/$username/g"  /etc/passwd;
            sed -i "s/$oldusername/$username/g"  /etc/shadow;
            [ "$oldusername" != "$username" ] && 
		sed -i "s/$oldusername/$username/g"  /etc/webpasswd;
    }
}
set_acct(){
	username="$1"
	password="$2"
	set_usrname "$username"
	echo "username=$username" >> /dev/console
	username_escape=$(echo "$username"|sed -e 's/%/%%/g')
	echo "username_escape=$username_escape" >> /dev/console
	set_auth "$username_escape" "$password"
}
set_acct_md5(){
	username="$1"
	password="$2"
	set_usrname "$username"
	echo "username=$username" >> /dev/console
	username_escape=$(echo "$username"|sed -e 's/%/%%/g')
	echo "username_escape=$username_escape" >> /dev/console
	set_auth_md5 "$username_escape" "$password"
}

case $1 in
	psencry)      psencry "$2" ;;
	login_auth)   login_auth "$2" "$3" ;;
	get_username) get_username ;;
	get_password) get_password ;;
	set_auth)     set_auth "$2" "$3" ;;
	set_usrname)  set_usrname "$2" ;;
	set_acct)     set_acct "$2" "$3" ;;
	set_acct_md5) set_acct_md5 "$2" "$3" ;;
	set_restfulAPI_auth)     set_restfulAPI_auth "$2" "$3" ;;
	check_restfulAPI_acct)   check_restfulAPI_acct "$2" "$3" ;;
esac

