#! /usr/bin/env bash
$EXTRACTRC appearance-config.ui >> rc.cpp || exit 11
$XGETTEXT appearance-config.cpp rc.cpp -o $podir/kcm_ktp_chat_appearance.pot
rm -f rc.cpp

$EXTRACTRC behavior-config.ui >> rc.cpp || exit 11
$XGETTEXT behavior-config.cpp rc.cpp -o $podir/kcm_ktp_chat_behavior.pot
rm -f rc.cpp
