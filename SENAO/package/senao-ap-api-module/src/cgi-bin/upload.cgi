#!/bin/sh

url=""
status_code=""
reason_phrase=""
stok=""

jsonValue() {
        KEY=$1
        num=$2
        awk -F"[,:}]" '{for(i=1;i<=NF;i++){if($i~/'$KEY'\042/){print $(i+1)}}}' | tr -d '"' | sed -n ${num}p
}

auth_check ()
{
	url=$(curl -k -X GET "https://127.0.0.1:4430/api/auth/check" -H "accept: */*"  -H "Content-Type: application/json" -H "Authorization: Bearer $1")
	status_code=$(echo $url | jsonValue status_code)
	reason_phrase=$(echo $url | jsonValue reason_phrase)
}

openapi_response ()
{
	printf 'Status: %d \r\n' $1
	printf 'Access-Control-Allow-Origin: *\r\n'
	printf 'Access-Control-Allow-Methods: GET, POST, DELETE, PUT\r\n'
	printf 'Access-Control-Allow-Headers: Content-Type, api_key, Authorization\r\n'
	printf 'X-Rate-Limit: 5000\r\n'
	printf 'Content-type: application/json\r\n\r\n'
	printf '%s' $2
	printf '\n'
}

# rm exist firmware.img
[ -e /tmp/firmware.img ] && rm /tmp/firmware.img

if [ "$REQUEST_METHOD" = "POST" ]; then
	if [ "$CONTENT_LENGTH" -gt 0 ]; then
		read -n $CONTENT_LENGTH POST_DATA <&0
	fi
fi

cat $QUERY_STRING > /tmp/query

query_len=$(cat /tmp/query | wc -l)

from_webpage=$(tail -n 5 /tmp/query | grep hpath | wc -l)

echo from_webpage:$from_webpage > /dev/console

[ $from_webpage -gt 0 ] &&
{ # cgi from web_page
	echo "into from_webpage" > /dev/console

	stok=$(tail -n 2 /tmp/query | head -n 1)

	# remove query string extension info
	# remove 3 line from head
	len_h=$(head -n 3 /tmp/query | wc -c)
	dd if=/tmp/query of=/tmp/tmp_firmware.img bs=$len_h skip=1
	rm /tmp/query

	# remove 6 (3+6=9) line from tail
	head -n $((query_len-9)) /tmp/tmp_firmware.img > /tmp/firmware.img
	# remove end of file 0x0a. tail 6th
	tail -n 6 /tmp/tmp_firmware.img | head -n 1 | tr -d '\n' >> /tmp/firmware.img
	rm /tmp/tmp_firmware.img

	# remove redundant ^M
	sed -i '$s/\r$//' /tmp/firmware.img
} ||
{ # cgi from curl command
	echo "into from_curl_command" > /dev/console
	echo JWT_TOKEN:$JWT_TOKEN > /dev/console

	# check jwt token
	auth_check $JWT_TOKEN
	[ $status_code != 200 ] &&
	{
		openapi_response $status_code "$url";
		rm /tmp/query
		return false;
	}

	# remove query string extension info
	cat /tmp/query | head -n $((query_len-1)) | tail -n $((query_len-4)) > /tmp/tmp_firmware.img
	rm /tmp/query

	tmp_firmware_len=$(cat /tmp/tmp_firmware.img | wc -l)

	# remove end of file 0x0a
	head -n $((tmp_firmware_len-1)) /tmp/tmp_firmware.img > /tmp/firmware.img
	tail -n 1 /tmp/tmp_firmware.img | tr -d '\n' >> /tmp/firmware.img
	rm /tmp/tmp_firmware.img

	# remove redundant ^M
	sed -i '$s/\r$//' /tmp/firmware.img
}

fw_check=$(. /lib/functions.sh; include /lib/upgrade; senao_image_header_check /tmp/firmware.img 2>&1 && echo 1 || echo 0)

[ $fw_check -eq 0 ] && rm /tmp/firmware.img

[ $from_webpage -gt 0 ] &&
{
	printf '<html>\n'
	printf '<head>\n'
	printf '<meta http-equiv="REFRESH"\n'
	printf 'content="0;url=/lsp/fw_upgrade.html?stok=%s&valid=%d">\n' $stok $fw_check
	printf '</head>\n'
	printf '</html>\n'
} ||
{
	[ "$fw_check" = "1" ] &&
	{
		openapi_response $status_code "$url"
	} ||
	{
		openapi_response 400 "{\"status_code\":400,\"reason_phrase\":\"Invalid Firmware\",\"error_code\":3,\"error_message\":\"INVALID VALUE OF ARGUMENTS:firmware\",\"data\":null}"
	}
}
