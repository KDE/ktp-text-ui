project (imagesharer)

set(image_sharer_SRCS  imagebinsharer.cpp
	simplestimagehostingsharer.cpp
	abstractsharer.cpp
	mpform.cpp
	imgursharer.cpp
        shareprovider.cpp
        )

find_package(QJSON REQUIRED)

include_directories(${QJSON_INCLUDE_DIR})

kde4_add_library (ktpimagesharer SHARED ${image_sharer_SRCS})

target_link_libraries(ktpimagesharer
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${QJSON_LIBRARIES}
)

install(TARGETS ktpimagesharer ${INSTALL_TARGETS_DEFAULT_ARGS})