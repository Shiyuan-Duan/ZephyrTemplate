# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers/max30003
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers/bme280
  # ${CMAKE_CURRENT_SOURCE_DIR}/lib/tflite-micro
  
)


find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Template)

target_sources(app PRIVATE src/main.c)
