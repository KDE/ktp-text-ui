include_directories(${CMAKE_SOURCE_DIR}/lib)

set(ktp-log-viewer_SRCS
        main.cpp
        log-viewer.cpp
        logs-import-dialog.cpp
        dates-model.cpp
        dates-view-delegate.cpp
        entity-model.cpp
        message-view.cpp
        entity-view.cpp
        entity-view-delegate.cpp
        entity-filter-model.cpp
        person-entity-merge-model.cpp
)

add_subdirectory(config)

ki18n_wrap_ui(ktp-log-viewer_SRCS log-viewer.ui)

add_executable(ktp-log-viewer ${ktp-log-viewer_SRCS})

target_link_libraries(ktp-log-viewer
            KF5::I18n
            KF5::ConfigCore
            KF5::ItemViews
            KF5::CoreAddons
            KF5::KIOWidgets
            KF5::Emoticons
            KF5::KCMUtils
            KF5::WebKit
            KTp::CommonInternals
            KTp::Logger
            KTp::Models
            ktpchat)

install(TARGETS ktp-log-viewer DESTINATION ${BIN_INSTALL_DIR})
install(FILES ktp-log-viewer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES log-viewer.rc DESTINATION  ${DATA_INSTALL_DIR}/ktp-log-viewer/)
