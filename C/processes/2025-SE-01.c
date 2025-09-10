#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

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

void pipew(int pd[2]){
    if(pipe(pd) == -1){
        err(6, "pipe");
    }
}

pid_t forkw(void){
    pid_t pid = fork();
    if(pid == -1){
        err(7, "fork");
    }
    return pid;
}

int dup2w(int old, int new){
    int res = dup2(old, new);
    if(res == -1){
        err(8, "dup2");
    }
    return res;
}

int main(void) {
	int driver_to_out[2];
	pipew(driver_to_out);

    pid_t drive = forkw();
	if(drive == 0){
        close(driver_to_out[0]);

        dup2(driver_to_out[1], 1);
        close(driver_to_out[1]);

        execlp("./sample_data/fake_driver", "./sample_data/fake_driver", (char*)NULL);
        err(3, "execlp");
	}
	close(driver_to_out[1]);

	int wheel_to_out[4][2];
	for(int i = 0; i < 4; i++){
        pipew(wheel_to_out[i]);
	}

	int in_to_wheel[4][2];
	for(int i = 0; i < 4; i++){
        pipew(in_to_wheel[i]);
	}
	
	for(int i = 0; i < 4; i++){
        pid_t wheels = forkw();
    	if(wheels == 0){
            close(driver_to_out[0]);
            close(wheel_to_out[i][0]);
            close(in_to_wheel[i][1]);

            for(int j = 0; j < 4; j++){
                if(j != i){ 
                close(wheel_to_out[j][0]); 
                close(wheel_to_out[j][1]);
                close(in_to_wheel[j][0]);
                close(in_to_wheel[j][1]);
                }
            }

            dup2(in_to_wheel[i][0], 0);
            close(in_to_wheel[i][0]);

            dup2(wheel_to_out[i][1], 1);
            close(wheel_to_out[i][1]);

            execlp("./sample_data/fake_wheel", "./sample_data/fake_wheel", (char*)NULL);
            err(4, "execlp");
    	}	
    }
    for(int i = 0; i < 4; i++){
        close(in_to_wheel[i][0]);
        close(wheel_to_out[i][1]);
    }

    uint16_t I = 0;
    
    while(1){
        uint16_t drive_pack[8];
        readw(driver_to_out[0], drive_pack, sizeof(drive_pack));

        uint16_t gas = drive_pack[4];

        uint16_t speed[4];
        for(int i = 0; i < 4; i++){
            uint16_t wheel_pack[8];
            readw(wheel_to_out[i][0], wheel_pack, sizeof(wheel_pack));
            speed[i] = wheel_pack[1];
        }
    
        uint16_t fullspeed = 0;
        for(int i = 0; i < 4; i++){
            fullspeed += speed[i];
        }

        uint16_t realspeed = fullspeed / 4;
        
        if(realspeed < gas){
            I++;
        } else {
            I--;
        }

        uint16_t elect_pack[8];
        memset(elect_pack, 0, sizeof(elect_pack));
        elect_pack[1] = I;

        for(int i = 0; i < 4; i++){
            writew(in_to_wheel[i][1], elect_pack, sizeof(elect_pack));
        }
    }
    return 0;
}