. /lib/config/uci.sh

set_portal_config() {
set +x
    uci delete portal."$2"
    uci set portal."$2"=portal
    radiusServer=$(uci get portal."$1".radiusServer)
    [ -n "$radiusServer" ] && uci set portal."$2".radiusServer="$radiusServer"
    radiusServer2=$(uci get portal."$1".radiusServer2)
    [ -n "$radiusServer2" ] && uci set portal."$2".radiusServer2="$radiusServer2"
    radiusPort=$(uci get portal."$1".radiusPort)
    [ -n "$radiusPort" ] && uci set portal."$2".radiusPort="$radiusPort"
    accountingServer=$(uci get portal."$1".accountingServer)
    [ -n "$accountingServer" ] && uci set portal."$2".accountingServer="$accountingServer"
    accountingEnable=$(uci get portal."$1".accountingEnable)
    [ -n "$accountingEnable" ] && uci set portal."$2".accountingEnable="$accountingEnable"
    accountingPort=$(uci get portal."$1".accountingPort)
    [ -n "$accountingPort" ] && uci set portal."$2".accountingPort="$accountingPort"
    idleTimeout=$(uci get portal."$1".idleTimeout)
    [ -n "$idleTimeout" ] && uci set portal."$2".idleTimeout="$idleTimeout"
    idleTimeoutEnable=$(uci get portal."$1".idleTimeoutEnable)
    [ -n "$idleTimeoutEnable" ] && uci set portal."$2".idleTimeoutEnable="$idleTimeoutEnable"
    uamformat=$(uci get portal."$1".uamformat)
    [ -n "$uamformat" ] && uci set portal."$2".uamformat="$uamformat"
    localAuth=$(uci get portal."$1".localAuth)
    [ -n "$localAuth" ] && uci set portal."$2".localAuth="$localAuth"
    port=$(uci get portal."$1".port)
    [ -n "$port" ] && uci set portal."$2".port="$port"
    httpsEnable=$(uci get portal."$1".httpsEnable)
    [ -n "$httpsEnable" ] && uci set portal."$2".httpsEnable="$httpsEnable"
    accountingInterval=$(uci get portal."$1".accountingInterval)
    [ -n "$accountingInterval" ] && uci set portal."$2".accountingInterval="$accountingInterval"
    portalEnable=$(uci get portal."$1".enable)
    [ -n "$portalEnable" ] && uci set portal."$2".enable="$portalEnable"
    userurl=$(uci get portal."$1".userurl)
    [ -n "$userurl" ] && uci set portal."$2".userurl="$userurl"
    walledGardenPages=$(uci get portal."$1".walledGardenPages)
    [ -n "$walledGardenPages" ] && uci set portal."$2".walledGardenPages="$walledGardenPages"
    radiusSecret=$(uci get portal."$1".radiusSecret)
    [ -n "$radiusSecret" ] && uci set portal."$2".radiusSecret="$radiusSecret"
    externalServer=$(uci get portal."$1".externalServer)
    [ -n "$externalServer" ] && uci set portal."$2".externalServer="$externalServer"
    externalSecret=$(uci get portal."$1".externalSecret)
    [ -n "$externalSecret" ] && uci set portal."$2".externalSecret="$externalSecret"
    accountingSecret=$(uci get portal."$1".accountingSecret)
    [ -n "$accountingSecret" ] && uci set portal."$2".accountingSecret="$accountingSecret"
    radiusSecret2=$(uci get portal."$1".radiusSecret2)
    [ -n "$radiusSecret2" ] && uci set portal."$2".radiusSecret2="$radiusSecret2"
    sessionTimeout=$(uci get portal."$1".sessionTimeout)
    [ -n "$sessionTimeout" ] && uci set portal."$2".sessionTimeout="$sessionTimeout"
    sessionTimeoutEnable=$(uci get portal."$1".sessionTimeoutEnable)
    [ -n "$sessionTimeoutEnable" ] && uci set portal."$2".sessionTimeoutEnable="$sessionTimeoutEnable"
    loginType=$(uci get portal."$1".loginType)
    [ -n "$loginType" ] && uci set portal."$2".loginType="$loginType"
    walledGardenEnable=$(uci get portal."$1".walledGardenEnable)
    [ -n "$walledGardenEnable" ] && uci set portal."$2".walledGardenEnable="$walledGardenEnable"
    authType=$(uci get portal."$1".authType)
    [ -n "$authType" ] && uci set portal."$2".authType="$authType"
set -x
}

set_portal_config $1 $2
