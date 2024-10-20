#!/bin/sh

# This shell includes all senao functions in /lib/senao-shell-libs/ .

for file in $(ls /lib/senao-shell-libs/*.sh 2>/dev/null); do
    . $file
done
