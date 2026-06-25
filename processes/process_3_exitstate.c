#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include <unistd.h>

#define NO_OPTIONS 0
static volatile int childStatus = 0;

int main(void){
    pid_t child_pid = 0;

    if((child_pid = fork())  < 0 ){
        perror("Could not fork()\n");
        exit(errno);
    }
    if( child_pid == 0 ){ /* Child */
        printf("Child process with pid = %d running...\n", getpid());
        sleep(500); /* Allowing for being stopped, continued or terminated from other shell via a signal.
        Reduce for normal termination */
        /* *((int *)0) = 0; /*Causes segmentation violation signal SIGSEV*/

        return 5 ; /* or exit(5); /* Normal exit */
    }
    else{ /* Parent */
        printf("Parent is waiting for child with pid = %d ...\n", child_pid);
        waitpid(child_pid,(int *)(&childStatus), WUNTRACED /* allows catching SIGSTOP*/ | WCONTINUED /* allows catching SIGCONT*/);
        /* Check child termination status */
        if(WIFEXITED(childStatus)){
            printf("Child Process %d exited normally with exit code %d\n",
                    child_pid, WEXITSTATUS(childStatus));
        }
        else if( WIFSIGNALED(childStatus) ){
            printf("Child Process %d terminated by signal %d\n",
                    child_pid, WTERMSIG(childStatus));
        }
        else if( WIFSTOPPED(childStatus) ){ 
            printf("Child process %d stoped\n", child_pid);
        }
        else if( WIFCONTINUED(childStatus) ){
            printf("Child process %d continued\n", child_pid);
        }
    }

    return 0;
}
