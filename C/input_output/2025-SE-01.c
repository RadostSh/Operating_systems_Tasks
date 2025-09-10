#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

int fd[20];
uint64_t id[20];
uint8_t len[20];
char role[20][256];
char text[UINT8_MAX + 1];

int openw(const char* name, int flag, mode_t mode){
    int fd1 = open(name, flag, mode);
    if(fd1 == -1){
        err(3, "read");
    }
    return fd1;
}

ssize_t readw(int fd1, void* buf, size_t byte){
    ssize_t res = read(fd1, buf, byte);
    if(res == -1){
        err(4, "read");
    }
    return res;
}

ssize_t writew(int fd1, const void* buf, size_t byte){
    ssize_t res = write(fd1, buf, byte);
    if(res == -1){
        err(5, "write");
    }
    return res;
}

int main(int argc, char* argv[]) {
	if(argc < 2 || argc > 21){
        errx(1, "Arg should be max 20 arg");
	}

    for(int i = 0; i < argc - 1; i++){
        fd[i] = openw(argv[i + 1], O_RDONLY, 0);
        
        uint64_t id1;
        readw(fd[i], &id1, sizeof(id1));
        if(id1 != 133742){
            errx(2, "Wrong magic number");
        }
        
        readw(fd[i], &len[i], sizeof(uint8_t));
        
        //memset(role[i], '\0', 256);
        readw(fd[i], role[i], len[i]);
        role[i][len[i]] = '\0';
    }

    for(int i = 0; i < argc - 1; i++){
        readw(fd[i], &id[i], sizeof(id[i]));
        readw(fd[i], &len[i], sizeof(len[i]));
    }

    int count = argc - 1;

    while(count){
        uint16_t min_el = UINT16_MAX;
        int min_idx = -1;

        for(int j = 0; j < argc - 1; j++){
            if(fd[j] != -1){
                if(min_el >= id[j]){
                    min_el = id[j];
                    min_idx = j;
                }
            }
        }
        
        memset(text, '\0', 256);
        readw(fd[min_idx], text, len[min_idx]);

        char buff[1024];
        int lent = snprintf(buff, sizeof(buff), "%s : %s\n", role[min_idx], text);
        writew(1, buff, lent);

        int res = readw(fd[min_idx], &id[min_idx], sizeof(uint64_t));
        if(res == 0){
            count--;
            close(fd[min_idx]);
            fd[min_idx] = -1;
            continue;
        }

        readw(fd[min_idx], &len[min_idx], sizeof(uint8_t));
    }
    return 0;
}