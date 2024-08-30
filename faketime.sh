#!/bin/bash

targetTime="$1"

if [[ ${targetTime} == "" ]]; then
    echo "need a absolute time. eg: 2024-08-30 18:00:00"
    exit
fi

current=$(date +%s)
target=$(date -d "$targetTime" +%s)

let time=(${target} - ${current})

if [[ ${time} -gt 0 ]]; then
    time="+${time}"
fi

echo "${time}"
echo "${time}" >${HOME}/.timeoffsetrc

# usage:
# ./faketime.sh "2024-08-30 18:00:00"
# LD_PRELOAD=./libfaketime.so.1 date
