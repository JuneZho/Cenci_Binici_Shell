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
//#define MAXLINE 8192
/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
extern char **environ;
char* retrieveEnvironVar();
bool isRunning = 1; 
bool showU = false, showS = false, showP = false, showV = false, showI = false, showA = false, showL = false, showC = false;
char statOrder[6] = "";
void unix_error(char *msg) /* Unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
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

        getrusage(RUSAGE_SELF, &usage);
        if (showU) startU = usage.ru_utime;
        if (showS) startS = usage.ru_stime;
        if (showP) startP = usage.ru_majflt;
        if (showV) startV = usage.ru_nvcsw;
        if (showI) startI = usage.ru_nivcsw;


        if (argv[0] == NULL)
            return;     /* Ignore empty lines */
        
        if (!builtin_command(argv)) {
            if ((pid = fork()) == 0) {  /*Child runs user job  */
                if (execvp(argv[0], argv) < 0) {
                    printf("%s: Command not found.\n", argv[0]);
                    exit(0);
                }
            } 

            /* Parent waits for foreground job to terminate */
            if (!bg) {
                int status;
                if (waitpid(pid, &status, 0) < 0)
                    unix_error("waitfg: waitpid error");
            }
            else
                printf("%d %s", pid, cmdline);           
        }

        getrusage(RUSAGE_SELF, &usage);
        for (int i = 0; i < strlen(statOrder); i++) {
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

        return;
}


