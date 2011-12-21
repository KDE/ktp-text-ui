#! /usr/bin/env bash
#$EXTRACTRC `find . -name "*.ui"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp"` -o $podir/ktp-adiumxtra-protocol-handler.pot
#rm -f rc.cpp
