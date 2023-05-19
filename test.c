#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DEVICE_NODE "/dev/randInt_dev"
#define MAX_LEN 4

/* Function to check entry point open of randInt driver */
int open_device() {
    int fd = open(DEVICE_NODE, O_RDWR);
    if(fd < 0) 
    {
        printf("Can not open the device file\n");
        exit(1);
    }
    printf("Handled open.\n");
    return fd;
}

/* Function to check entry point release of randInt driver */
void close_device(int fd) {
    close(fd);
    printf("Handle close.\n");
}

void read_data_device(int fd)
{
    char result[10];
    int ret = read(fd, &result, MAX_LEN);
    
    if (ret < 0)
        printf("Could not read int from %s\n", DEVICE_NODE);
    else{
        printf("Random Integer: %s\n", result);
    }
}



int main() {
    int ret = 0;
    char option = 'q';
    int fd = -1;
    printf("Enter your choice:\n");
    printf("o: open device node\n");
    printf("c: close device node\n");
    printf("q: quit\n");
    printf("r: read random integer\n");
    while (1) {
        printf("Enter your option: ");
        scanf(" %c", &option);

        switch (option) {
            case 'o':
                if (fd < 0)
                    fd = open_device();
                else
                    printf("%s has already opened\n", DEVICE_NODE);
                break;
            case 'c':
                if (fd > -1)
                    close_device(fd);
                else
                    printf("%s has not opened yet! Can not close\n", DEVICE_NODE);
                fd = -1;
                break;
            case 'r':
                read_data_device(fd);
                break;
            case 'q':
                if (fd > -1)
                    close_device(fd);
                printf("Exit the application.\n");
                return 0;
            default:
                printf("invalid option %c\n", option);
                break;
        }
    };
}