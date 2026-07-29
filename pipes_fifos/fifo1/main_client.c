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

//#define INPUT_REDIRECT_FLAG

static char fifo_buf [4096];

int fd_client, fd_server, bytes ;
char client_fifo_name [40] ;

void operation(void){
    /* Open server fifo for writing */
    if((fd_server = open(SERVER_FIFO, O_WRONLY /*| O_NONBLOCK*/))<0){
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
    /* lseek(fd_client,0, SEEK_SET);  // read from start? No, does not work with fifos*/
    while((bytes = read(fd_client, fifo_buf, 4096 )) <= 0){
        //perror("read: ");
    }
}

void signal_handler(int signum){
    printf("signal %s received\n", strsignal(signum));
    close(fd_client); close(fd_server);
}
int main(int argc, char **argv){
    char *fgets_ptr = NULL;


    struct sigaction signal_struct;
    signal_struct.sa_flags = 0;
    signal_struct.sa_handler = signal_handler;

    sigaction(SIGTERM, &signal_struct, NULL);

    /* Create Fifo for client process */
    snprintf(client_fifo_name, sizeof(client_fifo_name)
    , "/tmp/addition_client_fifo_%d", getpid());

    /* Create fifo */
    if ( mkfifo(client_fifo_name, 0644) < 0
    && errno != EEXIST ){
        perror("mkfifo: ");
        exit(errno);
    }

    /*  Open client fifo file for reading.
        The open was made non-blocking to proceed with code execution,
        then can then make blocking again to ensure syncronized reads from it,
        or just ensure we block until we read from it at least 1 byte
    */
    if((fd_client = open(client_fifo_name, O_RDONLY | O_NONBLOCK))<0){
        perror("open: ");
        exit(errno);
    }
    /*
    int fd_client_flags = 0;
    fd_client_flags = fcntl(fd_client,F_GETFD);
    fd_client_flags &= (~O_NONBLOCK);
    fcntl(fd_client, F_SETFD, fd_client_flags);
    */
    /* place fifo name in buffer */
    size_t client_fifo_name_len = strlen(client_fifo_name);

    memcpy(fifo_buf, client_fifo_name, client_fifo_name_len);
    fifo_buf[client_fifo_name_len] = ' '; /*separate fifo name with a space*/

    /* Simulate  redirected input */

    FILE *input_fptr = stdin;
    #ifdef INPUT_REDIRECT_FLAG
    int input_fd =  open("./testfile", O_RDONLY);
    int stdin_pipe [2];
    pipe(stdin_pipe);
    dup2(stdin_pipe[0], STDIN_FILENO);
    close(stdin_pipe[1]); /* close writing end */

    input_fptr = fdopen(input_fd, "r");
    #endif /* INPUT_REDIRECT_FLAG */

    /* read buffer */
    if(argc == 1){
        /*
        while( printf("Enter numbers: ") && fread(fifo_buf + client_fifo_name_len + 1,
                            1, 4096 -  client_fifo_name_len - 1,
                            stdin) > 0 ){ */ /* fread() does not stop on \n */
        int started_flag = 0;
        int feof_flag = 0;
        do{
            if(started_flag == 0){
                printf("Enter numbers: "); fflush(stdout);
                fgets_ptr = fgets(fifo_buf + client_fifo_name_len +1,
                                  4096 -  client_fifo_name_len - 1, input_fptr);
            }
            fifo_buf[ strlen(fifo_buf) - 1 ] = '\0'; /*terminate \n in order to allow next strlen()  call to return 0 on just pressing enter (\n)*/
            if( strlen(fifo_buf + client_fifo_name_len +1) == 0 ) {
                goto retry; /* continue; //  for older vesrion with while() */ 
            }
            /* save redirected input from a file before getting out of the do-while loop
               in order to print the output before the sum */
            memcpy(tmp_fifo_buf,fifo_buf + client_fifo_name_len +1, strlen(fifo_buf + client_fifo_name_len +1));

            operation();

            if(bytes == 0) { printf("\n"); goto retry; }

            fifo_buf[ bytes ] = '\0'; /* terminate for printing */

            memcpy(result_buf, fifo_buf, strlen(fifo_buf));
            if(input_fptr == stdin)
                printf("%s", fifo_buf);
            started_flag = 1; /* for next iterations */

retry:
            /* reset fifo buf for resending */
            memcpy(fifo_buf, client_fifo_name, client_fifo_name_len);
            fifo_buf[client_fifo_name_len] = ' '; /*separate fifo name with a space*/

            /* reset place for numbers in the buffer */
            memset(fifo_buf + client_fifo_name_len +1, 0, 4096 -  client_fifo_name_len - 1 );


            printf("Enter numbers: "); fflush(stdout);
            fgets_ptr = fgets(fifo_buf + client_fifo_name_len +1,
                              4096 -  client_fifo_name_len - 1, input_fptr);
            feof_flag = feof(stdin);
            /* simutale redirected input behaviour for debugging */
            #ifdef INPUT_REDIRECT_FLAG
            fgets_ptr = NULL; feof_flag = 1;
            #endif /* INPUT_REDIRECT_FLAG */
        }while( fgets_ptr != NULL && feof_flag == 0);
        /* EOF reached while using input redirection */

        /* (For previous while() version)
           Another "Enter number:" was printed before EOF was detected.
           To remove it and its whole line :
           -    1st \r (carriage return) moves cursor to the beginning of its line
           -    Enough spaces to overwrite the printed "Enter numbers:"
           -    2nd \r moves cursor again to the beginning of the line
           -    Then \b (backspace) removes the new line character before the cursor
        */
        /* printf("\r              \r\b"); */
        /* For current version with do-while() */
        printf("\b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b");

        /* print the re-directed input as it is not yet written on the console */
        printf(" %s\n", tmp_fifo_buf); /* \n flushes stdout */
        printf("%s", result_buf); /* already contains \n*/
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

