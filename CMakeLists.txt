cmake_minimum_required(VERSION 3.16)

project(New_Player VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick  Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick Multimedia)
include_directories("D:\\code\\Qt_Player\\ffmpeg\\include")
set(FFMPEG_LIB_DIR "D:\\code\\Qt_Player\\ffmpeg\\lib" CACHE PATH "FFmpeg installation directory")
find_library(AVCODEC_LIBRARY avcodec HINTS ${FFMPEG_LIB_DIR})
find_library(AVFORMAT_LIBRARY avformat HINTS ${FFMPEG_LIB_DIR})
find_library(AVUTIL_LIBRARY avutil HINTS ${FFMPEG_LIB_DIR})
find_library(SWSCALE_LIBRARY swscale HINTS ${FFMPEG_LIB_DIR})
find_library(SWRESAMPLE_LIBRARY swresample HINTS ${FFMPEG_LIB_DIR})
# 检查是否找到所有库
if(NOT AVCODEC_LIBRARY OR NOT AVFORMAT_LIBRARY OR NOT AVUTIL_LIBRARY OR NOT SWSCALE_LIBRARY)
  message(FATAL_ERROR "FFmpeg libraries not found!")
endif()

message(STATUS "AVCODEC_LIBRARY: ${AVCODEC_LIBRARY}")
message(STATUS "AVFORMAT_LIBRARY: ${AVFORMAT_LIBRARY}")
message(STATUS "AVUTIL_LIBRARY: ${AVUTIL_LIBRARY}")
message(STATUS "SWSCALE_LIBRARY: ${SWSCALE_LIBRARY}")
message(STATUS "AVDEVICE_LIBRARY: ${AVDEVICE_LIBRARY}")
message(STATUS "AVFILTER_LIBRARY: ${AVFILTER_LIBRARY}")
message(STATUS "POSTPROC_LIBRARY: ${POSTPROC_LIBRARY}")
message(STATUS "SWRESAMPLE_LIBRARY: ${SWRESAMPLE_LIBRARY}")
set(PROJECT_SOURCES
  main.cpp
  qml.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(New_Player
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    videofboitem.h videofboitem.cpp
    videoitem.h videoitem.cpp
    i420render.h i420render.cpp
    ffmpegplayer.h ffmpegplayer.cpp
    packetqueue.h packetqueue.cpp
    demuxthread.h demuxthread.cpp
    log.h
    threadbase.h threadbase.cpp
    videodecodethread.h videodecodethread.cpp
    audiodecodethread.h audiodecodethread.cpp
    audioplayer.h audioplayer.cpp
    log.cpp
    audiodevice.h audiodevice.cpp
  )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET New_Player APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
  if(ANDROID)
    add_library(New_Player SHARED
      ${PROJECT_SOURCES}
    )
  # Define properties for Android with Qt 5 after find_package() calls as:
  #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
else()
  add_executable(New_Player
    ${PROJECT_SOURCES}
  )
endif()
endif()

target_link_libraries(New_Player
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick
  Qt6::Multimedia
  ${AVCODEC_LIBRARY}
  ${AVDEVICE_LIBRARY}
  ${AVFILTER_LIBRARY}
  ${AVFORMAT_LIBRARY}
  ${AVUTIL_LIBRARY}
  ${POSTPROC_LIBRARY}
  ${SWSCALE_LIBRARY}
  ${SWRESAMPLE_LIBRARY}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.New_Player)
endif()
set_target_properties(New_Player PROPERTIES
  ${BUNDLE_ID_OPTION}
  MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS New_Player
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
  qt_import_qml_plugins(New_Player)
  qt_finalize_executable(New_Player)
endif()
