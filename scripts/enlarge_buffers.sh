#!/bin/bash

if [ `id -u` != "0" ]; then
  echo "Got Root?"
  exit 1
fi

# 100MB
echo "100000000" >/proc/sys/net/core/rmem_max
echo "100000000" >/proc/sys/net/core/wmem_max
