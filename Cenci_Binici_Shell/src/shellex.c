#define _XOPEN_SOURCE
#include "csapp.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdbool.h>

#define MAXARGS 128
#define READ_END 0
#define WRITE_END 1
//#define MAXLINE 8192
/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
extern char **environ;
char* retrieveEnvironVar();
volatile bool isRunning = true; 
bool showU = false, showS = false, showP = false, showV = false, showI = false, showA = false, showL = false, showC = false;
char statOrder[6] = "";
void unix_error(char *msg) /* Unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}
void sigint_handler(int signal)
{
    //printf("Killed child with pid %d\n", getpid());
}
void sigtstp_handler(int signal)
{
    //printf("Stopped child with pid %d\n", getpid());
}

void sigkill(int p)
{
    isRunning = true;
}
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv)
{
    char *delim;/* Points to first space delimiter */
    int argc; /* Number of args */
    int bg; /* Background job? */

    buf[strlen(buf)-1] = ' '; /* Replace trailing '\n' with space */
    while(*buf && (*buf == ' ')) /* Ignore leading spaces */
    {
        buf++;
    }
    /* Build argv list */
     argc =0;
    while((delim = strchr(buf, ' ')))
    {
        argv[argc++] = buf;
        *delim ='\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /*ignore spaces */
        {
            buf++;
        }
    }
    argv[argc] = NULL;

    if (argc == 0) /* ignore blank line */
        return 1;

    /*Should the job run in the background*/
    if ((bg =(*argv[argc-1] == '&')) !=0)
        argv[--argc] = NULL;

    return bg;


}

