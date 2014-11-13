add_subdirectory(bugzilla)
add_subdirectory(emoticons)
add_subdirectory(formatting)
add_subdirectory(images)
add_subdirectory(latex)
add_subdirectory(searchexpansion)
add_subdirectory(youtube)
add_subdirectory(highlight)
add_subdirectory(urlexpansion)
add_subdirectory(otr)

# TODO: is that a framework now?
# IF (EXISTS "${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml")
#     set (HAS_KTTS TRUE)
#     add_subdirectory(texttospeech)
# ENDIF ()
# macro_log_feature(HAS_KTTS "KTTSD" "KDE Text to Speech Deamon" "" FALSE "" "Needed for optional tts message plugin")


