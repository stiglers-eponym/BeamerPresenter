#!/bin/bash
echo $(( 16#$(wmctrl -lp | sed -n "s/^0x\([0-9a-f]\+\) \+[0-9]\+ \+$1 .*$/\1/p") ))
