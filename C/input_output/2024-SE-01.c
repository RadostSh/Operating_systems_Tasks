#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <err.h>
#include <stdlib.h>

typedef struct{
    uint64_t next;
    uint8_t user_data[504];
} node;

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(1, "fstat");
    }
    return st.st_size;
}

ssize_t read_wrapper(int fd, void* buff, ssize_t bytes){
    ssize_t res = read(fd, buff, bytes);
    if(res == -1){
        err(2, "read");
    }
    return res;
}

ssize_t write_wrapper(int fd, void* buff, size_t bytes){
    ssize_t res = write(fd, buff, bytes);
    if(res == -1){
        err(3, "write");
    }
    return res;
}

off_t lseek_wrapper(int fd, off_t offset, int whence){
    off_t res = lseek(fd, offset, whence);
    if(res == -1){
        err(4, "lseek");
    }

    return res;
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
	    errx(1, "args count");
    }

    int inputFd = open(argv[1], O_RDWR);
    if(inputFd == -1){
        err(6, "errro");
    }

    node nd;

    if(file_size(inputFd) % sizeof(nd) != 0){
        errx(7, "file size");
    }

    char tempName[] = "tempXXXXXX";
    int tempFd = mkstemp(tempName);

    uint64_t curr = 0;

    do{
        lseek_wrapper(inputFd, curr*sizeof(nd), SEEK_SET);
        if(read_wrapper(inputFd, &nd, sizeof(nd)) != sizeof(nd)){
            errx(10, "Unexpected EOF");
        }

        lseek_wrapper(tempFd, curr*sizeof(nd), SEEK_SET);
        write_wrapper(tempFd, &nd, sizeof(nd));
        curr = nd.next;
    } while(curr != 0);

    uint8_t zero = '\0';

    lseek_wrapper(inputFd, 0, SEEK_SET);

    int inputFileSize = file_size(inputFd);
    for(int i = 0; i < inputFileSize; i++){
        write_wrapper(inputFd, &zero, sizeof(zero));
    }

    lseek_wrapper(inputFd, 0, SEEK_SET);
    lseek_wrapper(tempFd, 0, SEEK_SET);

    uint8_t byte;
    while(read_wrapper(tempFd, &byte, sizeof(byte)) == sizeof(byte)){
        write_wrapper(inputFd, &byte, sizeof(byte));
    }

    if(unlink(tempName) == -1) {
            err(1, "could not delete temp file");
        }

    close(inputFd);
    close(tempFd);

}