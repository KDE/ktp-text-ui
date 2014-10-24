project (imagesharer)

add_library (ktpimagesharer simplestimagehostingsharer.cpp
        abstractsharer.cpp
        mpform.cpp
        imgursharer.cpp
        imagebinsharer.cpp
        shareprovider.cpp
)

target_link_libraries(ktpimagesharer
                      KF5::KDELibs4Support
                      KF5::KIOWidgets
                      KTp::OTR
)

install(TARGETS ktpimagesharer ${INSTALL_TARGETS_DEFAULT_ARGS})
