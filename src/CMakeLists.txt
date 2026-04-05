cmake_minimum_required(VERSION 3.16)

project(GermaniumDetector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIM_MODE "Build in simulation mode (heap-backed registers, no-op I2C)" OFF)

#---------------------------------------------------------------------------
# Dependencies
#---------------------------------------------------------------------------

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

#---------------------------------------------------------------------------
# Source files
#---------------------------------------------------------------------------

set(SOURCES
    detector_main.cpp
    common/Logger/Logger.cpp
    common/DeviceWorker/DeviceWorker.cpp
    device/Zynq/Register/Register.cpp
    device/Zynq/PsI2c/PsI2c.cpp
    device/Mars/Mars.cpp
    detector/Germanium/GermaniumDetector.cpp
    detector/Germanium/GermaniumZynq.cpp
    detector/Germanium/GermaniumZMQ.cpp
)

#---------------------------------------------------------------------------
# Include directories
#---------------------------------------------------------------------------

set(INCLUDE_DIRS
    common/base
    common/Logger
    common/DeviceMsg
    common/DeviceWorker
    common/ThreadQueue
    device/Network
    device/Zynq
    device/Zynq/Register
    device/Zynq/PsI2c
    device/Mars
    detector/Germanium
)

#---------------------------------------------------------------------------
# Build
#---------------------------------------------------------------------------

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIRS}
    ${ZMQ_INCLUDE_DIRS}
)

if(SIM_MODE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SIM_MODE)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${ZMQ_LIBRARIES}
    pthread
)
