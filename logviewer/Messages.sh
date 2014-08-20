#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc"` `find . -name "*.ui"`>> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp"` -o $podir/ktp-log-viewer.pot
#rm -f rc.cpp
