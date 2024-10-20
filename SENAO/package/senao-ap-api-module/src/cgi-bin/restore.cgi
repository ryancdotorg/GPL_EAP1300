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

# rm exist /tmp/restore.gz
[ -e /tmp/restore.gz ] && rm /tmp/restore.gz

if [ "$REQUEST_METHOD" = "POST" ]; then
    if [ "$CONTENT_LENGTH" -gt 0 ]; then
        read -n $CONTENT_LENGTH POST_DATA <&0
    fi
fi

cat $QUERY_STRING > /tmp/query

query_len=$(cat /tmp/query | wc -l)

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
cat /tmp/query | head -n $((query_len-1)) | tail -n $((query_len-4)) > /tmp/tmp_restore.gz
rm /tmp/query

tmp_restore_len=$(cat /tmp/tmp_restore.gz | wc -l)

# remove end of file 0x0a
head -n $((tmp_restore_len-1)) /tmp/tmp_restore.gz > /tmp/restore.gz
tail -n 1 /tmp/tmp_restore.gz | tr -d '\n' >> /tmp/restore.gz
rm /tmp/tmp_restore.gz

# remove redundant ^M
sed -i '$s/\r$//' /tmp/restore.gz

openapi_response $status_code "$url"