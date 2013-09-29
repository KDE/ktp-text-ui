INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib ${KDE4_INCLUDES} . )

set(textuipart_PART_SRCS KTpTextChatPart.cpp chatTabWidget.cpp)

kde4_add_plugin(KTpTextChatPart ${textuipart_PART_SRCS})

target_link_libraries(KTpTextChatPart ${KDE4_KPARTS_LIBS} ktpchat)

install(TARGETS KTpTextChatPart  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES KTpTextChatPart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES chatTabWidget.rc
        DESTINATION  ${DATA_INSTALL_DIR}/ktp-text-ui/)