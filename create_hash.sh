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

SRC_FILE=$1
DST_FILE=$2
HASH=$(sha256sum "$SRC_FILE" | sed -E "s/([0-9a-f]+).*$/\1/g");
echo "const char* OBJ_HASH = \"$HASH\";" > "$DST_FILE" ;