/* (Still not working)
 * build as:
 *    $ gcc main_client.c -o add-client-fifo
 * run as:
 *    $ ./add-client-fifo         # reads input from stdin, entered by user , line by line 
 *    $ ./add-client-fifo < file  # reads input from stdin, redirected from file , line by line, till EOF
 *    $ ./add-client-fifo         # reads input from argv, runs once on argv
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <signal.h>

#define SERVER_FIFO "/tmp/addition_server_fifo"
#define MAX_NUMBERS 512U

static char fifo_buf [4096];

int fd_client, fd_server, bytes ;
char client_fifo_name [40] ;

void operation(void){
    /* Open server fifo for writing */
    if((fd_server = open(SERVER_FIFO, O_WRONLY))<0){
        perror("open: ");
        exit(errno);
    }
    /* send data to server fifo file */
    if(write(fd_server, fifo_buf, strlen(fifo_buf)) < strlen(fifo_buf)){
        perror("write: ");
    }
    /* close server fifo file */
    if( close(fd_server) < 0 ){
        perror("close: ");
    }
    /* read sum from server process in same fifo buf */
    if( (bytes = read(fd_client, fifo_buf, 4096 )) < 0){
        perror("read: ");
    }
}

void signal_handler(int signum){
    printf("signal %s received\n", strsignal(signum));
    close(fd_client); close(fd_server);
}
int main(int argc, char **argv){

    struct sigaction signal_struct;
    signal_struct.sa_flags = 0;
    signal_struct.sa_handler = signal_handler;

    sigaction(SIGTERM, &signal_struct, NULL);

    /* Create Fifo for client process */
    sprintf(client_fifo_name
    , "/tmp/addition_client_fifo_%d\0", getpid());

    /* Create fifo */
    if ( mkfifo(client_fifo_name, 0644) < 0
    && errno != EEXIST ){
        perror("mkfifo: ");
        exit(errno);
    }

    /* Open client fifo file for reading */
    if((fd_client = open(client_fifo_name, O_RDONLY))<0){
        perror("open: ");
        exit(errno);
    }

    /* place fifo name in buffer */
    size_t client_fifo_name_len = strlen(client_fifo_name);

    memcpy(fifo_buf, client_fifo_name, client_fifo_name_len);
    fifo_buf[client_fifo_name_len] = ' '; /*separate fifo name with a space*/
    /* read buffer */
    if(argc == 1){
        while( printf("Enter numbers: ") && fgets(fifo_buf + client_fifo_name_len +1,
        4096 -  client_fifo_name_len - 1, stdin) != NULL ){
            operation();

            if(bytes == 0) continue;

            fifo_buf[ bytes ] = '\0'; /* terminate for printing */
            printf("%s", fifo_buf);

            /* reset fifo buf for resending */
            memcpy(fifo_buf, client_fifo_name, client_fifo_name_len);
            fifo_buf[client_fifo_name_len] = ' '; /*separate fifo name with a space*/

            /* reset place for numbers in the buffer */
            memset(fifo_buf + client_fifo_name_len +1, 0, 4096 -  client_fifo_name_len - 1 );
        }

    }
    else{
        /*values passed via argv */
        for( int i = 1; i < argc && i <= MAX_NUMBERS && strlen(fifo_buf) < 4096; i++ ){
            memcpy(fifo_buf+ strlen(fifo_buf), argv[i], strlen(argv[i]));
            fifo_buf[strlen(fifo_buf)] = ' '; /* Separate with spaces */
        }
        operation();
        fifo_buf[ bytes ] = '\0'; /* terminate for printing */
        printf("%s", fifo_buf);
        close(fd_client);  // close(fd_server); already closed in operaion()
    }
    return 0;
}
