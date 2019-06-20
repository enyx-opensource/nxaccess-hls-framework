#!/bin/sh

changelog=$(dirname $0)/CHANGELOG.md

re='^## \[\([0-9\.]\+\)\] - \([0-9-]\+\)$'

while getopts 'hvd' opt; do
    case "$opt" in
        h)
            echo "Usage: $0 [-hvd]"
            echo
            echo "Arguments:"
            echo "    -h    Show this help."
            echo "    -v    Show latest version."
            echo "    -d    Show latest date."
            ;;
        v)
            sed -n "s/$re/\1/p" $changelog | head -n1
            ;;
        d)
            sed -n "s/$re/\2/p" $changelog | head -n1
            ;;
    esac
done
