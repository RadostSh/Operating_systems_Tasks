#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>

int openw(const char* name, int flag, mode_t mode){
    int fd = open(name, flag, mode);
    if(fd == -1){
        err(10, "open");
    }
    return fd;
}

uint64_t file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(11, "fstat");
    }
    return st.st_size;
}

void handle(uint8_t byte, uint8_t oldBit, uint8_t N, int compFd, int originFd){
        if(oldBit == 1){
            N = byte ^ (1 << 7);
            uint8_t byte1;
                if(read(compFd, &byte1, sizeof(byte1)) != sizeof(byte1)){
                    err(7, "read");
                }
            for(int i = 0; i <= N; i++){
                 if(write(originFd, &byte1, sizeof(byte1)) != sizeof(byte1)){
                    err(9, "read");
                }   
            }
        } else if(oldBit == 0){
            N = byte;
            for(int i = 0; i <= N; i++){
                uint8_t byte1;
                if(read(compFd, &byte1, sizeof(byte1)) != sizeof(byte1)){
                    err(7, "read");
                }
                if(write(originFd, &byte1, sizeof(byte1)) != sizeof(byte1)){
                    err(9, "read");
                }
            }
        } else {
            errx(15, "Logic error");
        }
}

int main(int argc, char* argv[]) {
	if(argc != 3){
        errx(1, "Arguments must be 2");
	}

	int compFd = openw(argv[1], O_RDONLY, 0);
	int originFd = openw(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

	uint32_t magic;
	if(read(compFd, &magic, sizeof(magic)) != sizeof(magic)){
        err(2, "read");
	}

	if(magic != 0x21494D46){
        errx(3, "Wrong magic number");
	}

	uint32_t packet_count;
	if(read(compFd, &packet_count, sizeof(packet_count)) != sizeof(packet_count)){
        err(4, "read");
	}

	uint64_t original_size;
	if(read(compFd, &original_size, sizeof(original_size)) != sizeof(original_size)){
        err(5, "read");
	}

	for(uint32_t i = 0; i < packet_count; i++){
        uint8_t byte;
        if(read(compFd, &byte, sizeof(byte)) != sizeof(byte)){
            err(6, "read");
        }
        
        uint8_t mask = 1;
        uint8_t oldBit = (byte >> 7) & mask;
        uint8_t N = 0;
        // uint8_t N = ((uint8_t)(~(1 << 7))) & byte;

        handle(byte, oldBit, N, compFd, originFd);
	}

	if(file_size(originFd) != original_size){
        errx(9, "origin file not a full size");
	}

	close(compFd);
	close(originFd);
}