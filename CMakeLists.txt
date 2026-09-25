cmake_minimum_required(VERSION 3.22)
project(SonyHeadphonesLinux VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 DBus)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(sony-headphones-linux
    src/main.cpp
    src/devicecontroller.cpp
    src/devicecontroller.h
    src/bluezdiscovery.cpp
    src/bluezdiscovery.h
)

qt_add_qml_module(sony-headphones-linux
    URI Sony.Headphones
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/StatusPill.qml
        qml/components/NoiseModeButton.qml
    RESOURCES resources/sony-headphones-linux.svg
)

target_link_libraries(sony-headphones-linux PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::DBus
)

include(GNUInstallDirs)
install(TARGETS sony-headphones-linux RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/io.github.chrisjahner.SonyHeadphonesLinux.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES packaging/io.github.chrisjahner.SonyHeadphonesLinux.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
