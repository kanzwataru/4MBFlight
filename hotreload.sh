#!/bin/bash
orig_date=$(stat -c %y build/devgame_m.so)
make || exit 2
new_date=$(stat -c %y build/devgame_m.so)

if [[ $orig_date == $new_date ]];
then
    exit 1
else
    exit 0
fi
