#!/bin/bash

outputFile=$1
recordmydesktop --no-sound -o "$outputFile" --windowid `xwininfo -name QEMU | grep 'id: 0x' | grep -Eo '0x[a-z0-9]+'`
