#!/bin/sh

sleep 5
## 2g level0:110 ##
thermaltool -i wifi0 -e 1 -set -hi0 110
thermaltool -i wifi0 -e 1 -set -hi1 114 -lo1 105 -off1 20
thermaltool -i wifi0 -e 1 -set -hi2 116 -lo2 107 -off2 50
thermaltool -i wifi0 -e 1 -set -hi3 119 -lo3 109 -off3 90

## 5g level0:110 ##
thermaltool -i wifi1 -e 1 -set -hi0 108
thermaltool -i wifi1 -e 1 -set -hi1 112 -lo1 102 -off1 20
thermaltool -i wifi1 -e 1 -set -hi2 115 -lo2 103 -off2 50
thermaltool -i wifi1 -e 1 -set -hi3 119 -lo3 106 -off3 90
