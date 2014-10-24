include_directories(${CMAKE_SOURCE_DIR}/lib)

set(ktp-adiumxtra-protocol-handler_SRCS
        bundle-installer.cpp
        chat-style-installer.cpp
        emoticon-set-installer.cpp
        adiumxtra-protocol-handler.cpp
        main.cpp
)

add_executable(ktp-adiumxtra-protocol-handler ${ktp-adiumxtra-protocol-handler_SRCS})

target_link_libraries(ktp-adiumxtra-protocol-handler
            
            ${KDE4_KDEUI_LIBS}
            ${QT_QTWEBKIT_LIBRARY}
            KF5::Emoticons
            Qt5::Xml
            KF5::KIOWidgets
            ktpchat)


install(TARGETS ktp-adiumxtra-protocol-handler DESTINATION ${LIBEXEC_INSTALL_DIR})

configure_file(adiumxtra.protocol.in  ${CMAKE_CURRENT_BINARY_DIR}/adiumxtra.protocol)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/adiumxtra.protocol DESTINATION ${SERVICES_INSTALL_DIR})
