echo "enable DFS debug" > /dev/console
# senao channel-config debug
cfg80211tool wifi1 acs_dbgtrace 0x212
cfg80211tool ath1 qdf_cv_lvl 0x0027000a
radartool -i wifi1 dfsdebug 0x00200000
cfg80211tool ath1 dbgLVL 0x40c00000
