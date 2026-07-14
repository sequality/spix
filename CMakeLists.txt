cmake_minimum_required(VERSION 3.18)
project(Spix LANGUAGES CXX)

option(SPIX_BUILD_EXAMPLES "Build Spix examples." ON)
option(SPIX_BUILD_TESTS "Build Spix unit tests." OFF)
option(SPIX_BUILD_QTQUICK "Build the QtQuick scene library." ON)
option(SPIX_BUILD_QTWIDGETS "Build the QtWidgets scene library." OFF)
set(SPIX_QT_MAJOR "6" CACHE STRING "Major Qt version to build Spix against")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

set(BUILD_WITH_LOG4CPLUS OFF CACHE BOOL "Build AnyRPC without log4cplus" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build AnyRPC without examples" FORCE)
set(BUILD_PROTOCOL_MESSAGEPACK OFF CACHE BOOL "Build AnyRPC without Protocol Messagepack" FORCE)
# AnyRPC still declares cmake_minimum_required(VERSION 2.8); CMake >= 4.0 rejects that.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(_saved_cxx_flags "${CMAKE_CXX_FLAGS}")
string(REPLACE "-Wall" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

FetchContent_Declare(
    AnyRPC
    GIT_REPOSITORY https://github.com/sgieseking/anyrpc.git
    GIT_TAG b1949b3d40849229055ae75cf5334b9d08579502 # master as of 2024-05-14
)
FetchContent_MakeAvailable(AnyRPC)

set(CMAKE_CXX_FLAGS "${_saved_cxx_flags}")

unset(CMAKE_POLICY_VERSION_MINIMUM)

# AnyRPC adds its include directory via include_directories(${CMAKE_SOURCE_DIR}/include),
# which resolves to Spix instead of AnyRPC when AnyRPC is built as a sub-project.
# Put the include directory on the target, where consumers pick it up as well.
target_include_directories(anyrpc PUBLIC $<BUILD_INTERFACE:${anyrpc_SOURCE_DIR}/include>)

# AnyRPC exports a plain `anyrpc` target; Spix consumes the namespaced name.
if(NOT TARGET AnyRPC::anyrpc)
    add_library(AnyRPC::anyrpc ALIAS anyrpc)
endif()
set(AnyRPC_FOUND TRUE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

# Hide symbols unless explicitly flagged with SPIX_EXPORT
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(libs)

if(SPIX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(SPIX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(libs/Core/tests)
    if(SPIX_BUILD_QTQUICK)
        add_subdirectory(libs/Scenes/QtQuick/tests)
    endif()
    if(SPIX_BUILD_QTWIDGETS)
        add_subdirectory(libs/Scenes/QtWidgets/tests)
    endif()
endif()

#
# Install main Spix config
#
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SpixConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SpixConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/Spix/cmake"
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/SpixConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/Spix/cmake"
)
