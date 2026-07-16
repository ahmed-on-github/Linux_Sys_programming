/*
 * build as:
 *    $ gcc main_server.c -o add-server-fifo
 * run as:
 *    $ nohup ./add-server-fifo &
 *    In this case, it runs in background even if parent shell is closed (child receives SIGHUP and ignores it)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_FIFO "/tmp/addition_server_fifo"
#define MAX_NUMBERS 512U


int main(int argc, char **argv){
    int fd_server, fd_client;
    int bytes_read = 0;
    char buf[4096] = {0};
    char *return_fifo = NULL; /* Client fifo name */
    char *numbers[MAX_NUMBERS] = {NULL};

    int i = 0;

    int total_numbers ;
    double sum = 0, tmp_d;
    unsigned error = 0;
    char *ptr = NULL;

    /* Create fifo and ignore error if aleady exists */
    errno = 0;
    if( (mkfifo(SERVER_FIFO, 0644)) < 0
        && errno != EEXIST ){
        perror("mkfifo: ");
        exit(errno);
    }
    /* Open server fifo file */
    if((fd_server = open(SERVER_FIFO, O_RDONLY | O_NONBLOCK)) < 0){
        perror("open: server fifo");
        exit(errno);
    }
    /* Code */
    while(1){
        memset(buf, 0, sizeof(buf));
        if( (bytes_read = read(fd_server, buf, sizeof(buf))) < 0){
            perror("read: ");
        }
        if( bytes_read == 0) continue;
        /* Tokenize receieved string */
        return_fifo = strtok(buf,", \t\n");
        while( (numbers[i] = strtok(NULL, ", \t\n")) != NULL && (i< MAX_NUMBERS)){
            i++;
        }
        total_numbers = i;
        for(i = 0 ; i < total_numbers; i++){
            tmp_d = strtod(numbers[i], &ptr);
            if( *ptr != '\0'){
                error = 1; break;
            }
            sum += tmp_d;
        }
        /* open client fifo file */
        if( (fd_client = open(return_fifo, O_WRONLY | O_NONBLOCK)) ){
            perror("open: client fifo ");
            exit(errno);
        }
        if(error){
            sprintf(buf, "error in input.\n");
        }
        else{
            sprintf(buf, "sum = %.8g\n\0", sum);
        }

        if( write(fd_client, buf, strlen(buf)) < strlen(buf)){
            perror("write: ");
        }

        if( close(fd_client) < 0){
            perror("close: ");
        }

    }
    return 0;
}
