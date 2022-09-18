install(FILES Template.html
        DESTINATION ${KDE_INSTALL_DATADIR}/ktelepathy)

install(DIRECTORY styles
        DESTINATION ${KDE_INSTALL_DATADIR}/ktelepathy
        PATTERN "LICENSE" EXCLUDE
        PATTERN "README" EXCLUDE
        PATTERN "Renkoo LICENSE.txt" EXCLUDE)
