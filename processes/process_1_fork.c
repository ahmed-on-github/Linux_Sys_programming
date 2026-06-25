#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>


int main(int argc, char **argv){
	
	int fork_res = 0 , i = 0; 
	if ( (fork_res = fork()) == 0 ){ /* Child */
		/*If parent finishes before the chilld, the child keepps printing after the shell prompt
		  because the shell is waiting for the parent, not the child*/
		for( i = 0 ; i < 10000 ; i++){
			printf("  ** Child process, id = %d, count = %d\n",getpid(),i);
		}
	}	
	else{  
		for ( i = 0 ;  i < 10000 ; i++){
			printf("** Parent process, id = %d, count = %d\n",getpid(),i);
		}
	}		
	return 0;
}
