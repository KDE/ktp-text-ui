project (imagesharer)

add_library (ktpimagesharer simplestimagehostingsharer.cpp
        abstractsharer.cpp
        mpform.cpp
        imgursharer.cpp
        imagebinsharer.cpp
        shareprovider.cpp
)

target_link_libraries(ktpimagesharer
                      KF5::I18n
                      KF5::KIOWidgets
                      KTp::OTR
)

install(TARGETS ktpimagesharer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
