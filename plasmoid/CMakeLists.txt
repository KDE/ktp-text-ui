add_subdirectory(declarative-plugin)
install(DIRECTORY org.kde.ktp-chatplasmoid
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/)


install(FILES org.kde.ktp-chatplasmoid/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-ktpchat.desktop)
