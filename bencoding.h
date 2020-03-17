#ifndef BENCODING_H
#define BENCODING_H
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
void writeBencodeList(listNode *value, FILE *f);
void printBencodeList(listNode *value);
void writeBencodeDict(dictNode *value, FILE *f);
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
    if(value == NULL)
        exitWithError(-90,"Not enough memory");
    value->type = BINT;
    fscanf(f,"%Ld",&(value->value.val));
    if((fgetc(f)) != 'e')
        exit(-2);
    return value;
}

Bencoding *readList(FILE *f) {
    char c = 0;
    Bencoding *value = (Bencoding *)malloc(sizeof(Bencoding));
    if(value == NULL)
        exitWithError(-90,"Not enough memory");
    value->type = BLIST;
    value->value.list = (listNode *)malloc(sizeof(listNode));
    if(value->value.list == NULL)
        exitWithError(-90,"Not enough memory");
    listNode *list = value->value.list;
    listNode *list_prev;
    do {
        if(c != 0)
            ungetc(c,f);
        list->value = readBeconding(f);
        list->next = (listNode *)malloc(sizeof(listNode));
        list_prev = list;
        list = list->next;
    } while((c = fgetc(f)) != 'e' && (feof(f) == 0));
    list_prev->next = NULL;
    free(list);
    return value;
}

Bencoding *readDictionary(FILE *f) {
    char c = 0;
    Bencoding *value = (Bencoding *)malloc(sizeof(Bencoding));
    if(value == NULL)
        exitWithError(-90,"Not enough memory");
    value->type = BDICT;
    value->value.dict = (dictNode *)malloc(sizeof(dictNode));
    if(value->value.dict == NULL)
        exitWithError(-90,"Not enough memory");
    dictNode *dict = value->value.dict;
    dictNode *dict_prev;
    do {
        if(c != 0)
            ungetc(c,f);
        dict->key = readString(f);
        dict->value = readBeconding(f);
        dict->next = (dictNode *)malloc(sizeof(dictNode));
        if(dict->next == NULL)
            exitWithError(-90,"Not enough memory");
        dict_prev = dict;
        dict = dict->next;
    } while((c = fgetc(f)) != 'e' && (feof(f) == 0));
    dict_prev->next = NULL;
    free(dict);
    return value;
}

char *readString(FILE *f) {
    char c;
    long long len = 0;
    char *str;
    fscanf(f,"%Ld",&len);
    printf("%d\n",len);
    str = (char *)malloc(len+1);
    if(str == NULL)
        exitWithError(-90,"Not enough memory");
    if(len==0) {
        c = fgetc(f);
        if(feof(f))
            return NULL;
        exitWithError(-1,"Invalid Bencoding. length 0");
    }
    c = fgetc(f);
    if(c != ':')
        exitWithError(-1,"Invalid Bencoding. Expected : after length");
    fread(str,sizeof(char),len,f);
    str[len]='\0';
    return str;
}

Bencoding *readByteString(FILE *f) {
    Bencoding *value = (Bencoding *)malloc(sizeof(Bencoding));
    if(value == NULL)
        exitWithError(-90,"Not enough memory");
    value->type = BSTRING;
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
            if(feof(f))
                return NULL;
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
    if(head == NULL)
        exitWithError(-90,"Not enough memory");
}

void writeBencodeString(Bencoding *value, FILE *f) {
    fprintf(f,"%d:%s", strlen(value->value.str), value->value.str);
    //fwrite(value->value.str,sizeof(char),strlen(value->value.str),f);
}

void printBencodeString(Bencoding *value) {
    printf("String: %s\n", value->value.str);
}

void writeBencodeInt(Bencoding *value, FILE *f) {
    fprintf(f,"i%Lde",value->value.val);
}

void printBencodeInt(Bencoding *value) {
    printf("Int: %d\n", value->value.val);
}

void writeBencodeDict(dictNode *value, FILE *f) {
    fprintf(f,"d");
    dictNode *curr = value;
    Bencoding *bValue;
    do {
        fprintf(f,"%d:%s", strlen(curr->key),curr->key);
        bValue = curr->value;
        switch(bValue->type) {
            case BSTRING:
                writeBencodeString(bValue, f);
                break;
            case BINT:
                writeBencodeInt(bValue, f);
                break;
            case BLIST:
                writeBencodeList(bValue->value.list, f);
                break;
            case BDICT:
                writeBencodeDict(bValue->value.dict, f);
                break;
        }
        curr = curr->next;
    } while(curr != NULL);
    fprintf(f,"e");
}

void printBencodeDict(dictNode *value) {
    printf("Dict: \n\n");
    dictNode *curr = value;
    Bencoding *bValue;
    do {
        printf("%s: ", curr->key);
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
        curr = curr->next;
    } while(curr != NULL);
}

void writeBencodeList(listNode *value, FILE *f) {
    fprintf(f,"l");
    listNode *curr = value;
    Bencoding *bValue;
    do {
        bValue = curr->value;
        switch(bValue->type) {
            case BSTRING:
                writeBencodeString(bValue, f);
                break;
            case BINT:
                writeBencodeInt(bValue, f);
                break;
            case BLIST:
                writeBencodeList(bValue->value.list, f);
                break;
            case BDICT:
                writeBencodeDict(bValue->value.dict, f);
                break;
        }
        curr = curr->next;
    } while(curr != NULL);
    fprintf(f,"e");
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
        curr = curr->next;
    } while(curr != NULL);
}

void readFile(FILE *f) {
    listNode *next = head;
    listNode *next_prev;
    while(!feof(f)) {
        next->value = readBeconding(f);
        next_prev = next;
        next->next = (listNode *)malloc(sizeof(listNode));
        if(next->next == NULL)
            exitWithError(-90,"Not enough memory");
        next = next->next;
    }
    printBencodeList(head);
}
#endif