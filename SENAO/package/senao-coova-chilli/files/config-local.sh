#!/bin/sh
# Copyright (C) 2009-2012 David Bird (Coova Technologies) <support@coova.com>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

. ./functions.sh
. ./uam.sh
. ./wispr.sh

USERS=/etc/chilli/localusers
COOVA_USERURL=$COOKIE_COOVA_USERURL
COOVA_SESSIONID=$CHI_SESSION_ID
COOVA_CHALLENGE=$CHI_CHALLENGE
PORTAL_SESSIONID=${COOKIE_PORTAL_SESSIONID:-$SESSIONID}
FORM_userurl=${FORM_userurl:-$CHI_USERURL}
FORM_userurl="${FORM_userurl:-${GET_userurl}}"
FORM_userurl="${FORM_userurl:-${POST_userurl}}"
FORM_userurl=${FORM_userurl:-https://www.engeniustech.com/}

http_redirect3() {
printf "HTTP/1.1 302 Found\r\n"
printf "Location: $1\r\n"
printf "Connection: close\r\n"
printf "\r\n"

    exit
}

http_redirect2() {
printf "HTTP/1.1 302 Found\r\n"
printf "Location: $1\r\n"
printf "Set-Cookie: PORTAL_SESSIONID=$PORTAL_SESSIONID\r\n"
printf "Set-Cookie: COOVA_USERURL=$COOVA_USERURL\r\n"
printf "Connection: close\r\n"
printf "\r\n"

    exit
}

http_redirect() {
    http_header
    cat <<EOF
<body onload="document.form1.submit();">
<form action="$1" name="form1" id="form1" method="post">
<input name="res" value="$FORM_res" type="hidden">
<input name="reply" value="$FORM_reply" type="hidden">
</form>
</body>
EOF
printf "\r\n"

    exit
}

http_header() {
#    [ "$HS_MODE" = "hotspot" ] || {
#	http_redirect2 "/www/disabled.chi"
#    }
printf "HTTP/1.1 200 OK\r\n"
printf "Content-Type: text/html\r\n"
printf "Set-Cookie: PORTAL_SESSIONID=$PORTAL_SESSIONID\r\n"
printf "Set-Cookie: COOVA_USERURL=$COOVA_USERURL\r\n"
printf "Connection: close\r\n"
printf "Cache: none\r\n"
printf "\r\n"

}

header() {
    echo "<html><head>"

    uamfile title 0

    echo "<meta http-equiv=\"Cache-control\" content=\"no-cache\"/>
<meta http-equiv=\"Pragma\" content=\"no-cache\"/>
<style>"

    uamfile "css" 0 

    echo "</style>"
    echo "<script>"

    uamfile "js" 0 

    echo "</script>"
    echo "$1</head><body$2>"

    uamfile "header" 1 

    echo "<div id=\"body\">"
}

footer() {
    echo "</div>" 

    uamfile "footer" 1 

    echo "<table style=\"clear:both;margin:auto;padding-top:10px;\" height=\"30\">
<tr><td valign=\"center\" align=\"center\" style=\"color:#666;font-size:60%;\">Powered by</td>
<td valign=\"center\" align=\"center\"><a href=\"http://coova.org/\"><img border=0 src=\"coova.jpg\"></a>
</td></tr></table></body></html>"
}

error() { echo "<div class=\"err\">$1</div>"; }

href() {
    echo "<a href=\"$1\">$2</a>"
}

form() {
    echo "<form name=\"form\" method=\"post\" action=\"$1\"><INPUT TYPE=\"hidden\" NAME=\"userurl\" VALUE=\"$FORM_userurl\">$2</form>"
}

loginform() {
    case "$AUTHENTICATED" in
	1)
	    ;;
	
	*)
	    [ "$HS_OPENIDAUTH" = "on" ] && { \
		echo "<div id=\"login-label\" style=\"display:none;\"><label><a href=\"javascript:toggleAuth('login')\">&lt;&lt; back</a></label></div>"
		form "login.chi" "$(uamfile openid_form 1)"
	    }
	    
	    form "login.chi" "$(uamfile login_form 1)"
	    ;;
    esac
}

local_login_url() {
    if [ "$HS_USELOCALUSERS" = "on" ]; then
	line=$(head -1 $USERS)
	if [ "$line" = "" ]; then
	    echo "tos:$(echo '$$$(date)'|md5sum|cut -f1)" >> $USERS
	    line=$(head -1 $USERS)
	fi
	if [ "$line" != "" ]; then
	    user=$(echo "$line" | cut -f1 -d:)
	    pass=$(echo "$line" | cut -f2 -d:)
	    echo -n $(chi_login_url "$user" "$pass")
	fi
    else
	user=$REMOTE_MAC
	pass=$HS_ADMPWD
	echo -n $(chi_login_url "$user" "$pass")
    fi
}

reply_message() {
    case "$AUTHENTICATED" in
	1)
	    echo "You are now on-line!"
	    ;;

	*)
	   echo "$FORM_reply"
	   ;;
    esac
}

