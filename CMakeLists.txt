project(ktp-text-ui)

cmake_minimum_required(VERSION 2.8.12)

set(KTP_TEXT_UI_VERSION "15.04.1")
set(KTP_MESSAGE_FILTER_FRAMEWORK_VERSION "5")
set(IS_KTP_INTERNAL_MODULE TRUE)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package (KF5 REQUIRED COMPONENTS Archive Sonnet WidgetsAddons Service Emoticons KIO KCMUtils NotifyConfig Notifications I18n WebKit WindowSystem XmlGui ItemViews TextWidgets IconThemes)
find_package (Qt5 REQUIRED COMPONENTS WebKitWidgets)
find_package (KTp REQUIRED)

find_package (KF5People)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMSetupVersion)
include(FeatureSummary)

set_package_properties(KF5People PROPERTIES DESCRIPTION "KDE Contacts aggregation"
                       TYPE OPTIONAL
)

if (KF5People_FOUND)
    add_definitions(-DHAVE_KPEOPLE)
endif ()

add_subdirectory(image-sharer)
add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(config)
add_subdirectory(data)
add_subdirectory(adiumxtra-protocol-handler)
add_subdirectory(filters)
add_subdirectory(logviewer)

ecm_setup_version(${KTP_TEXT_UI_VERSION} VARIABLE_PREFIX KTP_TEXT_UI
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ktptextui_version.h")

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