/* eval - Evaluate a command line */
void eval (char *cmdline)
{
        char *argv[MAXARGS];    /* Argument list execve() */
        char buf[MAXLINE];      /* Holds modified command line */
        int bg;                 /* Should the job run in bg or fg? */
        pid_t pid;              /* Process id */

        strcpy(buf, cmdline);
        bg = parseline(buf, argv);

        struct rusage usage;
        struct timeval startU, startS;
        long startP, startV, startI;
        struct timeval endU, endS;
        long endP, endV, endI;


        if (argv[0] == NULL)
            return;     /* Ignore empty lines */
        
        char **passedVal = argv;
        int l = 0;
        while (argv[l] != NULL)
        {
            l++;
        }
        bool pipesExist = false;
        int inputLength = strlen(cmdline);
        char entireInput[inputLength + 1];
        entireInput[inputLength + 1] = '\0';
        strcpy(entireInput, cmdline);
        int pipeAmount = 0;
        for (int i = 0; i < inputLength; i++) {
            if (entireInput[i] == '|') 
            {
                pipeAmount++;
                pipesExist = true;
            }
        }

        /* Detecting if pipes exist within the user input and then parsing the lien with strtok. */
        if (pipesExist) {
            const char *cmds[pipeAmount + 1];//[pipeAmount + 1];
            char *pt;
            pt = strtok (cmdline," | "); // Looking for the pipe | symbol
            int valueHere = 0;
            while (pt != NULL) {
                cmds[valueHere] = pt;
                valueHere++;
                pt = strtok (NULL, " | "); // Nulling the array parsed.
                
                if (pt == NULL)
                {
                    char* lastVal = cmds[valueHere-1];
                    lastVal[strlen(lastVal) - 1] = 0;
                    cmds[valueHere - 1] = lastVal;
                }
                
            }

            //cmds[valueHere] = '\0';
            cmds[pipeAmount + 1] = '\0'; // finishes the command array list with the terminal 0 in the string.

            int pid, pid_ls, pid_grep;
            int pipefd[2];
            
            // STOP LOOP HERE
            if (pipe(pipefd) == -1) {
                fprintf(stderr, "parent: Failed to create pipe\n");
                return;
            }
            // END STOP LOOP

            pid_grep = fork();

            if (pid_grep == -1) {
                fprintf(stderr, "parent: Could not fork process to run grep\n");
                return;
            } 
            else if (pid_grep == 0) 
            {
                fprintf(stdout, "child: grep child will now run\n");

                // Set fd[0] (stdin) to the read end of the pipe
                if (dup2(pipefd[READ_END], STDIN_FILENO) == -1) 
                {
                    fprintf(stderr, "child: grep dup2 failed\n");
                    return;
                }

                // Close the pipe now that we've duplicated it
                close(pipefd[READ_END]);
                close(pipefd[WRITE_END]);

                // Setup the arguments to call
                char *new_argv[] = { cmds[1], argv[2], 0 };
                new_argv[1] = 0;

                fprintf(stderr,"[%s]\n",cmds[1]);
                fprintf(stderr, "Command: %s\nArgs[0]: %s\nArgs[1]: %s\n", cmds[0], cmds[0], cmds[1]);

                execvp(new_argv[0], new_argv);
                
                // Execution will never continue in this process unless execve returns because of an error
                fprintf(stderr, "child: Oops, %s failed!\n", cmds[1]);
                fprintf(stderr, "Errno(%d): %s\n", errno, strerror(errno));
                return;
            }
            pid_ls = fork();

            if (pid_ls == -1) {
                fprintf(stderr, "parent: Could not fork process to run ls\n");
                return;
            } 
            else if (pid_ls == 0)
            {
                fprintf(stdout, "child: ls child will now run\n");
                fprintf(stdout, "---------------------\n");

                // Set fd[1] (stdout) to the write end of the pipe
                if (dup2(pipefd[WRITE_END], STDOUT_FILENO) == -1) 
                {
                    fprintf(stderr, "ls dup2 failed\n");
                    return;
                }

                // Close the pipe now that we've duplicated it
                close(pipefd[READ_END]);
                close(pipefd[WRITE_END]);

                // Setup the arguments/environment to call
                char *new_argv[] = {cmds[0], argv[2], 0 };
                // Call execve(2) which will replace the executable image of this process
                new_argv[1] = 0;
                fprintf(stderr,"[%s]\n", cmds[0]);
                fprintf(stderr, "Command: %s\nArgs[0]: %s\nArgs[1]: %s\n", cmds[0], cmds[0], cmds[1]);		

                execvp(new_argv[0], new_argv);
                // Execution will never continue in this process unless execve returns because of an error
                fprintf(stderr, "child: Oops, %s failed!\n", cmds[0]);
                fprintf(stderr, "Errno(%d): %s\n", errno, strerror(errno));
                return;
            }

            // Parent doesn't need the pipes
            close(pipefd[READ_END]);
            close(pipefd[WRITE_END]);

            fprintf(stdout, "parent: Parent will now wait for children to finish execution\n");

            // Wait for all children to finish
            while (wait(NULL) > 0);

            fprintf(stdout, "---------------------\n");
            fprintf(stdout, "parent: Children has finished execution, parent is done\n");
            
            return;
        }

        if (!builtin_command(argv)) {
            
            if ((pid = fork()) == 0) {  /*Child runs user job  */
                if (execvp(argv[0], argv) < 0) {
                    printf("%s: Command not found.\n", argv[0]);
                    exit(0);
                }

            }
                // Instantiate statistics depnding on if the user wanted to show the statistics.
                getrusage(RUSAGE_SELF, &usage);
                if (showU) startU = usage.ru_utime;
                if (showS) startS = usage.ru_stime;
                if (showP) startP = usage.ru_majflt;
                if (showV) startV = usage.ru_nvcsw;
                if (showI) startI = usage.ru_nivcsw;

            /* Parent waits for foreground job to terminate */
            if (!bg) {
                int status;
                if (waitpid(pid, &status, WUNTRACED) < 0) // Using WUNTRACED coupled with signals ^C & ^Z
                    unix_error("waitfg: waitpid error");
            }
            else{
                printf("%d %s", pid, cmdline);        
            }
                    /* Statistics Print out in to show computer statsitics in the order specified by the user */
                getrusage(RUSAGE_SELF, &usage);
                for (int i = 0; i < strlen(statOrder); i++) { // Uses a for loop to process the order char* array set up.
                    if (statOrder[i] == 'u')
                    {
                        endU = usage.ru_utime;
                        printf("Started at: %ld.%lds | ", startU.tv_sec, startU.tv_usec);
                        printf("Ended at: %ld.%lds\n", endU.tv_sec, endU.tv_usec);
                    }
                    if (statOrder[i] == 's')
                    {
                        endS = usage.ru_stime;
                        printf("Started at: %ld.%lds | ", startS.tv_sec, startS.tv_usec);
                        printf("Ended at: %ld.%lds\n", endS.tv_sec, endS.tv_usec);
                    }
                    if (statOrder[i] == 'p')
                    {
                        endP = usage.ru_majflt;
                        printf("Number of Page Faults | %ld\n", endP, endP);
                    }
                    if (statOrder[i] == 'v')
                    {
                        printf("Started at: %ld.%lds | ", startV, startV);
                        printf("Ended at: %ld.%lds\n", endV, endV);
                    }
                    if (statOrder[i] == 'i')
                    {
                        endI = usage.ru_nivcsw;
                        printf("Started at: %ld.%lds | ", startI, startI);
                        printf("Ended at: %ld.%lds\n", endI, endI);
                    }
                }
                   
        }

        return;
}


