#! /usr/bin/env bash
#$EXTRACTRC `find . -name "*.ui"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp"` -o $podir/telepathy-kde-text-ui.pot
#rm -f rc.cpp
