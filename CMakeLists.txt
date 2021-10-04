project(ktp-text-ui)

cmake_minimum_required(VERSION 2.8.12)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "21")
set(RELEASE_SERVICE_VERSION_MINOR "08")
set(RELEASE_SERVICE_VERSION_MICRO "2")
set(KTP_TEXT_UI_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
set(KTP_MESSAGE_FILTER_FRAMEWORK_VERSION "5")
set(IS_KTP_INTERNAL_MODULE TRUE)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package (KF5 REQUIRED COMPONENTS Archive Sonnet WidgetsAddons Service Emoticons
                                      KIO KCMUtils NotifyConfig Notifications I18n
                                      WindowSystem XmlGui ItemViews TextWidgets
                                      IconThemes DBusAddons)
set(QT_REQUIRED_VERSION 5.7.0)
find_package(Qt5 ${QT_REQUIRED_VERSION} REQUIRED COMPONENTS WebEngine WebEngineWidgets)
find_package(Qt5 ${QT_REQUIRED_VERSION} OPTIONAL_COMPONENTS TextToSpeech)
if (NOT Qt5TextToSpeech_FOUND)
    message(STATUS "Qt5TextToSpeech not found, speech features will be disabled")
endif()
find_package (KTp REQUIRED)

find_package (KF5People)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(FeatureSummary)

set_package_properties(KF5People PROPERTIES DESCRIPTION "KDE Contacts aggregation"
                       TYPE OPTIONAL
)

if (KF5People_FOUND)
    add_definitions(-DHAVE_KPEOPLE)
endif ()
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

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
