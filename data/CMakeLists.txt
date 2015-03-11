install(FILES Template.html
        DESTINATION ${DATA_INSTALL_DIR}/ktelepathy)

install(DIRECTORY styles
        DESTINATION ${DATA_INSTALL_DIR}/ktelepathy
        PATTERN "LICENSE" EXCLUDE
        PATTERN "README" EXCLUDE
        PATTERN "Renkoo LICENSE.txt" EXCLUDE)
