#!/bin/bash
# show.sh <target> <func> : print a function's asm from the current split

awk -v f="$2" '$0 ~ "^glabel "f"$" {p=1} p && NF {print} $0 ~ "^endlabel "f"$" {p=0}' asm/overlays/$1/*.s | cut -c1-110 | sed 's|/\* [0-9A-F]* \([0-9A-F]*\) [0-9A-F]* \*/|\1|'
