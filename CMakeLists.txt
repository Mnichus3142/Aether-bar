cmake_minimum_required(VERSION 4.0)
project(Aether_bar)

set(CMAKE_CXX_STANDARD 26)

find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(LAYER_SHELL REQUIRED IMPORTED_TARGET gtk4-layer-shell-0)
pkg_check_modules(JSONCPP REQUIRED IMPORTED_TARGET jsoncpp)

add_executable(Aether_bar main.cpp
        src/Creator.cpp
        src/Creator.h
        src/Element.h
        src/Zones.h
)

target_link_libraries(Aether_bar PRIVATE
        PkgConfig::GTK4
        PkgConfig::LAYER_SHELL
        PkgConfig::JSONCPP
        spdlog::spdlog
)