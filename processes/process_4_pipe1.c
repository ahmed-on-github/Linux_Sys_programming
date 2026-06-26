/**
*** Small shell allowing for a piped command (2 commands)
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


#define BUFF_SIZE 1024U


typedef struct shell_parser{
    char **upStream, **downStream;
    bool pipeFound;
}shell_parser_t;


typedef union pipe_t{
    int fd[2];
    int downstream /* on fd[0] */, upstream /* on fd[1] */;
}pipe_t;


//#define DEBUG(FMT,...)

void strtok_textExample(void){
    /* Possible test string: Hello linux system programming in C :)*/
    /* Do not use gets() as it may cause buffer overflow*/
    char buff[BUFF_SIZE] = {0};
    printf("Enter a string: ");
    fgets(buff,BUFF_SIZE,stdin);
    printf("Entered string: %s\n", buff);

    /*String tokenization*/
    char *words[10] = {NULL}; int i = 0;
    if( ((words[0] = strtok(buff," ")) != NULL) && printf("%s\n", words[0]))
        while( words[++i] = strtok(NULL," \t") ){
            printf("%s\n", words[i]);
        }
    printf("After strtok(): Entered string: %s\n", buff); /* Only 1st token before any delimiter */
}

const shell_parser_t* shell_readAndParseCmnd(char *cmnd_buff){
    static shell_parser_t l_shell_struct= {0};
    static char* up  [256] = {0};   /*Allows 255 words in upstream command*/
    static char* down[256] = {0};   /*Allows 255 words in downstream command */

    char **tmp_ptr = up;
    int up_i = 0, down_i = 0;
    int *i_ptr = &up_i;

    /* Solved by ending with a placing a NULL pointer at end
    memset(up, 0, 256*sizeof(*up));
    memset(down, 0, 256*sizeof(*up));
    memset(&l_shell_struct, 0, sizeof(shell_parser_t));
    */

    if( cmnd == NULL  ){
        perror("Empty command passed\n");
        return NULL;
    }
    /*
    if( tokenArr == NULL ){
        perror("Empty token array passed\n");
        exit(-2);
    }
    */

    if( (tmp_ptr[*i_ptr] = strtok(cmnd_buff, " \t")) != NULL && i_ptr < 255 ){
        while( (tmp_ptr[++(*i_ptr)] = strtok(cmnd_buff, " \t")) != NULL ){
            switch(strlen(tmp_ptr[*i_ptr])){
                case 1:
                    if( tmp_ptr == up && tmp_ptr[*i_ptr][0] == '|'){
                        tmp_ptr[*i_ptr] = NULL ; /* Execlude the pipe '|' from 1st command's tokens*/
                        tmp_ptr = down; i_ptr = &down_i; /* Point to downstream command */
                        l_shell_struct.pipeFound = true;
                    }
                    break;
                default:
                    continue;
            }
        }
        tmp_ptr[++(*i_ptr)] = NULL; /*Terminate the command (either upstream or downstream)*/
    }
    return &l_shell_struct;
}
int main(void){

    strtok_textExample();
    char cmnd_buff[BUFF_SIZE], temp_cmnd_buff[BUFF] = {0};
    shell_parser_t *ret_ptr = NULL;
    int cmnd_exitStatus = 0;
    while(1){
        printf("\x1b[33;1m>\x1b[0m ");
        memset(cmnd_buff, 0, BUFF_SIZE);
        fgets(cmnd_buff, BUFF_SIZE,stdin);
        memcpy(temp_cmnd_buff, cmnd_buff, sizeof(cmnd_buff));
        temp_cmnd_buff[sizeof(cmnd_buff)] = '\0';

        if( (ret_ptr = shell_readAndParseCmnd(cmnd_buff)) ){
            switch( ret_ptr->pipeFound ){
                case false:
                    /* Just fork once and execute the upstream command*/
                    pid_t fork_res = fork();
                    if( fork_res == -1){
                        perror("Could not fork for command %s with exit status = %d\n", temp_cmnd_buff, errno);
                    }
                    else if( fork_res == 0 ){ /* Child process*/
                        execvp(ret_ptr->upStream[0], ret_ptr->upStream);
                    }
                    else{ /* Parent process*/
                        /* Nothing, shell will just wait for the current command */
                        wait(&cmnd_exitStatus);
                    }
                case true:
                    /* Create a pipe, fork twice and execute both the  upstream  and downstream command*/


                    ret_ptr->pipeFound = false ; /* Default for next commands */
                default

            }
        }
    }
    return 0;
}
