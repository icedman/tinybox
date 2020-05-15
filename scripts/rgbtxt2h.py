#!/usr/bin/python

import re

pat = re.compile(r"([0-9a-zA-Z]{1,3})[\t\s]*([0-9a-zA-Z]{1,3})[\t\s]*([0-9a-zA-Z]{1,3})[\t\s]{3,20}(.*)")

print("\
#include <string.h>\n\n\
typedef struct _colorEntry {\n\
    int r;\n\
    int g;\n\
    int b;\n\
    char *name;\n\
} colorEntry;\n\
\n\
colorEntry colorTable[] = {\
")

for l in open('rgb.txt'):
    l = l.strip()
    m = re.match(pat, l)
    if not m is None:
        o = '{ '
        o += m.group(1) + ', '
        o += m.group(2) + ', '
        o += m.group(3) + ', "'
        o += m.group(4) + '"'
        o += ' }, '
        print(o)

print('{ 0,0,0,0 }')
print('};\n')