#!/bin/bash

if [ $# -eq 0 ]
  then
    echo "Please provide some arguments"
else
    for argu in "$@"
    do
        sed "s/^/ .\/jobCommander issueJob /" "$argu" > "temp.txt"
        source "temp.txt"
    done
fi

rm temp.txt