#!/bin/sh

config="/var/etc/msmtprc"

clean_config()
{
	rm -f ${config}
}

generate_config()
{
	mkdir -p "$(dirname "${config}")" && {
		host="$(uci get emailalert.email.smtpaddr)"
		port="$(uci get emailalert.email.smtpport)"
		from="$(uci get emailalert.email.from)"
		auth="$(uci get emailalert.email.auth)"
#		if [ "${auth}" == "0" ]; then
#			auth="off"
#		else
#			auth="login"
#		fi
		user="$(uci get emailalert.email.user)"
		password="$(uci get emailalert.email.password)"
		security="$(uci get emailalert.email.security)"
		if [ -n "${user}" -a -n "${password}" ]; then
			auth="login"
		else
			auth="off"
		fi
		if [ "${security}" == "0" ]; then
			tls="off"
		else
			tls="on"
		fi
		if [ "${security}" != "2" ]; then
			starttls="off"
		else
			starttls="on"
		fi
		if [ "${host}" -a -n "${port}" -a -n "${from}" -a -n "${auth}" -a -n "${tls}" -a -n "${starttls}" ]; then
			(
cat <<EOF
host ${host}
port ${port}
timeout 15
from ${from}
auth ${auth}
user ${user}
password ${password}
tls ${tls}
tls_starttls ${starttls}
tls_certcheck off
EOF
			) > "${config}"
		fi
	}
}

send()
# $1: Subject
# $2: Message
{
	[ ! -f "${config}" ] && {
		generate_config
	}
	if [ -f "${config}" ]; then
		recipients="$(uci get emailalert.email.alert_to | sed -e "s/; */ /g")"
		if [ -n "${recipients}" ]; then
			cat <<EOF | msmtp --file=${config} ${recipients} &>/dev/null &
Subject: ${1}

${2}
--
$(date)
EOF
		fi
	fi
}

send_alert()
# $1: EventCode
# $2: AdditionalMessage
{
	if [ "$(uci get emailalert.email.enable)" == 1 ]; then
		rm -f ${config}
		if [ "$1" = "reboot" ]; then
			subject="[Email-Alert]"
			subject="$subject""[$(uci get sysProductInfo.model.modelName)]"
			subject="$subject""[$(ifconfig eth0|grep HWaddr|awk -F" " '{print $5}')]"
			subject="$subject""Reboot"
			message="$(uci get emailalert.email.alert_event_${1}_message | sed -e "s/\(\\\\[rn]\)\+/\n/g")$(printf "\n--\n" && echo "Reboot")"
		else
			subject="$(uci get emailalert.email.subject)"
			message="$(uci get emailalert.email.alert_event_${1}_message | sed -e "s/\(\\\\[rn]\)\+/\n/g")$([ -n "${2}" ] && printf "\n--\n" && echo "${2}")"
		fi
		send "${subject}" "${message}"
	fi
}

send_config_changes()
# auto fill in
{
	if [ "$(uci get emailalert.email.enable)" == 1 ]; then
		subject="$(uci get emailalert.email.subject)"
		changes="$(cat /tmp/config.changes|sort|uniq|grep -v -E "csrf|syskey")"
		message="$(echo [Configuration Changed])$changes"
		if [ ! -z "$changes" ]; then
			send "${subject}" "${message}"
			rm -f /tmp/config.changes
		fi
	fi
}

case "${1}" in
	"clean_config" | "generate_config" | "send" | "send_alert" | "send_config_changes")
		cmd="${1}"
		shift
		${cmd} "$@"
		;;
esac
