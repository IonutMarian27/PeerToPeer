#include "bencoding.h"
#include "metainfo.h"
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

void displayHelp(void) {
    printf("This tool generates a .torrent for given files\n");
    printf("Parameters: \n");
    printf("tracker <tracker_address> <filename/directory>\n");
    /* printf("Optional Parameters are put before tracker\n");
    printf("Used as: <optional_param> <value>\n");
    printf("comm <comment>\n");
    printf("author <author_name>\n");
    printf("md5sum\t <- Includes md5sum in file description\n");
     */exit(1);
}

int is_file(char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int is_directory(char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

char *getPiecesOfFile(char *filePath) {
    FILE *f = fopen(filePath, "rb");
    struct stat st;
    stat(filePath, &st);
    unsigned char *values = (unsigned char *)calloc(5120000,sizeof(char));
    //unsigned char values2[512000];
    char *shaResult = (char *)calloc(20*(st.st_size/(512*1000)+1)+1,sizeof(char));
    unsigned char partial[SHA_DIGEST_LENGTH];
    long i = 0;
    if(f != NULL) {
        do {
            long length = fread(values,sizeof(unsigned char),512*1000,f);
            if(length != 0) {
                //memcpy(values2,values,length);
                SHA1(values,length,partial);
                memcpy(shaResult+i*20,partial,20);
                i++;
            }
        } while(feof(f) == 0);
    } else {
        exitWithError(-1,"Could not open file");
    }
    free(values);
    fclose(f);
    //free(partial);
    return shaResult;
}

void writeFileMetainfo(char *filePath, Bencoding *dest) {
    dest->type = BDICT;
    dest->value.dict = (dictNode *)malloc(sizeof(dictNode));
    dictNode *currDict = dest->value.dict;
    currDict->key = "name";
    currDict->value = (Bencoding *)malloc(sizeof(Bencoding));
    currDict->value->type = BSTRING;
    struct stat st;
    stat(filePath, &st);
    currDict->value->value.str = filePath;
    currDict->next = (dictNode *)malloc(sizeof(dictNode));
    currDict = currDict->next;
    currDict->key = "length";
    currDict->value = (Bencoding *)malloc(sizeof(Bencoding));
    currDict->value->type = BINT;
    currDict->value->value.val = st.st_size;
    currDict->next = (dictNode *)malloc(sizeof(dictNode));
    currDict = currDict->next;
    currDict->key = "piece length";
    currDict->value = (Bencoding *)malloc(sizeof(Bencoding));
    currDict->value->type = BINT;
    currDict->value->value.val = 512*1000;
    currDict->next = (dictNode *)malloc(sizeof(dictNode));
    currDict = currDict->next;
    currDict->key = "pieces";
    currDict->value = (Bencoding *)malloc(sizeof(Bencoding));
    currDict->value->type = BSTRING;
    currDict->value->value.str = getPiecesOfFile(filePath);
    currDict->next = NULL;
}

int main(int argc, char **argv) {
    Bencoding *head;
    initializeHead();
    if(argc < 4) {
        if(argc == 2 && strcmp(argv[1],"help") == 0)
            displayHelp();
        char *str = (char *)malloc(strlen(argv[0])+21);
        sprintf(str,"See help using %s help",argv[0]);
        exitWithError(-1, str);
    }
    Bencoding *info = (Bencoding *)malloc(sizeof(Bencoding));
    info->type = BDICT;
    info->value.dict = (dictNode *)malloc(sizeof(dictNode));
    dictNode *dictHead = info->value.dict, *curr;
    if(strcmp(argv[1],"tracker") == 0) {
        dictHead->key = "announce";
        dictHead->value = (Bencoding *)malloc(sizeof(Bencoding));
        dictHead->value->type = BSTRING;
        dictHead->value->value.str = argv[2];
        dictHead->next = (dictNode *)malloc(sizeof(dictNode));
        curr = dictHead->next;
        if(is_file(argv[3])) {
            curr->key = "info";
            curr->value = (Bencoding *)malloc(sizeof(Bencoding));
            curr->next = NULL;
            writeFileMetainfo(argv[3], curr->value);
            char *targ = (char *)calloc(strlen(argv[3])+20,sizeof(char));
            sprintf(targ,"%s.torrent",argv[3]);
            FILE *newf = openFile(targ,"wb");
            writeBencodeDict(dictHead,newf);
            fclose(newf);
        } else if(is_directory(argv[3])) {
            DIR *dir;
            struct dirent *ent;
            if ((dir = opendir (argv[3])) != NULL) {
                /* print all the files and directories within directory */
                while ((ent = readdir (dir)) != NULL) {
                    printf ("%s\n", ent->d_name);
                }
                closedir (dir);
            } else {
                exitWithError(-1,"Could not open dir");
            }
        }
    } else {
        exitWithError(-1, "Missing -tracker");
    }
    return 1;
} 