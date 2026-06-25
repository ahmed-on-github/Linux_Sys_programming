#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>


#define EXECL()   execl("/bin/ls", "ls", "-la", 0);
#define EXECLE()  execle("/bin/ls", "ls", "-la", 0, cmnd_envp);

#define EXECV()   execv("/bin/ls", cmnd_argv);
#define EXECVE()  execve("/bin/ls", cmnd_argv, cmnd_envp); /*Doesn't exist?*/

#define EXECLP()  execlp("ls", "ls", "-la", ".", 0);
#define EXECVP()  execvp(cmnd_argv[0], cmnd_argv);
#define EXECVPE() execvpe(cmnd_argv[0], cmnd_argv, cmnd_envp);

extern char **environ;
static volatile int childStatus = 0;


int main(int argc, char **argv){
	char* cmnd_argv [] = {"ls", "-la", ".", /* "&&" , "tail" , "-f", "/dev/null", "&", */NULL};
	char* cmnd_envp [] = {"EDITOR=vi", "SHELL=/bin/sh", "TZ=:EST", NULL};

    if(argc != 2){
        perror("process_2_exec: usage: ./process_2_exec (exec[l|v][p?][e?])?\n");
        //exit(-1);
        goto no_args;
    }


    if( memcmp(argv[1],"execl", strlen(argv[1])) == 0){
        printf("\x1b[31;1m" "execl(\"/bin/ls\", \"ls\", \"-la\", 0);" "\x1b[0m\n");
        EXECL();
    }
    if( memcmp(argv[1],"execle", strlen(argv[1])) == 0){
        printf("\x1b[31;1m" "execle(\"/bin/ls\", \"ls\", \"-la\", 0, cmnd_envp);" "\x1b[0m\n");
        EXECLE();
    }
    if( memcmp(argv[1],"execv", strlen(argv[1])) == 0){
        printf("\x1b[31;1m" "execv(\"/bin/ls\", cmnd_argv);" "\x1b[0m\n");
        EXECV();
    }

    if( memcmp(argv[1],"execve", strlen(argv[1])) == 0){
        printf("\x1b[31;1m" "execve(\"/bin/ls\", cmnd_argv, cmnd_envp); " "\x1b[0m\n");
        EXECVE();
    }
    if( memcmp(argv[1],"execlp", strlen(argv[1])) == 0){
     	printf("\x1b[31;1m" "execlp(\"ls\", \"ls\", \"-la\", \".\", 0);" "\x1b[0m\n");
        EXECLP();
    }


    if( memcmp(argv[1],"execvp", strlen(argv[1])) == 0){
    	printf("\x1b[31;1m" "execvp(cmnd_argv[0], cmnd_argv);" "\x1b[0m\n");
        EXECVP();
	}
	if( memcmp(argv[1],"execvpe", strlen(argv[1])) == 0 ){
        printf("\x1b[31;1m" "execvpe(cmnd_argv[0], cmnd_argv, cmnd_envp);" "\x1b[0m\n");
		EXECVPE();
	}
    return 0;
no_args:
	for(int i = 0 ; i < 7 ; i++){
        if(fork() == 0){ /* Child*/
            switch(i){
                /* Do not leave an empty case as it will fork and new child
                without exiting like an execx[x?][x?] does*/
                case 0:
                    printf("\x1b[31;1m" "execl(\"/bin/ls\", \"ls\", \"-la\", 0);" "\x1b[0m\n");
                    EXECL();
                    break;
                case 1:
                    printf("\x1b[31;1m" "execle(\"/bin/ls\", \"ls\", \"-la\", 0, cmnd_envp);" "\x1b[0m\n");
                    EXECLE();
                    break;
                case 2:
                    printf("\x1b[31;1m" "execv(\"/bin/ls\", cmnd_argv);" "\x1b[0m\n");
                    EXECV();
                    break;
                case 3:
                    printf("\x1b[31;1m" "execve(\"/bin/ls\", cmnd_argv, cmnd_envp); " "\x1b[0m\n");
                    EXECVE();
                    break;
                case 4:
                    printf("\x1b[31;1m" "execlp(\"ls\", \"ls\", \"-la\", \".\", 0);" "\x1b[0m\n");
                    EXECLP();
                    break;
                case 5:
                    printf("\x1b[31;1m" "execvp(cmnd_argv[0], cmnd_argv);" "\x1b[0m\n");
                    EXECVP();
                    break;
                case 6:
                    printf("\x1b[31;1m" "execvpe(cmnd_argv[0], cmnd_argv, cmnd_envp);" "\x1b[0m\n");
                    EXECVPE();
                    break;
                default:
                    printf("Unknown function");
                    break;
            }
        }
        else{   /* Parent */
            wait(&childStatus);
            printf("Return value from child: %d\n", childStatus);
        }
	}
	return 0;
}
