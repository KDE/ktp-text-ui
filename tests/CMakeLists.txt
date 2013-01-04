

kde4_add_unit_test(ktpchat_tests
    TESTNAME message-processing-tests
    sync-processor.cpp
    message-processor-basic-tests.cpp
)

target_link_libraries(ktpchat_tests
    ktpchat
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${TELEPATHY_QT4_LIBRARIES}
    ${KTP_LIBRARIES}
)

add_definitions (-DQT_GUI_LIB)