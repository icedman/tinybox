#!/usr/bin/python

import re;

pat = re.compile(r"int\s([a-zA-Z0-9_]*);")

for l in open('style.h'):
    l = l.strip()
    m = re.match(pat, l)
    if not m is None:
        t = m.group(1)
        n = t.replace('_', '.') + ":";
        f = "parseValue"
        if "width" in n or "Width" in n:
            f = "parseInt"
        if "height" in n or "Height" in n:
            f = "parseInt"
        if "color" in n or "Color" in n:
            f = "parseColor"
        # print("{ \"" + n + ":\", (int*)&t->" + t + " },")
        print("{ \"" + n + "\", "+f+" } ,")