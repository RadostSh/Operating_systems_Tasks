#include <stdint.h>
#include <err.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct{
    uint16_t offset;
    uint8_t oldByte;
    uint8_t newByte;
}patch;

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(2, "error with stat");
    }
    return st.st_size;
}

int open_wrapper(const char* name, int flags, mode_t mode){
    int fd = open(name, flags, mode);
    if (fd == -1){
        err(3, "open");
    }

    return fd;
}

int main(int argc, char* argv[]) {
	if (argc != 4){
        errx(1, "Arg must be 3");
	}

    int f1 = open_wrapper(argv[1], O_RDONLY, 0);
    int f2 = open_wrapper(argv[2], O_RDONLY, 0);
    int fdpatch = open_wrapper(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if(file_size(f1) != file_size(f2)){
        errx(6, "file size");
    }

    int res;
    
    uint8_t ch1;
    uint8_t ch2;

    while ((res = read(f1, &ch1, sizeof(ch1))) == sizeof(ch1)){
        if(read(f2, &ch2, sizeof(ch2)) != sizeof(ch2)){
            err(6, "read");
        }

        if(ch1 != ch2){
            patch patch1;
            int offset;

            if((offset = lseek(f1, 0, SEEK_CUR)) == -1){
                err(7, "lseek");
            }

            patch1.offset = offset - 1;
            patch1.oldByte = ch1;
            patch1.newByte = ch2;
            
            if(write(fdpatch, &patch1, sizeof(patch1)) != sizeof(patch1)){
                err(8, "write");
            }
        }
    }

    if(res != 0){
        err(5, "error with read");
    }

    close(f1);
    close(f2);
    close(fdpatch);
}