#!/bin/bash

export MTU="3000"
export SLEEPDELAY="200"

SPLITDIR="msend"
PIECES="676"
#PIECES="17576"

mkdir ${SPLITDIR}
pushd ${SPLITDIR}

if [ "${PIECES}" == "17576" ]; then
  split -a 3 -n ${PIECES} ../test.bin mpartfile.

  for PART in mpartfile.???; do
    mcastsend.exe ${PART}
    usleep 10000
  done
elif [ "${PIECES}" == "676" ]; then
  split -a 2 -n ${PIECES} ../test.bin mpartfile.

  for PART in mpartfile.??; do
    mcastsend.exe ${PART}
    usleep 10000
  done
else
  echo "Unknown"
  exit 1
fi

echo "What extensions would you like to resend (Empty string to exit)?"
read ANS

while [ -n "${ANS}" ]; do
  for EXT in `echo ${ANS}`; do
    mcastsend.exe mpartfile.${EXT}
    usleep 1000000
  done

  echo "What would you like to resend?"
  read ANS
done

popd
echo -n "Removing ${SPLITDIR} ... "
rm -r ${SPLITDIR}
echo "Done"