char* retrieveEnvironVar(char* inputVariable) 
{
    return getenv(inputVariable);
}
/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    /*char *word = argv;
    printf("%d\n", (unsigned)strlen(argv));
    printf("%s\n", argv[0] + 2); // 2 chars away
    printf("%d\n", (unsigned)strlen(word));
    char *totalString ="";
    totalString = malloc((signed)strlen(argv));
    char **curPointer = argv;
    printf("%s\n", curPointer); // 2 chars away

    //while ()
    for (int i = 0; i < (signed)strlen(argv)+3; i++)
    {
        //strcpy(totalString, argv[0] + i);
        strcat(totalString, argv[0] + i);
        printf("%s\n", totalString);
    }*/
    
    /*char values[] = "";

    //strcat(values, argv[0] + 1);
    //printf("%s\n", values);
    //return 1;
    if (*(argv) != NULL)
    {
        for (int i = 0; i < 9; i++)
        {
            char *combo = malloc(strlen(*(argv[i]))+strlen(*(argv[i]+1))+1);
            strcpy(combo, *(argv[i]));
            strcat(combo, *(argv[i]+1));
            printf("%s\n", combo);
            return 1;
        }
        return 1;
    }
    printf("%s", *(argv));*/

    int lengthValue = strlen(argv[0]);
    for(int i = 0; i < lengthValue; i++) {

        if(*(argv[0]+i)== '=')
        {
            char* newVar = malloc(i+1);
            memcpy(newVar, *(&(argv[0])), i);

            char newVarsValue[(lengthValue - i)];
            memcpy(newVarsValue,  &*(argv[0]+i+1), lengthValue);
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
            setenv(newVar, newVarsValue, 0);
            argv[1] = newVar;
            return 1;
        }

    }

    if (*(argv[0]) == '$') /* Checks for the $ command */
    {
        char **followingString = argv[0] + 1;
        //printf("%s\n", followingString);
        //printf("%s\n", retrieveEnvironVar(followingString));
        char **a = retrieveEnvironVar(followingString); //
        printf("%s\n", retrieveEnvironVar(followingString));
        argv[1] = a;
        return 1;
    }
    
    if (argv[1] != NULL) {
        if (*(argv[1]) == '$') /* echo is read */
        {
                char **followingString = argv[1] + 1;
                char **a = retrieveEnvironVar(followingString); // 
                //argv[0] = malloc(strlen(4 + 1 + strlen(argv[0])));
                //strcat("/bin/", argv[0])f;
                argv[1] = a;
                return 0;
        }
    }
    
    /* Stats commands */
    if (!strcmp(argv[0], "stats"))
    { 
        if (!strcmp(argv[1], "-a")) {
            memset(statOrder, 0, sizeof statOrder);
            strcat(statOrder, "uspvi");
            showU = true;
            showS = true;
            showP = true;
            showV = true;
            showI = true;
            return 1;
        }
        if (!strcmp(argv[1], "-l")) {
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
        if (!strcmp(argv[1], "-c")) {
            memset(statOrder, 0, sizeof statOrder);
            showU = false;
            showS = false;
            showP = false;
            showV = false;
            showI = false;
            printf("The new stat order is: %s (END)\n", statOrder);
            return 1;
        }
        for (int i = 1; argv[i] != NULL; i++)
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

    //char* input = *(argv);
    if (strchr(argv, "1 | 2 | 3") != NULL)
    {
        char *pt;
        pt = strtok (*(argv),"|");
        while (pt != NULL) {
            printf("%s\n", pt);
            pt = strtok (NULL, "|");
        }
        return 1;
    }

    if (!strcmp(argv[0], "quit")) /* quit command */
	exit(0);  

    if (!strcmp(argv[0], "SIGHUP")) /* TERMINATE | Terminal line hangup */
    {
        printf("%s", "Command entered = SIGHUP\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGINT")) /* TERMINATE | Interrupt from keyboard */
    {
        printf("%s", "Command entered = SIGINT\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGQUIT")) /* TERMINATE | Quit from keyboard */
    {
        printf("%s", "Command entered = SIGQUIT\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGILL")) /* TERMINATE | Illegal Instruction */
    {
        printf("%s", "Command entered = SIGILL\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGTRAP")) /* TERMINATE & DUMP CORE| Trace trap */
    {
        printf("%s", "Command entered = SIGTRAP\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGABRT")) /* TERMINATE & DUMP CORE| Abort signal from abort function */
    {
        printf("%s", "Command entered = SIGABRT\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGBUS")) /* TERMINATE | Bus error */
    {
        printf("%s", "Command entered = SIGBUS\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGFPE")) /* TERMINATE & DUMP CORE| Floating-point exception */
    {
        printf("%s", "Command entered = SIGFPE\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGKILL")) /* TERMINATE | Kill program */
    {
        printf("%s", "Command entered = SIGKILL\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGUSR1")) /* TERMINATE | User-defined signal 1 */
    {
        printf("%s", "Command entered = SIGUSR1\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGSEGV")) /* TERMINATE & DUMP CORE| Invalid memory reference (seg fault) */
    {
        printf("%s", "Command entered = SIGSEGV\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGUSR2")) /* TERMINATE | User-defined signal 1 */
    {
        printf("%s", "Command entered = SIGUSR2\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGPIPE")) /* TERMINATE | Wrote to a pipe with no reader */
    {
        printf("%s", "Command entered = SIGPIPE\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGALRM")) /* TERMINATE | Timer signal from alarm function */
    {
        printf("%s", "Command entered = SIGALRM\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGTERM")) /* TERMINATE | Software termination signal */
    {
        printf("%s", "Command entered = SIGTERM\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGSTKFLT")) /* TERMINATE | Stack fault on coprocessor */
    {
        printf("%s", "Command entered = SIGSTKFLT\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGCHLD")) /* IGNORE | A child process has stopped or terminated */
    {
        printf("%s", "Command entered = SIGCONT\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGCONT")) /* IGNORE | Continue process if stopped  */
    {
        printf("%s", "Command entered = SIGCONT\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGSTOP")) /* STOP UNTIL NEXT SIGCONT | Stop signal not from terminal */
    {
        printf("%s", "Command entered = SIGSTOP\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGTSTP")) /* STOP UNTIL NEXT SIGCONT | Stop signal from terminal */
    {
        printf("%s", "Command entered = SIGTTIN\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGTTIN")) /* STOP UNTIL NEXT SIGCONT | Background process read from terminal */
    {
        printf("%s", "Command entered = SIGTTIN\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGTTOU")) /* STOP UNTIL NEXT SIGCONT | Background process wrote to terminal */
    {
        printf("%s", "Command entered = SIGTTOU\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGURG")) /* IGNORE | Urgent condition on socket */
    {
        printf("%s", "Command entered = SIGURG\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGXCPU")) /* TERMINATE | CPU time limit exceeded */
    {
        printf("%s", "Command entered = SIGXCPU\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGXFSZ")) /* TERMINATE | File size limit exceeded */
    {
        printf("%s", "Command entered = SIGXFSZ\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGVTALRM")) /* TERMINATE | Virtual timer expired */
    {
        printf("%s", "Command entered = SIGVTALRM\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGPROF")) /* TERMINATE | Profiling timer expired */
    {
        printf("%s", "Command entered = SIGPROF\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGWINCH")) /* IGNORE | Window size changed */
    {
        printf("%s", "Command entered = SIGWINCH\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGIO")) /* TERMINATE | I/O now possible on a descriptor */
    {
        printf("%s", "Command entered = SIGIO\n");
        return 1;
    }
    if (!strcmp(argv[0], "SIGPWR")) /* TERMINATE | Power failure */
    {
        printf("%s", "Command entered = SIGPWR\n");
        return 1;
    }
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
    return 1;

    return 0;                     /* Not a builtin command */
}

void InterruptHandler(int signal)
{
    isRunning = true;
}
int main()
{
        char cmdline[MAXLINE]; /* Command line */

        signal(SIGINT, InterruptHandler);

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