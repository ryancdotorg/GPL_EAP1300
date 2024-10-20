
		local config=$1
		local section=$2
		local option=$3

        defaultValue=`uci get -c /rom/etc/config $config.$section.$option`
        echo $defaultValue | tr -d '\n\r'  