char* retrieveEnvironVar(char* inputVariable)  // Returns the environment variable requested by the user.
{
    return getenv(inputVariable);
}
/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    
    if (!strcmp(argv[0], "fg")) {
        signal(SIGCONT, argv[1]);
    }

    // for the length of the word looks for the = sign.
    int lengthValue = strlen(argv[0]);
    for(int i = 0; i < lengthValue; i++) {

        if(*(argv[0]+i)== '=')
        {
            char* newVar = malloc(i+1);
            memcpy(newVar, *(&(argv[0])), i); // Saves the variable as the chars leading up to the = symbol

            char newVarsValue[(lengthValue - i)];
            memcpy(newVarsValue,  &*(argv[0]+i+1), lengthValue); // Saves the value as the chars after the = symbol
            newVarsValue[lengthValue] = "\0";

            char *oldValue[100];
            *oldValue = getenv(newVar);
            if (strlen(oldValue) == 0)  // Overwriting a null & existing value
            {
                unsetenv(newVar);
                setenv(newVar, newVarsValue, 1);
                return 1;
            }
            if (strlen(newVarsValue) == 0) // Setting a value to null
            { 
                unsetenv(newVar);
                return 1;
            }
            setenv(newVar, newVarsValue, 1); // overwrite the old value and set it in the environment
            argv[1] = newVar;
            return 1;
        }

    }

    if (*(argv[0]) == '$') /* Checks for the $ command */
    {
        char **followingString = argv[0] + 1;
        char **a = retrieveEnvironVar(followingString); // retrieves the string from path
        printf("%s\n", retrieveEnvironVar(followingString));
        argv[1] = a;
        return 1;
    }
    
    if (argv[1] != NULL) {
        if (*(argv[1]) == '$') /* echo is read */
        {
                char **followingString = argv[1] + 1;
                char **a = retrieveEnvironVar(followingString); 
                argv[1] = a;
                return 0;
        }
    }
    
    /* Stats commands */
    if (!strcmp(argv[0], "stats"))
    { 
        if (!strcmp(argv[1], "-a")) { // Turns all stats on
            memset(statOrder, 0, sizeof statOrder);
            strcat(statOrder, "uspvi");
            showU = true;
            showS = true;
            showP = true;
            showV = true;
            showI = true;
            return 1;
        }
        if (!strcmp(argv[1], "-l")) { // Shows all stats that are toggled on
            if (!showU && !showS && !showP && !showV && !showI) printf("%s\n", "No stats enabled.");
            else {
                       printf("%s\n","-------------------------------------------");
            if (showU) printf("%s\n","-u | CPU Time in User mode enabled");
            if (showS) printf("%s\n","-s | CPU Time in System/Kernel mode enabled");
            if (showP) printf("%s\n","-p | Number of Hard Page Faults enabled");
            if (showV) printf("%s\n","-v | Voluntary Context Switches enabled");
            if (showI) printf("%s\n","-i | Involuntary Context Switches enabled");
                       printf("%s\n","-------------------------------------------");
            }
            return 1;
        }
        if (!strcmp(argv[1], "-c")) { // Clears
            memset(statOrder, 0, sizeof statOrder);
            showU = false;
            showS = false;
            showP = false;
            showV = false;
            showI = false;
            printf("The new stat order is: %s (END)\n", statOrder);
            return 1;
        }
        for (int i = 1; argv[i] != NULL; i++) // activators and deactivators (flags) for each stat
        {
            if (!strcmp(argv[i], "-u")) {
                if (showU) {
                    showU = false;
                    for (int j = 0; j < strlen(statOrder); j++)
                    {
                        if (statOrder[j] == 'u') {
                            memmove(&statOrder[j], &statOrder[j + 1], strlen(statOrder) - j);
                        }
                    }
                }
                else {
                    showU = true;
                   int len = strlen(statOrder);
                    statOrder[len] = 'u';
                    statOrder[len+1] = '\0';
                }
            
            }
            if (!strcmp(argv[i], "-s")) {
                if (showS) {
                    showS = false;
                    for (int j = 0; j < strlen(statOrder); j++)
                    {
                        if (statOrder[j] == 's') {
                            memmove(&statOrder[j], &statOrder[j + 1], strlen(statOrder) - j);
                        }
                    }
                }
                else {
                    showS = true;
                    int len = strlen(statOrder);
                    statOrder[len] = 's';
                    statOrder[len+1] = '\0';
                }
                
            }
            if (!strcmp(argv[i], "-p")) {
                if(showP) {
                    showP = false;
                    for (int j = 0; j < strlen(statOrder); j++)
                    {
                        if (statOrder[j] == 'p') {
                            memmove(&statOrder[j], &statOrder[j + 1], strlen(statOrder) - j);
                        }
                    }
                }
                else {
                    showP = true;
                    int len = strlen(statOrder);
                    statOrder[len] = 'p';
                    statOrder[len+1] = '\0';
                }
                
            }
            if (!strcmp(argv[i], "-v")) {
                if(showV){
                    showV = false;
                    for (int j = 0; j < strlen(statOrder); j++)
                    {
                        if (statOrder[j] == 'v') {
                            memmove(&statOrder[j], &statOrder[j + 1], strlen(statOrder) - j);
                        }
                    }
                }
                else {
                    showV = true;
                    int len = strlen(statOrder);
                    statOrder[len] = 'v';
                    statOrder[len+1] = '\0';
                }
                
            }
            if (!strcmp(argv[i], "-i")) {
                if(showI)
                {
                    showI = false;
                    for (int j = 0; j < strlen(statOrder); j++)
                    {
                        if (statOrder[j] == 'i') {
                            memmove(&statOrder[j], &statOrder[j + 1], strlen(statOrder) - j);
                        }
                    }
                }
                else {
                    showI = true;
                    int len = strlen(statOrder);
                    statOrder[len] = 'i';
                    statOrder[len+1] = '\0';
                }
               
            }
        }

        printf("The new stat order is: %s\n", statOrder);
        return 1;
    }

    if (!strcmp(argv[0], "quit")) /* quit command */
	exit(0);  

    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
    return 1;

    return 0;                     /* Not a builtin command */
}

int main()
{
        char cmdline[MAXLINE]; /* Command line */


        signal(SIGINT, sigint_handler); // CTRL-C Signal
        signal(SIGTSTP, sigtstp_handler); // CTRL-Z Signal
        signal(SIGCONT, sigtstp_handler);
        //dup2(1, 2);
        while (isRunning) {
            /* Read */
            printf ("lsh> ");
            fgets(cmdline, MAXLINE, stdin);
            if (feof(stdin))
                exit(0);

            /* Evaluate */
            eval(cmdline);
        }
}