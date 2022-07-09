#!/bin/bash

convert_2()
{
  printf "\\$(printf %03o "$1")"
  printf "\\$(printf %03o "$2")"
}

print_missing_2()
{
  echo -n "Missing Pieces:"
  for ((i=97;i<=122;i++)); do
    for ((j=97;j<=122;j++)); do
      EXT=`convert_2 $i $j`
      if [ ! -f "mpartfile.${EXT}" ]; then echo -n " ${EXT}"; fi
    done
  done
  echo
}

convert_3()
{
  printf "\\$(printf %03o "$1")"
  printf "\\$(printf %03o "$2")"
  printf "\\$(printf %03o "$3")"
}

print_missing_3()
{
  echo -n "Missing Pieces:"
  for ((i=97;i<=122;i++)); do
    for ((j=97;j<=122;j++)); do
      for ((k=97;k<=122;k++)); do
        EXT=`convert_3 $i $j $k`
        if [ ! -f "mpartfile.${EXT}" ]; then echo -n " ${EXT}"; fi
      done
    done
  done
  echo
}

DONE="0"
COUNT="0"
while [ "${DONE}" == "0" ]; do
  COUNT=$(( COUNT + 1))
  if [ "${COUNT}" -gt "675" ];   then print_missing_2; fi
#  if [ "${COUNT}" -gt "17575" ]; then print_missing_3; fi
  mcastrecv.exe
  if [ "$?" == "99" ]; then DONE="1"; fi
done

#echo
#print_missing
