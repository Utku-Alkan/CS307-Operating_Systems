#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char *argv[]){


    printf("I’m SHELL process, with PID: %d - Main command is: man grep | grep -nA5 \\-w,\n", (int) getpid());
    int fd[2];
    pipe(fd); 


    int rc = fork();
    if (rc<0){
        
        fprintf(stderr, "fork failed\n");
        exit(1);
        
    }else if (rc == 0){ //child

        int rc2 = fork();

        if (rc2<0){
            
            fprintf(stderr, "fork failed\n");
            exit(1);
            
        }else if(rc2==0){ //grandchild

            printf("I’m GREP process, with PID: %d - My command is: grep -nA5 \\-w,\n", (int) getpid());

            close(STDOUT_FILENO);
            open("./output.txt", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            
            dup2(fd[0], 0); // reads from the pipe

            close(fd[0]);
            close(fd[1]);
            char *myargs2[4];
            myargs2[0] = "grep";
            myargs2[1] = "-nA5";
            myargs2[2] = "\\-w,";
            myargs2[3] = NULL;
            execvp(myargs2[0],myargs2);

        }else{ //parent
            printf("I’m MAN process, with PID: %d - My command is: man grep\n", (int) getpid());

            dup2(fd[1], 1); // writes to the pipe 
            close(fd[0]);
            close(fd[1]);

            char *myargs[3];
            myargs[0] = "man";
            myargs[1] = "grep";
            myargs[2] = NULL;
            execvp(myargs[0],myargs);
        }
    }else{ //parent

    
        int rc_wait = wait(NULL);
        printf("I’m SHELL process, with PID:%d - execution is completed, you can find the results in output.txt\n", (int) getpid());

    }
    exit(1);
    
    return 0;
}

