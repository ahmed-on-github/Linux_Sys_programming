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
        errno = 0;
        while( (bytes_read = read(fd_server, buf, sizeof(buf))) < 0
                && (errno == EAGAIN || errno == EWOULDBLOCK) ); /* retry reading on blockage*/
        if( bytes_read == 0) continue;
        if( errno != 0 || errno != EAGAIN || errno != EWOULDBLOCK){
            perror("read: ");
        }
        /* Tokenize receieved string */
        return_fifo = strtok(buf,", \t\n\0");
	    i=0; /*mandatory: reset i*/
        while( (numbers[i] = strtok(NULL, ", \t\n\0")) != NULL && (i< MAX_NUMBERS)){
            i++;
        }
        total_numbers = i; sum = 0; /*mandatory: reset sum*/
        for(i = 0 ; i < total_numbers; i++){
            tmp_d = strtod(numbers[i], &ptr);
            if( *ptr != '\0'){
                error = 1; break;
            }
            sum += tmp_d;
        }
        /* open client fifo file */
        if( (fd_client = open(return_fifo, O_WRONLY | O_NONBLOCK)) < 0 ){
            perror("open: client fifo ");
            exit(errno);
        }
        if(error){
            sprintf(buf, "error in input.\n");
	    error = 0; /* mandatory: reset error */
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
