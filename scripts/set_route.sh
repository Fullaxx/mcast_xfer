#!/bin/bash

if [ `id -u` != "0" ]; then
  echo "Got Root?"
  exit 1
fi

if [ -z "$1" ]; then
  echo "$0: <DEV>"
  exit 2
fi

DEV="$1"

if ! cat /proc/net/dev | awk '{print $1}' | grep -q ${DEV}; then
  echo "${DEV} not found in /proc/net/dev"
  exit 3
fi

echo "Adding route to 224.0.0.0/4 from ${DEV}"
# route add -net 224.0.0.0 netmask 240.0.0.0 ${DEV}
ip route add 224.0.0.0/4 dev ${DEV}
