add_subdirectory(emoticons)
#Commented out until https://bugs.kde.org/show_bug.cgi?id=302534 AND https://bugs.kde.org/show_bug.cgi?id=302862
# add_subdirectory(formatting)
add_subdirectory(images)
#Commented out until https://bugs.kde.org/show_bug.cgi?id=304740
# add_subdirectory(searchexpansion)


IF (EXISTS "${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml")
    set (HAS_KTTS TRUE)
ENDIF (EXISTS "${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml")

if(HAS_KTTS)
    add_subdirectory(texttospeech)
endif(HAS_KTTS)

macro_log_feature(HAS_KTTS "KTTSD" "KDE Text to Speech Deamon" "" FALSE "" "Needed for optional tts message plugin")
add_subdirectory(latex)
add_subdirectory(youtube)
add_subdirectory(substitution)
add_subdirectory(highlight)
