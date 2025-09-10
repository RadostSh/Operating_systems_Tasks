#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>
#include <stdint.h>
#include <unistd.h>

int openFile(const char* name, int flags, mode_t mode) {
    int fd = open(name, flags, mode);
    if(fd < 0) {
        err(2, "error open %s", name);
    }
    return fd;
}

int file_size(int fd) {
    struct stat file_stat;

    if(fstat(fd, &file_stat)) {
        err(3, "error with stat");
    }

    return file_stat.st_size;
}

int main(int argc, char* argv[]) {
	if(argc != 3) {
        errx(1, "provide 2 arguments");
	}

    int fd1 = openFile(argv[1], O_RDONLY, 0);
    int fd2 = openFile(argv[2], O_RDONLY, 0);
    int fd3 = openFile("f3", O_WRONLY | O_CREAT | O_TRUNC, 0666);

    int size1 = file_size(fd1);
    int size2 = file_size(fd2);

    if (size1 % 8 != 0 || size2 % 4 != 0){
        errx(5, "error with size of files");
    }

    uint32_t pair[2];
    uint32_t num;

    int res;

    while((res = read(fd1, &pair, sizeof(pair))) == sizeof(pair)){
        if(pair[0]*sizeof(uint32_t) > (uint32_t)size2){
            errx(6, "Invalid content");
        }

        if(lseek(fd2, pair[0]*sizeof(uint32_t), SEEK_SET) == -1){
            err(5, "seek");
        }

        for(uint32_t i = 0; i < pair[1]; i++){
            int readRes = read(fd2, &num, sizeof(num));
            if (readRes == 0){
                errx(5, "Unexpedted EOF");
            } else if (readRes == -1){
                err(6, "read");
            }

            if(write(fd3, &num, sizeof(num)) != sizeof(num)){
                err(6, "write");
            }
        }
    }

    if(res != 0){
        err(7, "read");
    }

    close(fd1);
    close(fd2);
    close(fd3);
}