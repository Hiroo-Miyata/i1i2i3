#!/bin/bash
if [ "$1" == "miyata" ] ; then
    IP="157.82.201.83"
elif [ "$1" == "shimono" ] ; then
    IP="157.82.202.64"
elif [ "$1" == "myself" ] ; then
    IP="127.0.0.1"
fi

gcc -g -o i3_fft i3.c fft.c -Wall -lm
./i3_fft $IP 50000