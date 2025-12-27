cmake_minimum_required(VERSION 3.16)
project(vterm-asci LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG REQUIRED
    libavcodec
    libavformat
    libavutil
    libswscale
)

file(GLOB_RECURSE SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_library(vterm-asci_lib ${SOURCES})

target_include_directories(vterm-asci_lib PRIVATE
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(vterm-asci_lib PRIVATE
    ${FFMPEG_LIBRARIES}
)

target_compile_options(vterm-asci_lib PRIVATE
    ${FFMPEG_CFLAGS_OTHER}
)

add_executable(vterm-asci src/main.cpp)

target_link_libraries(vterm-asci PRIVATE
    vterm-asci_lib
)
