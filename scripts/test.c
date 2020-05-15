#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/stringop.h"

struct style {
#include "h.txt"
};


typedef void parse_func(int argc, char **argv, int *target, char* name);

struct map_entry {
    const char *name;
    void *data;
    parse_func *cmd;
};


void parseValue(int argc, char **argv, int *target, char* name) {
    printf("%s parseValue\n", name);
}

void parseInt(int argc, char **argv, int *target, char* name) {
      *target = strtol(argv[1], NULL, 10);
    printf("%s parseInt %d\n", name, *target);
}

void parseString(int argc, char **argv, int *target, char* name) {
    printf("%s parseString\n", name);
}

void parseColor(int argc, char **argv, int *target, char* name) {
    printf("%s parseColor\n", name);
}

int getPropertyIndex(struct map_entry *e, char *name) {
  for (int i = 0;; i++) {
    struct map_entry *p = &e[i];
    if (p->name == 0) {
      break;
    }
    if (strcmp(p->name, name) == 0) {
      return i;
    }
  }
  return -1;
}

int main(int argc, char **argv) 
{
    printf("struct size: %d\n",sizeof(struct style));
    
    if (argc > 1) {
        printf("%s\n", argv[1]);
    }

    struct style _style;
    struct style *style = &_style;

    struct map_entry entries[] = {
        #include "m.txt"
        {0,0,0}
    };

    FILE *f = fopen(argv[1], "r");

  char *line = NULL;
  size_t line_size = 0;
  size_t nread;
  while (!feof(f)) {
    nread = getline(&line, &line_size, f);
    if (!nread) {
      continue;
    }
    int argc;
    char **argv = split_args(line, &argc);

    if (argc > 0) {
      int idx = getPropertyIndex(entries, argv[0]);
      if (idx == -1) {
        free_argv(argc, argv);
        continue;
      }

      entries[idx].cmd(argc, argv, entries[idx].data, argv[0]);
    }

    free_argv(argc, argv);
  }

  fclose(f);


    return 0;
}