image() {
    ext=$(echo "$1"|awk -v FS=. '{ print tolower($NF) }')
    base=$(echo "$1"|awk -v FS=/ '{ gsub(/[^a-zA-Z0-9_\/-]/,""); print tolower($NF) }')
    echo -n "img-$base.$ext"
}

registerform() {
    form "register.chi" "$(uamfile register_form 1)"
}

contactform() {
    form "contact.chi" "$(uamfile contact_form 1)"
}

termsform() {
    form "tos.chi" "$(uamfile terms_form 1)"
}

runlogin() {
    out=$($CHILLI_QUERY login sessionid "$COOVA_SESSIONID" username "$1" password "$2")
}

chi_login_url() {
    decode=$(echo -e `echo "$2" | sed -f /etc/chilli/url_decode.sed`)
	
	[ -n "$FORM_oauth" ] && HS_RAD_PROTO=oauth

    case "$HS_RAD_PROTO" in
	oauth)
	    echo -n "http://$HS_UAMLISTEN:$HS_UAMPORT/login?username=$1&oauth=$2&userurl=${3:-$COOVA_USERURL}${FORM_redirurl:+&redirurl=${FORM_redirurl}}"
	    ;;
	pap)
	    response=$($CHILLI_RESPONSE -pap "$CHI_CHALLENGE" "$HS_UAMSECRET" "$decode")
	    echo -n "http://$HS_UAMLISTEN:$HS_UAMPORT/login?username=$1&password=${response}&userurl=${3:-$COOVA_USERURL}${FORM_redirurl:+&redirurl=${FORM_redirurl}}${FORM_oauth_AAA:+&provider=$FORM_provider}${FORM_oauth_AAA:+&token=$FORM_token}"
	    ;;
	mschapv2)
	    response=$($CHILLI_RESPONSE -nt "$CHI_CHALLENGE" "$HS_UAMSECRET" "$1" "$decode")
	    echo -n "http://$HS_UAMLISTEN:$HS_UAMPORT/login?username=$1&ntresponse=${response}&userurl=${3:-$COOVA_USERURL}${FORM_redirurl:+&redirurl=${FORM_redirurl}}${FORM_oauth_AAA:+&provider=$FORM_provider}${FORM_oauth_AAA:+&token=$FORM_token}"
	    ;;
	*)
	    response=$($CHILLI_RESPONSE "$CHI_CHALLENGE" "$HS_UAMSECRET" "$decode")
	    echo -n "http://$HS_UAMLISTEN:$HS_UAMPORT/login?username=$1&response=${response}&userurl=${3:-$COOVA_USERURL}${FORM_redirurl:+&redirurl=${FORM_redirurl}}${FORM_oauth_AAA:+&provider=$FORM_provider}${FORM_oauth_AAA:+&token=$FORM_token}"
	    ;;
    esac
}


dologin() {
    url=$(chi_login_url "$FORM_username" "$FORM_password" "$FORM_userurl")
    cat <<ENDHTML
<html><head>
<meta http-equiv="refresh" content="0;url=$url"/>
</head></html>
ENDHTML
    wisprLoginResultsURL "$url"
}

domail() {
#    from=$1;to=$2;file=$3
#    (uamfile "$file" 0
#	echo
#	echo "-------------------------------------------------"
#	echo "Powered by Coova - http://www.coova.org/"
#	echo) | /usr/sbin/sendmail -t -f "$from" && return 0
#    return 1;
return 0
}

FORM_redirurl="${FORM_redirurl:-${GET_redirurl}}"
FORM_redirurl="${FORM_redirurl:-${POST_redirurl}}"

FORM_username="${FORM_username:-$FORM_UserName}"
FORM_username="${FORM_username:-$FORM_Username}"
FORM_password="${FORM_password:-$FORM_Password}"

# For WISPr 2.0 EAP, bounce back to chilli
[ "$FORM_res" = "wispr" ] && \
    [ "$FORM_WISPrEAPMsg" != "" ] && \
    [ "$FORM_WISPrVersion" = "2.0" ] && {
    http_redirect2 "http://$HS_UAMLISTEN:$HS_UAMPORT/login?username=$FORM_username&WISPrEAPMsg=$FORM_WISPrEAPMsg&WISPrVersion=2.0"
}

if [ "$FORM_uamip" != "" ] && [ "$HS_UAMSECRET" != "" ]
then
    if [ "$FORM_res" != "wispr" ]
    then
	QS=$(echo $QUERY_STRING | sed 's/&md=[^&=]*$//')
	HTTP="http"
	[ "$HTTPS" = "on" ] && HTTP="https"
	URL="$HTTP://$SERVER_NAME/$REQUEST_URI?$QS"
	CHECK="$URL$HS_UAMSECRET"
	CHECK_MD5=$(echo -n "$CHECK" |md5sum|cut -d' ' -f1|tr 'a-z' 'A-Z');
	if [ "$CHECK_MD5" = "$FORM_md" ]; then
	    COOVA_USERURL=$FORM_userurl
	else
	    http_redirect "/www/error.chi"
	fi
    fi
fi

