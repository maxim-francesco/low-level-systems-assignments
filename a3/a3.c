#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#define RESP_PIPE_50150 "RESP_PIPE_50150"
#define REQ_PIPE_50150 "REQ_PIPE_50150"
#define MEM "/ee41TF7M"
#define CHUNGHA 3072

int fd1 = -1, fd2 = -1, shmFd, fd = -1, size;
char req[250];
unsigned char *data = NULL, *sharedChar = NULL;
unsigned int re = 0;

void respond(char *resp, unsigned int res, bool selection) {
    if (selection == true) {
        int sizeOfRespond = strlen(resp);
        write(fd2, resp, sizeOfRespond);
    } else {
        write(fd2, &res, 4);
    }
    return;
}

void request(bool selection, bool selection2) {
    if (selection == true) {
        int i = 0;
        char c = 0;
        while (read(fd1, &c, 1)) {
            req[i++] = c;
            if (c == '#') break;
        }
        req[i] = '\0';
        if (selection2 == true) {
            if (strcmp("EXIT#", req) != 0) {
                respond(req, 0, true);
            }
        }
    } else {
        read(fd1, &re, 4);
    }
    return;
}

void decision(int ok) {
    if (ok == 0) {
        respond("SUCCESS#", 0, true);
    } else {
        respond("ERROR#", 0, true);
    }
    return;
}

unsigned int CALCULEAZA(unsigned int offLOG) {
    unsigned char noOfSections = *(data + 6);
    unsigned int offsets[noOfSections], sizes[noOfSections], sect_offset, sect_size, calc, current_logical_offset = 0;

    for (int i = 0; i < noOfSections; i++) {
        memcpy(&sect_offset, data + 7 + i * 17 + 9, 4);
        memcpy(&sect_size, data + 7 + i * 17 + 13, 4);
        offsets[i] = sect_offset;
        sizes[i] = sect_size;
        if (offLOG >= current_logical_offset && offLOG < current_logical_offset + sizes[i]) {
            calc = offsets[i] + (offLOG - current_logical_offset) ; 
            return calc;
        }
        current_logical_offset += sizes[i];
        current_logical_offset = (1+(current_logical_offset / CHUNGHA) ) * CHUNGHA;
    }

    return 1606;
}

int main(void) {
    if (mkfifo(RESP_PIPE_50150, 0600) != 0) {
        perror("cannot create the response pipe");
        return (1);
    }

    fd1 = open(REQ_PIPE_50150, O_RDONLY);
    fd2 = open(RESP_PIPE_50150, O_WRONLY);

    if (fd2 == -1) {
        perror("cannot open the request pipe");
        return (1);
    }

    respond("CONNECT#", 0, true);

    while (true) {
        request(true, true);
        if (strcmp(req, "PING#") == 0) {
            respond(NULL, 50150, false);
            respond("PONG#", 0, true);
        } else if (strcmp(req, "CREATE_SHM#") == 0) {
            int ok = 0;
            request(false, false);
            shmFd = shm_open(MEM, O_CREAT | O_RDWR, 0600);
            if (shmFd < 0) {
                ok = 1;
            }
            ftruncate(shmFd, re);
            sharedChar = (unsigned char *) mmap(0, re, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
            if (sharedChar == (void *) -1) {
                ok = 1;
            }
            decision(ok);
        } else if (strcmp(req, "WRITE_TO_SHM#") == 0) {
            request(false, false);
            unsigned int offset = re, value, ok = 0;
            request(false, false);
            value = re;
            lseek(shmFd, offset, SEEK_SET);
            if (offset < 0 || offset > 2510542) {
                ok = 1;
            }
            if (write(shmFd, &value, 4) != 4) {
                ok = 1;
            }
            decision(ok);
        } else if (strcmp(req, "MAP_FILE#") == 0) {
            request(true, false);
            req[strlen(req) - 1] = '\0';
            int ok = 0;
            fd = open(req, O_RDONLY);
            if (fd == -1) {
                ok = 1;
            }
            size = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            data = (unsigned char *) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
            if (data == (void *) -1) {
                ok = 1;
            }
            decision(ok);
        } else if (strcmp(req, "READ_FROM_FILE_OFFSET#") == 0) {
            request(false, false);
            unsigned int offset = re, noOfBytes, p = 0, ok = 0;
            request(false, false);
            noOfBytes = re;
            if (sharedChar == NULL || offset + noOfBytes > size || fd == -1) {
                ok = 1;
            }
            for (int q = offset; q <= offset + noOfBytes; q++) {
                *(sharedChar + p++) = *(data + q);
            }
            decision(ok);
        } else if (strcmp(req, "READ_FROM_FILE_SECTION#") == 0) {
            request(false, false);
            unsigned int sectionNo = re, offset, noOfBytes, ok = 0;
            request(false, false);
            offset = re;
            request(false, false);
            noOfBytes = re;
            unsigned char noOfSections = *(data + 6);
            if (sectionNo > noOfSections) {
                decision(1);
            } else {
                int kkk = 7;
                unsigned char section[noOfSections][18];
                for (int k = 0; k < noOfSections; k++) {
                    for (int kk = 0; kk < 17; kk++) {
                        section[k][kk] = *(data + kkk++);
                    }
                    section[k][17] = '\0';
                }
                unsigned int didi = 0;
                memcpy(&didi, *(section + sectionNo - 1) + 9, 4);
                int ii = didi;
                for (int i = 0; i < noOfBytes; i++) {
                    *(sharedChar + i) = *(data + offset + ii++);
                }
                decision(ok);
            }
        } else if (strcmp(req, "READ_FROM_LOGICAL_SPACE_OFFSET#") == 0) {
            request(false, false);
            unsigned int logical_offset = re, noOfBytes, ok = 0;
            request(false, false);
            noOfBytes = re;

            unsigned int francyOFF = CALCULEAZA(logical_offset);
            if (francyOFF ==  -1 || francyOFF + noOfBytes > size || sharedChar == NULL) {
                ok = 1;
            } else {
                memcpy(sharedChar, data + francyOFF, noOfBytes);
            }

            decision(ok);
        } else if (strcmp(req, "EXIT#") == 0) {
            close(fd1);
            close(fd2);
            unlink(RESP_PIPE_50150);
            return (0);
        } else {
            return (0);
        }
    }

    return (0);
}