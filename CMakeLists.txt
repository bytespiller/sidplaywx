cmake_minimum_required(VERSION 3.11.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(sidplaywx CXX)

#set(THREADS_PREFER_PTHREAD_FLAG ON)

# external headers
include(${CMAKE_SOURCE_DIR}/deps/include/msw/libsidplayfp/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/deps/include/msw/portaudio/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/deps/include/msw/wxWidgets/CMakeLists.txt)

# external libs
include(${CMAKE_SOURCE_DIR}/deps/lib/msw/libsidplayfp/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/deps/lib/msw/portaudio/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/deps/lib/msw/wxWidgets/CMakeLists.txt)

# exe
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*)
#message(STATUS "[INFO] SRC_FILES: ${SRC_FILES}")

set(SRC_FILES ${SRC_FILES} ../dev/resources.rc) # MSW manifest & stuff

add_executable(${PROJECT_NAME} ${SRC_FILES})

# libs
find_package(Threads)
target_link_libraries(${PROJECT_NAME}
    libsidplayfp
    libportaudio
    setupapi dsound dxguid winmm # Windows-specific (for audio)
    Threads::Threads
    -static-libgcc -static-libstdc++ # needed for MinGW posix threading
    -liconv # string encoding operations (manual)

    -mwindows # WxWidgets, don't open console/terminal
    liboleacc.a # WxWidgets MSW, must be defined BEFORE the wxWidgets_core
    wxWidgets_propgrid wxWidgets_adv # prefs window (must be included before others)
    wxWidgets_core wxWidgets_base wxWidgets_regex
    wxWidgets_jpeg wxWidgets_tiff wxWidgets_png wxWidgets_zlib
    wxWidgets_xml wxWidgets_libwxexpat # wx XML support
    wxWidgets_net # for "Check for updates" support

    shlwapi version # needed for WxWidgets static linking and is MinGW specific
    uxtheme kernel32 user32 gdi32 comdlg32 winspool winmm shell32 comctl32 ole32 oleaut32 uuid rpcrt4 advapi32 # wxWidgets MSW
)

target_compile_options(${PROJECT_NAME} PRIVATE -Werror -DHAVE_CONFIG_H -DHAVE_CXX11) # TODO: try -fno-ms-extensions for extra errors (ISO standard violations and such)
#target_compile_options(${PROJECT_NAME} PUBLIC -g -O0 -Werror -Wall -Wextra -pedantic) # TEMP!!!

# CPack
#set(CPACK_PROJECT_NAME ${PROJECT_NAME})
#set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)
