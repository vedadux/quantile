#
#   Copyright 2023 Vedad Hadžić, Graz University of Technology
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

cmake_minimum_required(VERSION 3.10)

set(JSON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/aes_2bit.json)
message(STATUS "Setting JSON_FILE=${JSON_FILE}")

add_quantile(
  NAME aes_2bit
  SOURCES tb_aes_2bit.cpp
  DEFINES JSON_FILE="${JSON_FILE}"
)

add_custom_command(OUTPUT ${JSON_FILE}
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/../synth.py
    -j ${CMAKE_CURRENT_SOURCE_DIR}/aes_2bit.json
    -v ${CMAKE_CURRENT_SOURCE_DIR}/aes_2bit.v
    -svl ${CMAKE_SOURCE_DIR}/designs/aes_2bit/aes_beautified_verilog.v
    -t aes_top --no-synth --no-opt
    --license ${CMAKE_CURRENT_SOURCE_DIR}/license-header.txt
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../synth.py ${CMAKE_SOURCE_DIR}/designs/aes_2bit/aes_beautified_verilog.v
)

add_custom_target(aes_2bit_json DEPENDS ${JSON_FILE})
add_dependencies(tb_aes_2bit aes_2bit_json)
