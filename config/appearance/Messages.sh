#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.cpp` -o $podir/kcm_ktp_chat_appearance.pot
rm -f rc.cpp
