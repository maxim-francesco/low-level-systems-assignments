#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>

#define VARIANT 50150

int listOptions(int argc, char **argv, char *path, int *size, char *name){
    int k = 0;
    for(int i = 1; i < argc; i++){
        if(strstr(argv[i], "path=") != NULL){
            sscanf(argv[i], "path=%s", path);
        }
        if(strcmp(argv[i], "recursive") == 0){
            k += 100;
        }
        if(strstr(argv[i],"size_smaller=") != NULL){
            sscanf(argv[i], "size_smaller=%d", size);
            k += 10;
        }
        if(strstr(argv[i], "name_starts_with=") != NULL){
            sscanf(argv[i], "name_starts_with=%s", name);
            k += 1;
        }
    }
    return k;
}

DIR* openDir(const char *path){
    DIR *dir = NULL;
    dir = opendir(path);
    return dir;
}

int stringStart(const char *s1,const char *s2){
    for(size_t i = 0; i < strlen(s2); i++){
        if(s1[i] != s2[i]){
            return 0;
        }
    }
    return 1;
}

void list(const char *path, int i, int *k, char *name, int size){
    DIR *dir = openDir(path);
    if(dir == NULL){
        printf("ERROR\n");
    }else{
        if((*k) == 0){
            printf("SUCCESS\n");
            (*k)++;
        }
        struct dirent *entry = NULL;
        struct stat stat_buffer;
        char filePath[512];
        int ok = 0;
        while((entry = readdir(dir)) != NULL){
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
                snprintf(filePath, 512, "%s/%s", path, entry->d_name);
                if(i % 10 == 1){
                    ok = 1;
                    if(stringStart(entry->d_name, name) == 1){
                        printf("%s\n", filePath);
                    }
                }
                if((i / 10) % 10 == 1){
                    ok = 1;
                    if(lstat(filePath, &stat_buffer) == 0){
                        if(stat_buffer.st_size <= size){
                            if(!S_ISLNK(stat_buffer.st_mode) && !S_ISDIR(stat_buffer.st_mode)){
                                printf("%s\n", filePath);
                            }
                        }
                    }
                }
                if((i / 100) % 10 == 1){
                    if(lstat(filePath, &stat_buffer) == 0){
                        if(S_ISDIR(stat_buffer.st_mode)){
                            list(filePath, i, k, name, size);
                        }
                    }
                }
                if(ok == 0){
                    printf("%s\n", filePath);
                }
            }
        }
        free(entry);
    }
    closedir(dir);
}

void forParse(int argc, char **argv, char *path){
    for(int i = 1; i < argc; i++){
        if(strstr(argv[i], "path=") != 0){
            sscanf(argv[i], "path=%s", path);
        }
    }
}

int openFile(const char *path){
    int fd = -1;
    fd = open(path, O_RDONLY);
    return fd;
}

typedef struct{
    unsigned char sectName[9];
    unsigned char sectType;
    int sectOffset;
    int sectSize;
}subHeader;

typedef struct{
    unsigned char magic[3];
    unsigned char headerSize[3];
    unsigned char version[3];
    unsigned char noOfSections;
    subHeader** sections;
}headerFile;

void printSubStruct(int n, subHeader **headerFile){
    for(int i = 0; i < n; i++){
        printf("SECT NR:%d\n", i);
        printf("SECT_NAME: %s\n", headerFile[i]->sectName);
        printf("SECT_TYPE: %c\n", headerFile[i]->sectType);
        printf("SECT_OFFSET: %d\n", headerFile[i]->sectOffset);
        printf("SECT_SIZE: %d\n", headerFile[i]->sectSize);
    }
}

void printStruct(headerFile *myFile){
    printf("MAGIC: %s\n", myFile->magic);
    printf("HEADER SIZE: %s\n", myFile->headerSize);
    printf("VERSION: %s\n", myFile->version);
    printf("NO OF SECT: %c\n", myFile->noOfSections);
    printSubStruct((int)myFile->noOfSections, myFile->sections);
}

int valueOfString(unsigned char *s1){
    int value = 0;
    for(size_t i = 0; i < strlen((char*)s1); i++){
        value += (int)s1[i];
    }
    return value;
}

void printForFunction(headerFile *myFile){
    printf("version=%d\n", valueOfString(myFile->version));
    printf("nr_sections=%d\n", (int)myFile->noOfSections);
    for(int i = 0; i < (int)myFile->noOfSections; i++){
        printf("section%d: %s %d %d\n", i + 1, myFile->sections[i]->sectName, myFile->sections[i]->sectType, myFile->sections[i]->sectSize);
    }
}

int parse(const char *path, headerFile* myFile, char *message){
    int fd = openFile(path);
    if(fd == -1){
        return -1;
    }else{
        int value = 0;
        // magic
        read(fd, myFile->magic, 2); myFile->magic[2] = '\0';
        if(strcmp((char*)myFile->magic, "q8") != 0){
            snprintf(message, 20, "%s", "wrong magic");
            free(myFile);
            close(fd);
            return -1;
        }
        // header size
        read(fd, myFile->headerSize, 2); myFile->headerSize[2] = '\0';
        // version
        read(fd, myFile->version, 2); myFile->version[2] = '\0';
        value = valueOfString(myFile->version);
        if(value < 102 || value > 207){
            snprintf(message, 20, "%s", "wrong version");
            free(myFile);
            close(fd);
            return -1;
        }
        // noOfSections
        read(fd, &myFile->noOfSections, 1);
        value = (int)myFile->noOfSections;
        if(value < 6 || value > 16){
            if(value != 2){
                snprintf(message, 20, "%s", "wrong sect_nr");
                free(myFile);
                close(fd);
                return -1;
            }
        }
        // section headers
        myFile->sections = (subHeader**)malloc(sizeof(subHeader*) * (int)myFile->noOfSections);
        for(int k = 0; k < (int)myFile->noOfSections; k++){
            myFile->sections[k] = (subHeader*)malloc(sizeof(subHeader) * (int)myFile->noOfSections);
            // sectName
            read(fd, myFile->sections[k]->sectName, 8); myFile->sections[k]->sectName[8] = '\0';
            // sectType
            read(fd, &myFile->sections[k]->sectType, 1);
            value = (int)myFile->sections[k]->sectType;
            if(value != 14 && value != 77 && value != 84 && value != 27 && value != 59){
                snprintf(message, 20, "%s", "wrong sect_types");
                close(fd);
                int y = k;
                while(y >= 0){
                    free(myFile->sections[y--]);
                }
                free(myFile->sections);
                free(myFile);
                return -1;
            }
            // sectOffset
            read(fd, &myFile->sections[k]->sectOffset, 4); 
            // sectSize
            read(fd, &myFile->sections[k]->sectSize, 4);
        }
    }
    close(fd);
    return 0;
}

