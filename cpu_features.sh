#!/bin/bash

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

FEATURES=("sse2" "avx2" "avx512f")

COMPILER_BIN=$1
COMPILER_ID=$2

for feature in "${FEATURES[@]}"; do
  if [ "$COMPILER_ID" = "Clang" ]; then
    "$COMPILER_BIN" -E - -march=native -### 2>&1 | grep -q -- "\"-target-feature\" \"+$feature\"" ;
  elif [ "$COMPILER_ID" = "GNU" ]; then
    "$COMPILER_BIN" -Q -march=native --help=target | grep -q -E -- "-m$feature\s+\[enabled\]" ;
  else
    false ;
  fi
  if [ "$?" = "0" ]; then
    echo "-m$feature" ;
  fi
done