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
            Qt5::WebEngine
            KF5::Emoticons
            Qt5::Xml
            KF5::KIOWidgets
            ktpchat)


install(TARGETS ktp-adiumxtra-protocol-handler DESTINATION ${KDE_INSTALL_LIBEXECDIR})

configure_file(adiumxtra.protocol.in  ${CMAKE_CURRENT_BINARY_DIR}/adiumxtra.protocol)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/adiumxtra.protocol DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
