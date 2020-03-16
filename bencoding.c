#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Bencoding Bencoding;
typedef struct listNode listNode;
typedef struct dictNode dictNode;

Bencoding *readInteger(FILE *f);
Bencoding *readList(FILE *f);
Bencoding *readDictionary(FILE *f);
Bencoding *readByteString(FILE *f);
Bencoding *readBeconding(FILE *f);
char *readString(FILE *f);
void printBencodeList(listNode *value);
void printBencodeDict(dictNode *value);
void exitWithError(int i, char *reason);

typedef enum bType {
	BNONE = 0,
	BSTRING,
	BINT,
	BLIST,
	BDICT
} bType;

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

listNode *head;

Bencoding *readInteger(FILE *f) {
    Bencoding *value = (Bencoding *)malloc(sizeof(Bencoding));
    value->type = BINT;
    fscanf(f,"%ld",value->value.val);
    if((fgetc(f)) != 'e')
        exit(-2);
    return value;
}

Bencoding *readList(FILE *f) {
    char c;
    Bencoding *value = (Bencoding *)malloc(sizeof(Bencoding));
    value->type = BLIST;
    value->value.list = (listNode *)malloc(sizeof(listNode));
    listNode *list = value->value.list;
    listNode *list_prev;
    do {
        list->value = readBeconding(f);
        list->next = (listNode *)malloc(sizeof(listNode));
        list_prev = list;
        list = list->next;
    } while((c = fgetc(f)) != 'e');
    list_prev->next = NULL;
    free(list);
    return value;
}

Bencoding *readDictionary(FILE *f) {
    char c;
    Bencoding *value = (Bencoding *)malloc(sizeof(Bencoding));
    value->type = BDICT;
    value->value.dict = (dictNode *)malloc(sizeof(dictNode));
    dictNode *dict = value->value.dict;
    dictNode *dict_prev;
    do {
        dict->key = readString(f);
        dict->value = readBeconding(f);
        dict->next = (dictNode *)malloc(sizeof(dictNode));
        dict_prev = dict;
        dict = dict->next;
    } while((c = fgetc(f)) != 'e');
    dict_prev->next = NULL;
    free(dict);
    return value;
}

char *readString(FILE *f) {
    long long len;
    char *str;
    fscanf(f,"%ld",len);
    str = (char *)malloc(len+1);
    char *readFormat;
    if(len==0)
        exitWithError(-1,"Invalid Bencoding. length 0");
    if((fgetc(f)) != ':')
        exitWithError(-1,"Invalid Bencoding. Expected : after length");
    sprintf(readFormat, "\%%d", len);
    fscanf(f, readFormat, str);
    return str;
}

Bencoding *readByteString(FILE *f) {
    Bencoding *value = (Bencoding *)malloc(sizeof(Bencoding));
    value->type = BSTRING;
    long long len;
    fscanf(f,"%ld",len);
    value->value.str = readString(f);
    return value;
}

Bencoding *readBeconding(FILE *f) {
    char c = fgetc(f);
    switch(c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            ungetc(c,f);
            return readByteString(f);
            break;
        case 'i':
            return readInteger(f);
        case 'l':
            return readList(f);
        case 'd':
            return readDictionary(f);
        default:
            exitWithError(-1,"Invalid bencoding");
    }
}

void exitWithError(int i, char *reason) {
    fprintf(stderr,reason);
    exit(-1);
}

FILE *openFile(char *fileName, char* mode) {
    FILE *f = fopen(fileName, mode);
    if(f != NULL)
        return f;
    exitWithError(-1, "Could not open file with name");
}

void initializeHead() {
    head = (listNode *)malloc(sizeof(listNode));
}

void printBencodeString(Bencoding *value) {
    printf("String: %s\n", value->value.str);
}

void printBencodeInt(Bencoding *value) {
    printf("Int: %d\n", value->value.val);
}

void printBencodeDict(dictNode *value) {
    printf("Dict: \n\n");
    dictNode *curr = value;
    Bencoding *bValue;
    printf("%s: ", curr->key);
    do {
        bValue = curr->value;
        switch(bValue->type) {
            case BSTRING:
                printBencodeString(bValue);
                break;
            case BINT:
                printBencodeInt(bValue);
                break;
            case BLIST:
                printBencodeList(bValue->value.list);
                break;
            case BDICT:
                printBencodeDict(bValue->value.dict);
                break;
        }
    } while(curr != NULL);
}

void printBencodeList(listNode *value) {
    printf("List: \n\n");
    listNode *curr = value;
    Bencoding *bValue;
    do {
        bValue = curr->value;
        switch(bValue->type) {
            case BSTRING:
                printBencodeString(bValue);
                break;
            case BINT:
                printBencodeInt(bValue);
                break;
            case BLIST:
                printBencodeList(bValue->value.list);
                break;
            case BDICT:
                printBencodeDict(bValue->value.dict);
                break;
        }
    } while(curr != NULL);
}

void readFile(FILE *f) {
    listNode *next = head;
    listNode *next_prev;
    while(!feof(f)) {
        next->value = readBeconding(f);
        next_prev = next;
        next->next = (listNode *)malloc(sizeof(listNode));
        next = next->next;
    }
    printBencodeList(head);
}

int main(int argc, char **argv) {
    initializeHead();
    if(argc < 3) {
        exitWithError(-1, "Incorrect Usage.\nUsage: .\\prog <fileName> <r/w>");
    }
    FILE *f = openFile(argv[1],argv[2]);
    if(strcmp(argv[2],"r") == 0)
        readFile(f);
    fclose(f);
    return 1;
}