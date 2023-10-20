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

import argparse
import subprocess as sp
import os
import sys
import shutil
import json

def ap_check_file_exists(file_path):
    if not os.path.isfile(file_path):
        raise argparse.ArgumentTypeError("File '%s' does not exist" % file_path)
    return os.path.abspath(file_path)

def ap_check_dir_exists(file_path):
    dir_path = os.path.dirname(os.path.abspath(file_path))
    if not os.path.isdir(dir_path):
        raise argparse.ArgumentTypeError("Directory '%s' does not exist" % dir_path)
    return os.path.abspath(file_path)

def check_program(program):
    try:
        print("Checking \"%s\": " % " ".join(program), end="")
        r = sp.check_output(program, stderr=sp.PIPE)
        print("SUCCESS")
        return True
    except:
        print("FAILED")
        return False


VERILOG = "Verilog"
VHDL = "VHDL"
SYS_VERILOG = "SystemVerilog"
YOSYS_CMD = ["yosys"]
DOCKER_PREFIX = ["docker", "run", "--rm"] 
DOCKER_IMAGE = ["hdlc/ghdl:yosys"]
YOSYS_GHDL_CMD = ["yosys", "-m", "ghdl"]
SYNTH_TMP = "/tmp/synth"


def parse_args():
    parser = argparse.ArgumentParser(description="Yosys Synthesis")
    parser.add_argument("-j", "--jout", dest="json_out_path", type=ap_check_dir_exists, required=True, 
                        help="Output path of compiled circuit in JSON format")
    parser.add_argument("-v", "--vout", dest="vlog_out_path", type=ap_check_dir_exists, required=True, 
                        help="Output path of compiled circuit in Verilog format")
    parser.add_argument("-svh", "--source-vhdl", dest="src_vhdl_paths", nargs="+", type=ap_check_file_exists,
                        help="File path(s) to VHDL source file(s)", metavar="VHDL_SOURCE_FILES")
    parser.add_argument("-svl", "--source-vlog", dest="src_vlog_paths", nargs="+", type=ap_check_file_exists,
                        help="File path(s) to (System)Verilog source file(s)", metavar="VLOG_SOURCE_FILES")                    
    parser.add_argument("-t", "--top", dest="top", required=True, type=str, 
                        help="Name of the top module")
    parser.add_argument("--no-synth", dest="do_synth", action="store_false", default=True,
                        help="Do not synthesize the circuit")
    parser.add_argument("--no-opt", dest="do_opt", action="store_false", default=True,
                        help="Do not optimize with ABC or after synthesis")
    parser.add_argument("--ghdl-flags", dest="ghdl_flags", nargs="+", type=str,
                        help="Additional flags provided to ghdl")
    parser.add_argument("--license", dest="license_path", type=ap_check_file_exists,
                        help="License file containing appropriate license header")                    
    return parser.parse_args()

def clean_paths_with_space(paths):
    new_paths = []
    if not os.path.exists(SYNTH_TMP + "/"):
        os.makedirs(SYNTH_TMP)
    for f in paths:
        if " " not in f:
            new_paths.append(f)
        else:
            assert(f[0] == "/")
            dst_f = SYNTH_TMP + f.replace(" ", "_")
            os.makedirs(os.path.dirname(dst_f), exist_ok=True)
            shutil.copy(f, dst_f)
            new_paths.append(dst_f)
    return new_paths


def create_mounts(paths, mount_pairs):
    new_paths = []
    for f in paths:
        assert(" " not in f)
        f_dir = os.path.dirname(f)
        if f_dir not in mount_pairs:
            mount_pairs[f_dir] = "/" + f_dir[1:].replace("/", "::")
        new_paths.append(f.replace(f_dir, mount_pairs[f_dir]))
    return new_paths


