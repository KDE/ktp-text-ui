#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp"` -o $podir/ktp-log-viewer.pot
#rm -f rc.cpp
