# Build only the MDR protocol and Linux RFCOMM backend. The upstream top-level
# project also builds an SDL client and fetches dependencies, neither of which
# belongs in this native Qt application or an offline-friendly distro package.
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED IMPORTED_TARGET dbus-1)
find_package(fmt REQUIRED)
find_library(BLUETOOTH_LIBRARY NAMES bluetooth REQUIRED)

set(MDR_VENDOR ${CMAKE_SOURCE_DIR}/vendor/SonyHeadphonesClient)
add_library(mdr-c INTERFACE)
target_include_directories(mdr-c INTERFACE ${MDR_VENDOR}/libmdr/include)

add_library(mdr_Includes INTERFACE)
target_link_libraries(mdr_Includes INTERFACE mdr-c fmt::fmt)
target_include_directories(mdr_Includes INTERFACE ${MDR_VENDOR}/libmdr/include)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mdr_Includes INTERFACE "SHELL:-include cstring")
endif()

file(GLOB MDR_SOURCES CONFIGURE_DEPENDS ${MDR_VENDOR}/libmdr/src/*.cpp ${MDR_VENDOR}/libmdr/src/Generated/*.cpp)
add_library(mdr STATIC ${MDR_SOURCES})
target_link_libraries(mdr PUBLIC mdr_Includes)
target_include_directories(mdr PUBLIC ${MDR_VENDOR}/libmdr/src)
target_include_directories(mdr BEFORE PRIVATE ${CMAKE_SOURCE_DIR}/cmake/fmt-compat)

add_library(mdr-bt STATIC
    ${MDR_VENDOR}/libmdr-bt/src/Linux/ConnectionLinux.cpp
    ${MDR_VENDOR}/libmdr-bt/src/Linux/DBusHelper.cpp)
target_include_directories(mdr-bt PUBLIC ${MDR_VENDOR}/libmdr-bt/include ${MDR_VENDOR}/libmdr/include)
target_include_directories(mdr-bt PRIVATE ${MDR_VENDOR}/libmdr-bt/src ${DBUS_INCLUDE_DIRS})
target_link_libraries(mdr-bt PUBLIC mdr PRIVATE PkgConfig::DBUS ${BLUETOOTH_LIBRARY})
