cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(imu_unoq_evk)

if(NOT DEFINED IMU_TARGET)
    set(IMU_TARGET "IM3J" CACHE STRING "IMU_TARGET is not defined, defaulting to IM3J")
endif()

if(IMU_TARGET STREQUAL "IM3J" OR IMU_TARGET STREQUAL "IM3J_STUB")
    set(IMU_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/IM3J)
    set(IMU_PLATFORM "IM3J")
elseif(IMU_TARGET STREQUAL "IM8S" OR IMU_TARGET STREQUAL "IM8S_STUB")
    set(IMU_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/IM8S)
    set(IMU_PLATFORM "IM8S")
else()
    message(FATAL_ERROR "Unsupported IMU_TARGET value: ${IMU_TARGET}")
endif()

if(IMU_TARGET STREQUAL "IM3J")
    target_compile_definitions(app PRIVATE -DSELECTED_PLATFORM_IM3J -DIMU_TARGET_IM3J)
    message(STATUS "Using IM3J code")
elseif(IMU_TARGET STREQUAL "IM3J_STUB")
    target_compile_definitions(app PRIVATE -DSELECTED_PLATFORM_IM3J -DIMU_TARGET_IM3J)
    message(STATUS "Using IM3J code (stub)")
elseif(IMU_TARGET STREQUAL "IM8S")
    target_compile_definitions(app PRIVATE -DSELECTED_PLATFORM_IM8S -DIMU_TARGET_IM8S)
    message(STATUS "Using IM8S code")
elseif(IMU_TARGET STREQUAL "IM8S_STUB")
    target_compile_definitions(app PRIVATE -DSELECTED_PLATFORM_IM8S -DIMU_TARGET_IM8S)
    message(STATUS "Using IM8S code (stub)")
endif()

if(IMU_TARGET STREQUAL "IM3J")
    add_library(DsImuSdk STATIC IMPORTED)
    set_target_properties(DsImuSdk PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/IM3J/DsImuSdk_Zephyr_CM33.a
    )
elseif(IMU_TARGET STREQUAL "IM8S")
    add_library(DsImuSdk STATIC IMPORTED)
    set_target_properties(DsImuSdk PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/IM8S/DsImuSdk_Zephyr_CM33.a
    )
elseif(IMU_TARGET STREQUAL "IM3J_STUB")
    add_library(DsImuSdk STATIC IMPORTED)
    set_target_properties(DsImuSdk PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/IM3J/DsImuSdk_placeholder.a
    )
elseif(IMU_TARGET STREQUAL "IM8S_STUB")
    add_library(DsImuSdk STATIC IMPORTED)
    set_target_properties(DsImuSdk PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/IM8S/DsImuSdk_placeholder.a
    )
endif()

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${IMU_INCLUDE_DIR}
)

include_directories(${IMU_INCLUDE_DIR})

target_sources(app PRIVATE
    src/main.c
    src/zephyr_spi_hal.c
    src/zephyr_systick.c
    src/zephyr_async_uart.c
    src/uart_protocol.c
    src/Serial_Cmd.c
    src/nvs_config.c
)

target_compile_options(app PRIVATE
    -Wno-psabi
    -O2
)

target_link_libraries(app PRIVATE DsImuSdk stdc++)

set_property(TARGET app PROPERTY CXX_STANDARD 17)
