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

set(JSON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/aes_dom.json)
message(STATUS "Setting JSON_FILE=${JSON_FILE}")

add_quantile(
  NAME aes_dom_fresh
  SOURCES ../common_tb_utils.cpp tb_aes_dom.cpp
  DEFINES JSON_FILE="${JSON_FILE}" MASK_USE=MaskUse::FRESH_MASKS 
)

add_quantile(
  NAME aes_dom_fixed
  SOURCES ../common_tb_utils.cpp tb_aes_dom.cpp
  DEFINES JSON_FILE="${JSON_FILE}" MASK_USE=MaskUse::ALWAYS_SAME_MASKS 
)

add_quantile(
  NAME aes_dom_none
  SOURCES ../common_tb_utils.cpp tb_aes_dom.cpp
  DEFINES JSON_FILE="${JSON_FILE}" MASK_USE=MaskUse::NO_MASKS 
)

add_quantile(
  NAME aes_dom_rep_1
  SOURCES ../common_tb_utils.cpp tb_aes_dom.cpp
  DEFINES JSON_FILE="${JSON_FILE}" MASK_USE=MaskUse::SAME_MASKS_AFTER_N_ROUNDS NUM_ROUNDS_BEFORE_MASK_REPEAT=1 
)

add_quantile(
  NAME aes_dom_rep_2
  SOURCES ../common_tb_utils.cpp tb_aes_dom.cpp
  DEFINES JSON_FILE="${JSON_FILE}" MASK_USE=MaskUse::SAME_MASKS_AFTER_N_ROUNDS NUM_ROUNDS_BEFORE_MASK_REPEAT=2 
)

add_executable(aes_dom_speed_test ${PROJECT_SOURCE_DIR}/sampler_speed_test.cpp ${PROJECT_TEMPORARY_DIR}/aes_dom_fresh.cpp)
target_compile_options(aes_dom_speed_test PUBLIC -O3 -g)
foreach(FEATURE IN LISTS FOUND_CPU_FEATURES_LIST)
    target_compile_options(aes_dom_speed_test PUBLIC ${FEATURE})
endforeach()