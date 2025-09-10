#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <stdint.h>
#include <unistd.h>

typedef struct{
    uint16_t offset;
    uint8_t lenght;
    uint8_t notUsed;
} idx;

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(2, "error with stat");
    }

    return st.st_size;
}

int open_wrapper(const char* file, int flags, mode_t mode){
    int fd = open(file, flags, mode);
    if(fd == -1){
        err(4, "Error with open %s", file);
    }

    return fd;
}

int main(int argc, char* argv[]) {
	if (argc != 5){
        errx(1, "Arguments must be 4");
	}

    int fdF1Dat = open_wrapper(argv[1], O_RDONLY, 0);
    int fdF1Idx = open_wrapper(argv[3], O_RDONLY, 0);
    int fdF2Dat = open_wrapper(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    int fdF2Idx = open_wrapper(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0666);

    idx idx1;

    if(file_size(fdF1Idx) % sizeof(idx1) != 0){
        errx(5, "File size is not multiple of record size");
    }

    int res;

    while((res = read(fdF1Idx, &idx1, sizeof(idx1))) == sizeof(idx1)){
        if(idx1.offset > file_size(fdF1Dat)){
            errx(6, "Invalid offset");
        }

        if(lseek(fdF1Dat, idx1.offset, SEEK_SET) == -1){
            err(7, "seek");
        }

        uint8_t searchedStr[256];
        int readRes = read(fdF1Dat, &searchedStr, idx1.lenght);
        if(readRes == -1){
            err(8, "read");
        } else if(readRes != idx1.lenght){
            errx(9, "EOF");
        }
        
        if(searchedStr[0] >= 'A' && searchedStr[0] <= 'Z'){
            idx idx2;

            idx2.lenght = idx1.lenght;

            int offset;
            if((offset = lseek(fdF2Dat, 0, SEEK_CUR)) == -1){
                err(10, "lseek");
            }

            idx2.offset = offset;

            if(write(fdF2Dat, &searchedStr, idx2.lenght) != idx2.lenght){
                err(11, "s");
            }
            
            if(write(fdF2Idx, &idx2, sizeof(idx2)) != sizeof(idx2)){
                err(12, "e");
            }
        }
    }

    if (res != 0){
        err(13, "f");
    }

    close(fdF1Dat);
    close(fdF1Idx);
    close(fdF2Dat);
    close(fdF2Idx);
}