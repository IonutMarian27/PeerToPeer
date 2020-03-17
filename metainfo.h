#ifndef METAINFO_H
#define METAINFO_H
#include "bencoding.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Metainfo Metainfo;
typedef struct infoDictionary infoDictionary;

Bencoding *getMetainfoFromFile(FILE *f) {
    Bencoding *curr;
    //Dictionary
    return readBeconding(f);
}
#endif