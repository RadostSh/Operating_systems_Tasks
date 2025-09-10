#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	if(argc != 3){
        errx(1, "arg must be 2");
	}

	int scl = open(argv[1], O_RDONLY);
	if(scl == -1){
        err(2, "Error with open scl file.");
	}

	int sdl = open(argv[2], O_RDONLY);
	if(sdl == -1){
        err(3, "Error with open sdl file.");
	}

	int newFile = open("new_file", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if(newFile == -1){
        err(4, "Error with open new file.");
	}

    int pos = 0;
    uint16_t el;
    int res;
    
    while((res = read(sdl, &el, sizeof(el))) == sizeof(el)){
        int bucket = pos / 8;
        int bucketPos = pos % 8;

        if(lseek(scl, (off_t)bucket, SEEK_SET) == -1){
            err(6, "lseek");
        }

        uint8_t curr;
        if(read(scl, &curr, sizeof(curr)) == -1){
            err(7, "read");
        }

        uint8_t mask = 1;
        mask <<= (7 - bucketPos);

        if(mask & curr){
            if(write(newFile, &el, sizeof(el)) == -1){
                err(8, "write");
            }
        }
        pos++;
    }
    if(res == -1){
        err(5, "read");
    }

    close(scl);
    close(sdl);
    close(newFile);
}