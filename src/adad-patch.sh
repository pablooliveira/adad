#!/bin/bash

BINARY=$1
SYMBOL=$2
if [[ -z "$SYMBOL" || -z "$BINARY" ]]; then
  echo "Usage: $0 <binary> <symbol>"
  exit 1
fi

# If SIZE is empty then the symbol is not found and we stop
SIZE=$(readelf -s $BINARY |grep " $SYMBOL\$"|cut -d ' ' -f6)
if [ -z "$SIZE" ]; then
  echo "Error: Symbol '$SYMBOL' not found in the binary."
  exit 1
fi

# First ensure we have permissions to access perf system wide events
PERF_EVENT_PARANOID=$(cat /proc/sys/kernel/perf_event_paranoid)
if [ $PERF_EVENT_PARANOID -ne -1 ]; then
  echo "Error: kernel.perf_event_paranoid is not set to -1. Please, run the following command:"
  echo "sudo sysctl -w kernel.perf_event_paranoid=-1"
  exit 1
fi

# Patch the binary
./e9tool -CFR -E .text..$SYMBOL -E $SYMBOL+$SIZE...text.end \
  -M "F.name=\"$SYMBOL\"" -M 'F.entry=1' -P 'hook_entry()@adad-rt' \
  -M "F.name=\"$SYMBOL\"" -M 'asm=/ret/'  -P 'hook_exit()@adad-rt' \
  $BINARY

echo -e "\e[32mPatched $BINARY with $SYMBOL successfully. You can now run a.out\e[0m"
