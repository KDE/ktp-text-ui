#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui" -name "*.rc"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp"` -o $podir/ktp-text-ui.pot
#rm -f rc.cpp