void extractArguments(char **argv, char *path, int *nr, int *line_nr){
    for(int i = 2; i < 5; i++){
        if(strstr(argv[i], "path=") != 0){
            sscanf(argv[i], "path=%s", path);
        }
        if(strstr(argv[i], "section=") != 0){
            sscanf(argv[i], "section=%d", nr);
        }
        if(strstr(argv[i], "line=") != 0){
            sscanf(argv[i], "line=%d", line_nr);
        }
    }
}

int extractFunction(const char *path, int nr, int line_nr, char *line){
    headerFile* myFile = (headerFile*)malloc(sizeof(headerFile));
    char message[50];
    int parseStatus = parse(path, myFile, message);
    if(parseStatus == -1){
        printf("ERROR");
        free(myFile);
        return -1;
    }

    if((int)myFile->noOfSections < nr){
        printf("ERROR");
        free(myFile);
        return -1;
    }

    int fd = openFile(path);
    ssize_t offset = myFile->sections[nr - 1]->sectOffset;
    lseek(fd, offset, SEEK_SET);
    char c;
    int linie = 1;
    printf("SUCCESS\n");
    while(read(fd, &c, 1) == 1){
        if(c == 0){
            break;
        }
        if(linie == line_nr){
            printf("%c", c);
            if(c == '\n'){
                break;
            }
        }
        if(c == '\n'){
            linie++;
        }
    }
    while(myFile->noOfSections > 0){
        free(myFile->sections[--myFile->noOfSections]);
    }
    free(myFile->sections);
    free(myFile);
    close(fd);
    return 0;
}

int findall(const char *path, int *k){
    DIR *dir = openDir(path);
    if(dir == NULL){
        printf("ERROR\n");
        closedir(dir);
        return -1;
    }else{
        if((*k) == 0){
            printf("SUCCESS\n");
            (*k)++;
        }
        struct dirent *entry = NULL;
        struct stat stat_buffer;
        char filePath[512];
        while((entry = readdir(dir)) != NULL){
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
                snprintf(filePath, 512, "%s/%s", path, entry->d_name);
                if(lstat(filePath, &stat_buffer) == 0){
                    if(S_ISDIR(stat_buffer.st_mode)){
                        findall(filePath, k);
                    }else{
                        headerFile* myFile = (headerFile*)malloc(sizeof(headerFile));
                        char message[50];
                        int status = parse(filePath, myFile, message);
                        if(status == 0){
                            int w = 0;
                            for(int q = 0; q < (int)myFile->noOfSections; q++){
                                if(myFile->sections[q]->sectSize > 1273){
                                    w = 1;
                                    break;
                                }
                            }
                            if(w == 0){
                                printf("%s\n", filePath);
                            }
                            while(myFile->noOfSections > 0){
                                free(myFile->sections[--myFile->noOfSections]);
                            }
                            free(myFile->sections);
                            free(myFile);
                        }
                    }
                }

            }
        }
    }
    closedir(dir);
    return 0;
}

int main(int argc, char** argv){
    if(argc == 2){
        if(strcmp(argv[1], "variant") == 0){
            printf("%d\n", VARIANT);
        }
    }else{
        // ./a1 list [recursive] [filt=ceva] path=ceva
        if(strcmp(argv[1], "list") == 0){
            char filePath[512];
            int size;
            char name[512];
            int i = listOptions(argc, argv, filePath, &size, name);
            int k = 0;
            list(filePath, i, &k, name, size);
        }else{
            if(strcmp(argv[1], "parse") == 0 || strcmp(argv[2], "parse") == 0){
                char filePath[512];
                forParse(argc, argv, filePath);
                headerFile* myFile = (headerFile*)malloc(sizeof(headerFile));
                char message[50];
                int statusParse = parse(filePath, myFile, message);
                if(statusParse == -1){
                    printf("ERROR\n%s",message);
                }else{
                    printf("SUCCESS\n");
                    printForFunction(myFile);
                    while(myFile->noOfSections > 0){
                        free(myFile->sections[--myFile->noOfSections]);
                    }
                    free(myFile->sections);
                    free(myFile);
                }
            }else{
                if(strcmp(argv[1], "extract") == 0){
                    int nr = 0;
                    int line_nr = 0;
                    char filePath[512];
                    extractArguments(argv, filePath, &nr, &line_nr);
                    char extractedString[2000];
                    extractFunction(filePath, nr, line_nr, extractedString);
                    
                }else{
                    if(strcmp(argv[1], "findall") == 0){
                        char filePath[512];
                        sscanf(argv[2], "path=%s", filePath);
                        int k = 0;
                        findall(filePath, &k);
                    }
                }
            }
        }
    }
}