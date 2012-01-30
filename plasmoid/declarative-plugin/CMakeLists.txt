# project(ktpchatqmlplugin)

set(ktpchatqmlplugin_SRCS
        qml-plugins.cpp
        messages-model.cpp
        telepathy-text-observer.cpp
        conversation.cpp
        conversation-target.cpp
        conversations-model.cpp
        conversation-que-manager.cpp
    )

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

# qt4_automoc(${ktpchatqmlplugin_SRCS})


kde4_add_library(ktpchatqmlplugin SHARED ${ktpchatqmlplugin_SRCS})

target_link_libraries(ktpchatqmlplugin
        ${KDE4_KDECORE_LIBS}
        ${QT_QTSCRIPT_LIBRARY}
        ${QT_QTDECLARATIVE_LIBRARY}
        ${TELEPATHY_QT4_LIBRARIES}
        ${KTP_LIBRARIES}
        ${KDE4_KDEUI_LIBS}
        ktpchat
)

install(TARGETS ktpchatqmlplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/telepathy/chat)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/telepathy/chat)