def cmd_prefix(args):
    if args.src_vhdl_paths is not None:
        args.src_vhdl_paths = clean_paths_with_space(args.src_vhdl_paths)
    
    if args.src_vlog_paths is not None:
        args.src_vlog_paths = clean_paths_with_space(args.src_vlog_paths)
    
    if (args.src_vhdl_paths is None): return []
    
    prefix = DOCKER_PREFIX
    mount_pairs = {}
    
    if args.src_vhdl_paths is not None:
        args.src_vhdl_paths = create_mounts(args.src_vhdl_paths, mount_pairs)
    
    if args.src_vlog_paths is not None:
        args.src_vlog_paths = create_mounts(args.src_vlog_paths, mount_pairs)
        
    assert(len(mount_pairs) != 0)
    
    json_dir = os.path.dirname(args.json_out_path)
    vlog_dir = os.path.dirname(args.vlog_out_path)
    
    if json_dir not in mount_pairs:
        mount_pairs[json_dir] = "/" + json_dir[1:].replace("/", "::")
    
    args.json_out_path = args.json_out_path.replace(json_dir, mount_pairs[json_dir])

    if vlog_dir not in mount_pairs:
        mount_pairs[vlog_dir] = "/" + vlog_dir[1:].replace("/", "::")
    
    args.vlog_out_path = args.vlog_out_path.replace(vlog_dir, mount_pairs[vlog_dir])

    for src,dst in mount_pairs.items():
        prefix += ["--mount", "type=bind,source=%s,destination=%s" % (src,dst)]
    
    prefix += DOCKER_IMAGE
    return prefix


def read_cmd(args):
    cmd = ""
    if args.src_vlog_paths is not None:
        for f in args.src_vlog_paths:
            defer = "-defer" if args.src_vhdl_paths is None else ""
            cmd += f"read_verilog -sv {defer} \"{f}\" ; "
    
    if args.src_vhdl_paths is not None:
        ghdl_flags = ""
        if args.ghdl_flags is not None:
            ghdl_flags = "".join(["-%s" % f for f in args.ghdl_flags])
        cmd += "ghdl %s " % ghdl_flags
        for f in args.src_vhdl_paths:
            assert (" " not in f)
            cmd += f"{f} "
        cmd += f"-e {args.top} ; "
    return cmd


def main(args):
    if (args.src_vhdl_paths is None and not check_program(YOSYS_CMD + ["-V"])):
        sys.exit(1)
    if (args.src_vhdl_paths is not None and not check_program(DOCKER_PREFIX + DOCKER_IMAGE + YOSYS_GHDL_CMD + ["-V"])):
        sys.exit(2)
    
    prefix = cmd_prefix(args)
    yosys_cmd = read_cmd(args)
    if (args.do_synth):
        use_abc = "" if args.do_opt else "-noabc"
        yosys_cmd += "synth %s -top %s; flatten; " % (use_abc, args.top)
    else:
        yosys_cmd += "hierarchy -check -top %s; proc; flatten; " % args.top
    yosys_cmd += "techmap; "
    if (args.do_opt):
        yosys_cmd += "opt_expr; "
    yosys_cmd += "clean -purge; "
    yosys_cmd += "rename -enumerate *; clean -purge; "
    yosys_cmd += "write_verilog -noattr %s; " % args.vlog_out_path
    yosys_cmd += "write_json %s; " % args.json_out_path
    
    yosys = YOSYS_GHDL_CMD if (args.src_vhdl_paths is not None) else YOSYS_CMD
    cmd = prefix + yosys + ["-p", yosys_cmd]
    
    try:
        print(" ".join(cmd))
        sys.stdout.flush()
        proc = sp.Popen(cmd, stderr=sp.STDOUT, stdout=sp.PIPE)
        while True:
            line = proc.stdout.readline()
            print(line.decode(), end="")
            if not line: break
        r = proc.wait()
        if (r == 0):
            print("Synthesis SUCCESS")
            if args.license_path is not None:
                with open(args.license_path, "r") as f:
                    license = f.read()
                with open(args.json_out_path, "r") as f:
                    circ_json = json.load(f)
                circ_json["license"] = license
                with open(args.json_out_path, "w") as f:
                    json.dump(circ_json, f, indent=4)
                with open(args.vlog_out_path, "r") as f:
                    circ_vlog = f.read()
                with open(args.vlog_out_path, "w") as f:
                    for l in license.split("\n"):
                        f.write("//   ")
                        f.write(l)
                        f.write("\n")
                    f.write("\n")
                    f.write(circ_vlog)
            sys.exit(0)
        else:
            print("Synthesis FAILED")
            sys.exit(3)    
    except Exception as e:
        print(e)
        print("Synthesis FAILED")
        sys.exit(4)

if __name__ == "__main__":
    main(parse_args())
