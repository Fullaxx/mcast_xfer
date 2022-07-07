#!/bin/bash

guess_sldir()
{
  if [ -f /usr/lib64/libpthread.a ]; then
    SLDIR="/usr/lib64"
  elif [ -f /usr/lib/x86_64-linux-gnu/libpthread.a ]; then
    SLDIR="/usr/lib/x86_64-linux-gnu"
  fi
}

find_sldir()
{
  echo `find /usr -type f -name libpthread.a | head -n1 | xargs dirname`
}

set -e

BAKAPI="../bak_api"
OPT="-O2"
DBG="-ggdb3 -DDEBUG"
CFLAGS="-Wall -I${BAKAPI}"
OPTCFLAGS="${CFLAGS} ${OPT}"
DBGCFLAGS="${CFLAGS} ${DBG}"
SLDIR=${SLDIR:-`find_sldir`}

rm -f *.exe *.dbg

gcc ${OPTCFLAGS} mcastsend.c ${BAKAPI}/{async_udp4,cryptohash,futils}.c -lpthread -lcrypto -o mcastsend.exe
gcc ${DBGCFLAGS} mcastsend.c ${BAKAPI}/{async_udp4,cryptohash,futils}.c -lpthread -lcrypto -o mcastsend.dbg

gcc ${OPTCFLAGS} mcastrecv.c ${BAKAPI}/{async_udp4,cryptohash,futils}.c -lpthread -lcrypto -o mcastrecv.exe
gcc ${DBGCFLAGS} mcastrecv.c ${BAKAPI}/{async_udp4,cryptohash,futils}.c -lpthread -lcrypto -o mcastrecv.dbg

if [ -f ${SLDIR}/libpthread.a ] && [ -f ${SLDIR}/libcrypto.a ]; then
  gcc -static ${OPTCFLAGS} mcastsend.c ${BAKAPI}/{async_udp4,cryptohash,futils}.c ${SLDIR}/libpthread.a ${SLDIR}/libcrypto.a -o mcastsend.static.exe
  gcc -static ${OPTCFLAGS} mcastrecv.c ${BAKAPI}/{async_udp4,cryptohash,futils}.c ${SLDIR}/libpthread.a ${SLDIR}/libcrypto.a -o mcastrecv.static.exe
fi

strip *.exe
