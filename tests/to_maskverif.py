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

# coding: utf-8
import re
import os
import sys

if len(sys.argv) < 3:
    print("usage: {sys.argv[0]} INPUT OUTPUT")
    sys.exit(1)

in_file = sys.argv[1]
out_file = sys.argv[2]

num_cycles = -1
if len(sys.argv) >= 4:
    num_cycles = int(sys.argv[3])

TMP_DIR = os.path.abspath(os.path.dirname(os.path.realpath(__file__)) + "/../tmp")

with open(in_file, "r") as f:
    code = f.read()

run_circuit_rex = re.compile("run_circuit_cycle_(\d+)\(wtype_t\* s\)\s+\{([^}]*)\}", flags=re.MULTILINE)
define_rex = re.compile("#define\s+(\S+)\s+(_[a-z]+)\s*\(\s*(\S+)\s*(?:,\s*(\S+)\s*)?\)\s*")
assign_rex = re.compile("(\S+)\s*=\s*(\S+)\s*;\s*")
rename_rex = re.compile("#define\s+(\S+)\s+([^,]+)")
debug_info_rex = re.compile("const\s+char\s*\*\s+DEBUG_INFO\s*\[\s*(\d+)\s*\]\s*=\s*\{([^}]*)\}\s*;")
debug_entry = re.compile("\"([^\"]*)\",?")



OP_MAP = {"_xor": lambda x: f"{x[0]} + {x[1]}", 
          "_and": lambda x: f"{x[0]} * {x[1]}", 
          "_or" : lambda x: f"{x[0]} * {x[1]}", 
          "_not": lambda x: f"{x[0]}"}

pos = 0
cycle_code = {}
while True:
    m = run_circuit_rex.search(code, pos=pos)
    if m is None: break
    cycle_code[int(m.groups()[0])] = m.groups()[1]
    pos = m.span()[1]
cycle_code = {i:[c.strip() for c in cc.strip().split("\n")] for i,cc in cycle_code.items()}

debug_info_match = debug_info_rex.search(code)
assert(debug_info_match is not None)
debug_info_size = int(debug_info_match.groups()[0])
debug_info_code = debug_info_match.groups()[1].strip();
debug_info_code = [debug_entry.match(l.strip()).groups()[0] for l in debug_info_code.split("\n")]
assert(len(debug_info_code) == debug_info_size)

share_info_rex = re.compile("(secret|data) (\d+) share (\d+)")
unshared_info_rex = re.compile("(secret|data) (\d+) unmasked")
mask_info_rex = re.compile("mask (\d+)")

def mangled_id(name):
    for i in range(10):
        name = name.replace(str(i), chr(ord('a') + i))
    return name

def get_shares_names(code):
    renames = {}
    inputs = {}
    deleted = set()
    masks = []
    for idx, c in enumerate(code):
        m_share = share_info_rex.fullmatch(c)
        m_unsh = unshared_info_rex.fullmatch(c)
        m_mask = mask_info_rex.fullmatch(c)
        if m_share is not None:
            shared_id = f"{m_share.groups()[0]}_{mangled_id(m_share.groups()[1])}"
            if shared_id not in inputs.keys():
                inputs[shared_id] = 0
            inputs[shared_id] += 1
            renames[f"s_{idx}"] = f"{shared_id}[{m_share.groups()[2]}]"
        elif m_unsh is not None:
            deleted.add(f"s_{idx}")
        elif m_mask is not None:
            mask_id = f"mask_{m_mask.groups()[0]}"
            renames[f"s_{idx}"] = mask_id
            masks.append(mask_id)

    return inputs, masks, renames, deleted

sid_rex = re.compile("s\[(\d+)\]")
tid_rex = re.compile("t(\d+)")

sid_new_rex = re.compile("s_(\d+)")
tid_new_rex = re.compile("t_(\d+)")


def is_id(x): return "+" not in x and "*" not in x

def get_sid(x): return int(sid_rex.fullmatch(x).groups()[0])
def get_tid(x): return int(tid_rex.fullmatch(x).groups()[0])
 
s_defs = {}
t_defs = {}
instrs = []
inputs, masks, renames, deleted = get_shares_names(debug_info_code)

if num_cycles == -1:
    num_cycles = max(cycle_code.keys()) + 1
for cycle in range(num_cycles):
    if cycle_code[cycle] == [""]:
        continue
    for line in cycle_code[cycle]:
        if line.startswith("//"): 
            continue
        d_match = define_rex.fullmatch(line)
        a_match = assign_rex.fullmatch(line)
        r_match = rename_rex.fullmatch(line)
        if d_match is not None:
            t_id = get_tid(d_match.groups()[0])
            op = d_match.groups()[1]
            args = []
            for g in d_match.groups()[2:]:
                if g is None: continue
                if g.startswith("t"): args.append(f"t_{get_tid(g)}")
                elif g.startswith("s"): args.append(f"s_{get_sid(g)}")
                else: assert(False)
            args = [a if a not in renames else renames[a] for a in args]    
            if any(map(lambda x: x in deleted, args)):
                deleted.add(f"t_{t_id}")
            else:    
                t_defs[t_id] = OP_MAP[op](args)
                if is_id(t_defs[t_id]):
                    res = t_defs[t_id] if t_defs[t_id] not in renames else renames[t_defs[t_id]]
                    renames[f"t_{t_id}"] = res
                else:   
                    instrs.append((f"t_{t_id}", t_defs[t_id]))
        elif a_match is not None:
            s_id = get_sid(a_match.groups()[0])
            t_id = get_tid(a_match.groups()[1])
            if f"s_{s_id}" in renames:
                renames[f"f_{t_id}"] = renames[f"s_{s_id}"]
            else:    
                s_defs[s_id] = t_defs[t_id]
                res = f"t_{t_id}" 
                if res in renames: res = renames[res]
                renames[f"s_{s_id}"] = res
        elif r_match is not None:
            t_id = get_tid(r_match.groups()[0])
            s_id = get_sid(r_match.groups()[1])
            t_defs[t_id] = f"s_{s_id}"
            res = f"s_{s_id}" 
            if res in renames: res = renames[res]
            renames[f"t_{t_id}"] = res
        else:
            assert(f"\"{line}\" not matched")

instrs = [instr if instr[0] not in renames else (renames[instr[0]], instr[1])
          for instr in instrs]


maskverif_code = "\n".join(f"\t{instr[0]} := {instr[1]} ;" for instr in instrs)

with open(out_file, "w+") as f:
    f.write("proc design:\n")
    f.write("\tinputs: " + ", ".join([f"{idx}[0:{l-1}]" for idx,l in inputs.items()]) + "\n")
    f.write("\toutputs: \n")
    f.write("\trandoms: " + ", ".join([m for m in masks]) + " ;\n")
    
    f.write(maskverif_code)
    f.write("\nend\n")
    f.write("noglitch Probing design\n")
