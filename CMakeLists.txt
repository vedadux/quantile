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
project(verify)
set(CMAKE_CXX_STANDARD 17)
set(PROJECT_TEMPORARY_DIR ${PROJECT_SOURCE_DIR}/tmp)

# set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

execute_process(
  COMMAND ${PROJECT_SOURCE_DIR}/cpu_features.sh ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ID}
  OUTPUT_VARIABLE FOUND_CPU_FEATURES_STR)
string(STRIP ${FOUND_CPU_FEATURES_STR} FOUND_CPU_FEATURES_STR)
string(REPLACE "\n" ";" FOUND_CPU_FEATURES_LIST ${FOUND_CPU_FEATURES_STR})
foreach(FEATURE IN LISTS FOUND_CPU_FEATURES_LIST)
    message(STATUS "Found CPU feature: ${FEATURE}")
endforeach()

set(SUBMODULES cxxopts json)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive ${SUBMODULES}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/AssureOutOfSourceBuilds.txt)
AssureOutOfSourceBuilds()

include_directories(${PROJECT_SOURCE_DIR}/cxxopts/include)
include_directories(${PROJECT_SOURCE_DIR}/json/single_include/nlohmann)

add_library(libsim common.cpp SymbolManager.cpp Symbol.cpp Cell.cpp cell_types.cpp Circuit.cpp Simulator.cpp SymbolVector.cpp)

add_executable(merge_savedata merge_savedata.cpp SaveDataMI.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(USE_UNSPILL no)

function(add_quantile)
    cmake_parse_arguments(ADD_Q "" "NAME;DIR" "SOURCES;DEFINES" ${ARGN})
    # Parse the name and set tb_ and quantile_ targets
    if(NOT DEFINED ADD_Q_NAME)
        message(FATAL_ERROR "NAME not defined")
    endif()
    set(ADD_Q_TB_TARGET tb_${ADD_Q_NAME})
    set(ADD_Q_MI_TARGET quantile_${ADD_Q_NAME})
    message(STATUS "Setting TB_TARGET to ${ADD_Q_TB_TARGET}")
    message(STATUS "Setting MI_TARGET to ${ADD_Q_MI_TARGET}")

    # Set result directory to tmp if not provided
    if(NOT DEFINED ADD_Q_DIR)
        set(ADD_Q_DIR ${PROJECT_TEMPORARY_DIR})
        message(STATUS "Setting DIR to ${ADD_Q_DIR}")
    endif()

    # Define names of generated files
    set(ADD_Q_GENERATED_CPP  ${ADD_Q_DIR}/${ADD_Q_NAME}.cpp)
    set(ADD_Q_GENERATED_VCD  ${ADD_Q_DIR}/${ADD_Q_NAME}.vcd)
    set(ADD_Q_GENERATED_O    ${ADD_Q_DIR}/${ADD_Q_NAME}.o)
    set(ADD_Q_GENERATED_HASH ${ADD_Q_DIR}/${ADD_Q_NAME}.hash)

    # Make sure sources are provided
    if(NOT DEFINED ADD_Q_SOURCES)
        message(FATAL_ERROR "SOURCES not defined")
    endif()

    # Add target for the testbench
    add_executable(${ADD_Q_TB_TARGET} ${ADD_Q_SOURCES})
    target_compile_definitions(${ADD_Q_TB_TARGET} PUBLIC GENERATED_CPP="${ADD_Q_GENERATED_CPP}")
    target_compile_definitions(${ADD_Q_TB_TARGET} PUBLIC GENERATED_VCD="${ADD_Q_GENERATED_VCD}")

    if(DEFINED ADD_Q_DEFINES)
        foreach(DEF IN LISTS ADD_Q_DEFINES)
            message(STATUS "Adding definition ${DEF}")
            target_compile_definitions(${ADD_Q_TB_TARGET} PUBLIC ${DEF})
        endforeach()
    endif()
    target_link_libraries(${ADD_Q_TB_TARGET} libsim)

    # Add command for generating the runner CPP
    add_custom_command(OUTPUT ${ADD_Q_GENERATED_CPP}
      COMMAND ${ADD_Q_TB_TARGET}
      DEPENDS ${ADD_Q_TB_TARGET} ${PROJECT_SOURCE_DIR}/circuit_utils.h
    )

    # Add command for generating the runner OBJ
    add_custom_command(OUTPUT ${ADD_Q_GENERATED_O}
      COMMAND ${CMAKE_CXX_COMPILER} ${ADD_Q_GENERATED_CPP} ${FOUND_CPU_FEATURES_LIST} -O3 -g -c -o ${ADD_Q_GENERATED_O}
      DEPENDS ${ADD_Q_GENERATED_CPP}
    )

    # Add command for generating the object HASH
    add_custom_command(OUTPUT ${ADD_Q_GENERATED_HASH}.o
      COMMAND ${PROJECT_SOURCE_DIR}/create_hash.sh ${ADD_Q_GENERATED_O} ${ADD_Q_GENERATED_HASH}
      COMMAND ${CMAKE_CXX_COMPILER} -x c++ -g -c ${ADD_Q_GENERATED_HASH} -o ${ADD_Q_GENERATED_HASH}.o
      DEPENDS ${ADD_Q_GENERATED_O}
    )

    # Add executable quantile binary
    add_executable(${ADD_Q_MI_TARGET}
      ${PROJECT_SOURCE_DIR}/verify_mi.cpp ${PROJECT_SOURCE_DIR}/OptionsMI.cpp ${PROJECT_SOURCE_DIR}/SaveDataMI.cpp
      ${ADD_Q_GENERATED_O} ${ADD_Q_GENERATED_HASH}.o)
    target_link_libraries(${ADD_Q_MI_TARGET} PRIVATE Threads::Threads)
    target_compile_options(${ADD_Q_MI_TARGET} PUBLIC -O3 -g)
    foreach(FEATURE IN LISTS FOUND_CPU_FEATURES_LIST)
        target_compile_options(${ADD_Q_MI_TARGET} PUBLIC ${FEATURE})
    endforeach()
endfunction()

##################################### AES DOM #####################################
add_subdirectory(tests/aes_dom)

##################################### AES 2BIT #####################################
add_subdirectory(tests/aes_2bit)

