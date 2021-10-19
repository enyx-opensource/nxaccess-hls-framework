cmake_minimum_required(VERSION 2.8)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Configure project.
project(enyx-oe-hwstrat-hls-engine C CXX)

set(CPACK_PACKAGE_NAME "enyx-oe-hwstrat-hls-engine")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Enyx Order Entry HLS Hardware Strategy Control-Engine")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_CONTACT "Enyx Support <support@enyx.com>")
set(CPACK_PACKAGE_VENDOR "Enyx S.A.")
#TODO: use GetVersionFromChangeLogFile.cmake
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_BUILD_TYPE}")
set(CPACK_COMPONENTS_ALL base devel)

set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_RPM_PACKAGE_RELOCATABLE ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_PACKAGE_AUTOREQ OFF)

include(GNUInstallDirs)

if (CMAKE_INSTALL_PREFIX MATCHES "/usr/?$")
    set(CMAKE_INSTALL_SYSCONFDIR "/etc" CACHE PATH "read-only single machine data (etc)")
endif ()


# Check dependencies.
find_package(Threads REQUIRED QUIET)
find_package(Rt QUIET)
find_package(PkgConfig REQUIRED)
find_package(enyx-hfp 2.0.0 REQUIRED)
find_package(enyx-hw 5.5.0 REQUIRED)

# you need to have both libhfp and libenyxutils installed on your system
pkg_check_modules(EnyxLibs REQUIRED
    enyxutils>=7.2.7)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
add_definitions(-Wall -Wextra -Werror -Wshadow)
add_definitions(-DENYX_NO_HLS_SUPPORT)

# Add headers directory to include path.
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/include)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/lib)
include_directories(${EnyxLibs_INCLUDE_DIRS})

# Add libraries directory to the link path.
link_directories(${EnyxLibs_LIBRARY_DIRS})

# Build everything in subdirs.
add_subdirectory(lib)
add_subdirectory(python-binding)
add_subdirectory(demo)
add_subdirectory(tools)

# Fake check directory
add_custom_target(check COMMAND /bin/true)

include(CPack)
