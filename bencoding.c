#include <stdlib.h>
#include <stdio.h>
#include "list.h"

typedef enum bType {
	BNONE = 0,
	BSTRING,
	BINT,
	BLIST,
	BDICT
} bType;

typedef struct Bencoding Bencoding;
typedef struct listNode listNode;
typedef struct dictNode dictNode;

struct listNode {
    Bencoding *value;
    listNode *next;
};

struct dictNode {
    char *key;
    Bencoding *value;
    dictNode *next;
};

struct Bencoding {
    bType type;
    union {
        long long val;
        listNode *list;
        char *str;
        dictNode *dict;
    } value;
};

void readInteger(FILE *f) {
    int i;
    fscanf(f,"%i",i);
    if((fgetc(f)) != 'e')
        exit(-2);
}

void readList(FILE *f) {
    
}

char *readBeconding(FILE *f) {
    char c;
    while(1) {
        c = fgetc(f);
        switch(c) {
            case 'i':
                readInteger(f);
            case 'l':
                readList(f);
            case 'd':
                readDictionary(f);
            default:
                readByteString(f);
        }
    }
}

int main(void) {
    return 1;
}