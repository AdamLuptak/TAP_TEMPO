cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(PROJECT_NAME TAP_TEMPO)
project(${PROJECT_NAME})

set(${CMAKE_PROJECT_NAME}_BOARD uno)
set(${CMAKE_PROJECT_NAME}_PORT /dev/ttyUSB0)

set(${CMAKE_PROJECT_NAME}_SKETCH TAP_TEMPO.ino)
generate_arduino_firmware(${CMAKE_PROJECT_NAME})
