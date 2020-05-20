#!/usr/bin/python

import re
import sys

from os import listdir
from os.path import isfile, join

pat = re.compile(r"([a-zA-Z0-9\.]*):")

current_type = ""
configs = []
config_types = {}

def process_theme(path, name):
    global current_type
    # print(current_type + ":" + name + " ~" + path)
    for l in open(path):
        l = l.strip()
        m = re.match(pat, l)
        if not m is None:
            # print(m.group(0))
            nn = m.group(0);
            # nn = nn.lower()
            if nn not in config_types:
                 config_types[nn] = {}
            config_types[nn][current_type] = current_type
            if nn not in configs:
                configs.append(nn)

def process_file(path, name):
    if name.endswith(".xpm") or name.endswith(".xbm"): 
        return
    if name.endswith(".am"): 
        return
    if name.endswith(".in"): 
        return
    if name == "Makefile":
        return
    
    process_theme(path, name)

def process_dir(path, prev):
    for d in listdir(path):
        name = d
        if "themerc" in name or "theme.cfg" in name:
            name = prev
        file_path = join(path, d)
        if isfile(file_path):
            process_file(file_path, name)
            # break
        else:
            if prev != "" and name == "openbox-3":
                process_dir(join(file_path), prev)
            else:
                process_dir(file_path, name)


# types = [ "bbwm", "fb", "ob" ]
types = [ "bbwm", "fb" ]

def read_styles():
    global current_type
    for type in types:
        current_type = type
        style_dir = "../styles/" + type
        process_dir(style_dir, "")
    configs.sort()

def generate_map(configs, gtype):
    for n in configs:
        comment = "//"
        for type in types:
            if type in config_types[n]:
                comment += " " + type

        cfg_identifier = "\"" + n + "\""
        field = n.replace('.', '_').replace(':','')
        field_identifier = "&style->" + field
        field_identifier_string = "style->" + field

        t = "int"
        f = "parseValue"
        if "font" in n or "Font" in n:
            f = "parseString"
        if "pixmap" in n or "Pixmap" in n:
            f = "parseString"
        if "width" in n or "Width" in n:
            f = "parseInt"
        if "height" in n or "Height" in n:
            f = "parseInt"
        if "color" in n or "Color" in n:
            f = "parseColor"

        if f == "parseString":
            t = "char*";

        if gtype == "-s" or gtype == "s":
            if t == "char*":
                print(field_identifier_string +",")

        if gtype == "-h" or gtype == "h":
            line = t + " " + field + ";"
            h = line.ljust(60) + comment
            print(h)
        if gtype == "-m" or gtype == "-m":
            line1 = "{ " +cfg_identifier + ","
            line2 = "\n\t\t" + field_identifier + ", " + f + " },"
            line2 = line2.ljust(60) + comment
            map = line1 + line2
            print(map)

if len(sys.argv) < 2:
    print("usage: -[h,s,m]")
    exit()

tt = sys.argv[1]

read_styles()

# generate_map(configs, "-h")
# generate_map(configs, "-m")

generate_map(configs, tt)