#define _XOPEN_SOURCE 700 // for realpath

#include "tinybox/style.h"
#include "tinybox/server.h"
#include "tinybox/style_defaults.h"

#include "common/stringop.h"
#include "common/util.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void to_lower_string(char* str, char* out)
{
    for (int i = 0; i < 32 && str[i]; i++) {
        out[i] = tolower(str[i]);
        out[i + 1] = 0;
    }
}

/* Fowler/Noll/Vo (FNV) hash function, variant 1a */
static size_t fnv1a_hash(const char* cp)
{
    size_t hash = 0x811c9dc5;
    while (*cp) {
        hash ^= (unsigned char)*cp++;
        hash *= 0x01000193;
    }
    return hash;
}

typedef void parse_func(int argc, char** argv, void* target, char* name);

struct style_property {
    const char* name;
    void* data;
    parse_func* cmd;
};

struct style_flag {
    char* name;
    int flag;
};

int getPropertyIndex(struct style_property* props, char* name)
{
    for (int i = 0;; i++) {
        struct style_property* p = &props[i];
        if (p->name == 0) {
            break;
        }
        if (strcmp(p->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void parseValue(int argc, char** argv, void* target, char* name)
{
    struct style_flag flagMap[] = { { "solid", (int)sf_solid },
        { "flat", (int)sf_flat },
        { "raised", (int)sf_raised },
        { "diagonal", (int)sf_diagonal },
        { "crossdiagonal", (int)sf_crossdiagonal },
        { "border", (int)sf_border },
        { "bevel", (int)sf_bevel },
        { "bevel1", (int)sf_bevel1 },
        { "bevel2", (int)sf_bevel2 },
        { "gradient", (int)sf_gradient },
        { "interlaced", (int)sf_interlaced },
        { "sunken", (int)sf_sunken },
        { "vertical", (int)sf_vertical },
        { "horizontal", (int)sf_horizontal },
        { 0, 0 } };

    int flags = 0;
    char lowered[255];
    for (int i = 0; i < argc; i++) {
        for (int j = 0;; j++) {
            if (flagMap[j].name == 0) {
                break;
            }

            to_lower_string(argv[i], lowered);

            if (strcmp(flagMap[j].name, lowered) == 0) {
                flags = flags | flagMap[j].flag;
            }
        }

        // printf("%s %d??\n", argv[i], (int)flagMap[0].flag);
    }

    // is file path?

    // is font?

    *(int*)target = flags;
}

void parseInt(int argc, char** argv, void* target, char* name)
{
    *(int*)target = strtol(argv[1], NULL, 10);
    // printf("parseInt: %d\n", *target);
}

void parseString(int argc, char** argv, void* target, char* name)
{
    if (argc < 2) {
        return;
    }

    int l = strlen(argv[1]);
    if (l) {
        *(char**)target = calloc((l + 1), sizeof(char));
        strcpy(*(char**)target, argv[1]);
    }
}

void parseColor(int argc, char** argv, void* target, char* name)
{
    uint32_t color;
    if (parse_color(argv[1], &color)) {
        *(int*)target = color;
    }
    float colors[4];
    color_to_rgba(colors, *(int*)target);
    // printf("parseColor: %f %f %f\n", colors[0], colors[1], colors[2]);
}

void load_style(struct tbx_server* server, const char* path)
{
    struct tbx_style* config_style = &server->style;

    char* expanded = 0;
    if (path) {
        expanded = calloc(strlen(path) + 1, sizeof(char));
        strcpy(expanded, path);
        expand_path(&expanded); // return;
    }

    FILE* f = fopen(expanded, "r");
    if (!f) {
        printf("load defaults\n");
        memcpy(config_style, style_bin, sizeof(struct tbx_style));
        strcpy(config_style->font, "monospace 10");
        free(expanded);
        return;
    }

    struct tbx_style _style;
    struct tbx_style* style = &_style;

    memset(style, 0, sizeof(struct tbx_style));
    // int *styleFirstProp = &style.toolbar;

    style->hash = fnv1a_hash(expanded);
    free(expanded);

    struct style_property pointerMap[] = {
#include "style.ints.h"
        { 0, 0, 0 }
    };

    char* line = NULL;
    size_t line_size = 0;
    size_t nread;
    while (!feof(f)) {
        nread = getline(&line, &line_size, f);
        if (!nread) {
            continue;
        }

        int argc;
        char** argv = split_args(line, &argc);

        if (argc > 0) {
            int idx = getPropertyIndex(pointerMap, argv[0]);
            if (idx == -1) {
                free_argv(argc, argv);
                continue;
            }

            void* target = pointerMap[idx].data;

            // if (pointerMap[idx].cmd == parseString) {
            //   int l = strlen(argv[1]);
            //   if (l) {
            //     *(char**)pointerMap[idx].data = calloc(1, (l+1)*sizeof(char));
            //     strcpy(*(char**)pointerMap[idx].data, argv[1]);
            //   }
            //   // _parseString(argc, argv, target, argv[0]);
            // } else {
            pointerMap[idx].cmd(argc, argv, target, argv[0]);
            // }
        }

        free_argv(argc, argv);
    }

    fclose(f);

    memcpy(config_style, style, sizeof(struct tbx_style));
    strcpy(config_style->font, "monospace 10");

    // dump defaults bin
    // FILE* fp = fopen("./style.bin", "wb");
    // fwrite(style, 1, sizeof(struct tbx_style), fp);
    // fclose(fp);
}

void free_style(struct tbx_style* style)
{
    printf("freeing..\n");
    char* strings[] = {
#include "style.strings.h"
        "!"
    };

    for (int i = 0;; i++) {
        char* s = strings[i];
        if (s && s[0] == '!')
            break;
        if (s) {
            // printf(">%d %s\n", i, s);
            free(strings[i]);
        }
    }

    memcpy(style, style_bin, sizeof(struct tbx_style));
    strcpy(style->font, "monospace 10");
}