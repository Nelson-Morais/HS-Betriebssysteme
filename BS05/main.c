#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <values.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>


int maxChar = 1024;
int maxArgs = 100;


int numberSettings = 2;

char *user;
char *host;
char *settingsPtr[2];
char *settingsName[2];
extern char **environ;

void initSettings() {

    settingsName[0] = "user";
    settingsName[1] = "host";

}

void settings(char **parse) {
//BUG: pointers user/host zeigen jetzt auf parse[3] statt wert zu übernehmen.
    int switchVar = 0;
    for (int i = 0; i < numberSettings; i++) {
        if ((strcmp(parse[2], settingsName[i])) == 0) {
            switchVar = i + 1;
        }
    }
    switch (switchVar) {
        case 1:
            user = parse[3];
            break;
        case 2:
            host = parse[3];
            break;
        default:
            break;
    }
}

void help() {
    printf("\n*********HELP*********\n\n\n");

    printf("exit  -  ");
    printf("Closes the shell.\n\n");
    printf("options 'arg1' 'arg2'  -  ");
    printf("See and modify Shell Options.\n\n");
    printf("cd 'arg1'  -  ");
    printf("Change directory\n\n");

    printf("\n*********END OF FILE*********\n\n\n");

}

int options(char **parse) {
    int numberOfOpts = 4;


    char *listOfOpts[numberOfOpts];
    int switchVar;
    listOfOpts[0] = "help";
    listOfOpts[1] = "all";
    listOfOpts[2] = "set";
    listOfOpts[3] = "get";

    for (int i = 0; i < numberOfOpts; i++) {
        if (strcmp(parse[1], listOfOpts[i]) == 0) {
            switchVar = i + 1;
            break;
        }
    }

    switch (switchVar) {
        case 1:
            printf("\n*********HELP - options*********\n\n");

            printf("all  -  ");
            printf("Shows all availabe options.\n\n");
            printf("set [option] [value]  -  ");
            printf("Change setting to a specific value.\n\n");

            printf("Shell created by: NELSON MORAIS\n");
            printf("\n*********END OF FILE*********\n\n\n");
            return 1;
        case 2:
            printf("\n*********LIST - options*********\n\n\n");
            printf("user - change Shell current user name.\n\n");
            printf("host - Change Shell currtent host name.\n\n");
            printf("\n*********END OF FILE*********\n\n\n");
            return 1;
        case 3:
            if (parse[2] != NULL && parse[3] != NULL) {
                settings(parse);

            } else {
                printf("SYSTEM: Missing arguments. -- set 'arg1' 'arg2'\n");
            }
            break;

        case 4:

            printf("\n*********LIST - Settings*********\n\n");
            for (int i = 0; i < numberSettings; i++) {
                printf("%s", settingsName[i]);
                printf("=");
                printf("%s\n", settingsPtr[i]);
            }
            printf("\n*********END OF FILE*********\n\n\n");
            break;

        default:

            break;
    }

    return 0;


}

int executePipe(char** parse, char** parsePipe){
    int fdes[2];
    pid_t cid1, cid2, waitID1, waitID2;
    int i = 0;
    int j = 0;
    int status;



    if (pipe(fdes) < 0){
        printf("Pipe 1 did not work\n");
    }
    if((cid1 = fork()) < 0){
        perror("fork");
    }




   if(cid1 == -1){
       perror("Fork Failed (Return value -1) \n");
       exit(EXIT_FAILURE);
   }else if(cid1 == 0){

       close(fdes[0]);
       dup2(fdes[1],STDOUT_FILENO);
       close(fdes[1]);
       if((execvp(parse[0], parse)) == -1){
           printf("Can't find command 1\n", *parse);
       }
   }else{
       if ((cid2 = fork())==-1){
           perror("Fork Failed (Return value -1) \n");
           exit(EXIT_FAILURE);
       }else if(cid2 == 0){


           close(fdes[1]);
           dup2(fdes[0], STDIN_FILENO);
           close(fdes[0]);
           if((execvp(parsePipe[0], parsePipe)) == -1){
               printf("Can't find command 2\n", *parse);
           }
       }else{
           close(fdes[0]);
           close(fdes[1]);

       }



       while (i != -1) {
           waitID1 = waitpid(cid1, &status, WNOHANG | WUNTRACED);

           if (waitID1 == -1) {
               i = i-1;
           }
           if (waitID1 == cid1) {
               i = i-1;
           }

       }
       while (j != -1) {

           waitID2 = waitpid(cid2,&status,WNOHANG | WUNTRACED);
           if (waitID2 == -1) {
               j = j-1;
           }
           if (waitID2 == cid1) {
               j = j-1;
           }
       }

   }
   return 1;

}

int execute(char **parse) {
    pid_t cid, waitID;
    int status;
    int now;

    if ((cid = fork()) == -1) {
        perror("Fork Failed (Return value -1) \n");
        exit(EXIT_FAILURE);
    } else if (cid == 0) {

        if ((execvp(parse[0], parse)) == -1) {
            printf("Can't find command\n", *parse);
        }
    }
    int i;
    while (i != -1) {
        waitID = waitpid(cid, &status, WNOHANG | WUNTRACED);
        if (waitID == -1) {
            i = -1;
        }
        if (waitID == cid) {
            i = -1;
        }
    }
    return 0;
}

int cmdHandler(char **parse) {
    int numberOfCmd = 6;
    char *listOfCmd[numberOfCmd];
    int switchVar = 0;
    int errno = 0;
    static char *variable;


    listOfCmd[0] = "exit";
    listOfCmd[1] = "cd";
    listOfCmd[2] = "set";
    listOfCmd[3] = "export";
    listOfCmd[4] = "options";
    listOfCmd[5] = "help";

    for (int i = 0; i < numberOfCmd; i++) {
        if (strcmp(parse[0], listOfCmd[i]) == 0) {
            switchVar = i + 1;
            break;
        }
    }
    switch (switchVar) {
        case 1:
            printf("SYSTEM: ******CLOSING******\n");
            exit(0);

        case 2:
            errno = chdir(parse[1]);
            if (errno != 0) {
                printf("SYSTEM: Can't change to that Directory.\n");
            }
            return 1;

        case 3:
            if (parse[1] != NULL) {
                variable = getenv(parse[1]);

                if (variable != NULL) {
                    printf("%s", parse[1]);
                    printf("=%s\n", variable);
                } else {
                    printf("Can't find variable.");
                }
            } else {
                for (char **env = environ; *env; ++env) {
                    printf("%s\n", *env);
                }

            }
            return 1;
        case 4:

            if (parse[1] != NULL) {
                char *export[maxArgs];
                export[0] = strtok(parse[1], "=");

                for (int i = 1; i < maxArgs; i++) {
                    export[i] = strtok(NULL, "=");
                    if (export[i] == NULL) {
                        break;
                    }
                }
                if (export[0] != NULL && export[1] != NULL) {
                    errno = setenv(export[0], export[1], 1);
                } else {
                    printf("SYSTEM: Missing arguments. -- export 'arg1'='arg2'\n");
                }
                if (errno == -1) {
                    printf("SYSTEM: ERROR(-1).\n");
                }
            }else {
                printf("SYSTEM: Missing arguments. -- export 'arg1'='arg2'\n");
            }
            return 1;


        case 5:
            variable = parse[1];
            if (variable != NULL) {
                options(parse);

            } else {
                printf("\nSYSTEM: arguments for command ");
                printf("%s", parse[0]);
                printf(" missing, type 'options help' for more info.\n\n");
            }

            return 1;

        case 6:
            help();
            return 1;
        default:

            break;
    }


    return 0;
}

char strParse(char *input, char **parse) {

    for (int i = 0; i < maxArgs; i++) {
        parse[i] = strsep(&input, " ");
        if (parse[i] == NULL) {
            break;
        }
        if (strlen(parse[i]) == 0) {
            i--;
        }
    }
}

int pipeParse(char *input, char **inputPiped){
    for(int i = 0; i < 2; i++){
        inputPiped[i] = strsep(&input,"|");
        if (inputPiped[i] == NULL){
            break;
        }
    }

    if (inputPiped[1] == NULL){
        return 0;
    } else{
        return 1;
    }
}


int strHandler(char *input, char **parse, char **parsePipe) {

    char *inputPiped[2];
    int isPiped = 0;



    isPiped = pipeParse(input, inputPiped);

    if(isPiped){
        strParse(inputPiped[0], parse);
        strParse(inputPiped[1], parsePipe);
    }else {

        strParse(input, parse);
    }

    if (cmdHandler(parse)) {
        return 0;
    }
    return 1+isPiped;
}

char *getDir() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char *dir = cwd;
    return dir;
}


char *getUser() {

    struct passwd *p = getpwuid(getuid());
    return p->pw_name;
}

char *getHost() {

    char hostbuffer[HOST_NAME_MAX + 1];
    int hostname;
    struct hostent *host;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    host = gethostbyname(hostbuffer);

    return host->h_name;
}

int main() {
    char *parse[maxArgs];
    char input[maxChar];
    char *parsePipe[maxArgs];
    user = getUser();
    host = getHost();

    initSettings();


    printf("Welcome to Nelsons-Shell\n");
    while (1) {
        char *dir = getDir();
        int type = 0;


        printf("%s", user);
        printf("@");
        printf("%s", host);
        printf(":");
        printf(dir);
        printf("$ ");

        fgets(input, MAX_INPUT, stdin);
        input[strcspn(input, "\n")] = '\0';
        if (input[0] == NULL || *input == ' ') {
            continue;

        }


        type = strHandler(input, parse, parsePipe);

        if (type == 1) {
            execute(parse);
        }
        if (type == 2){
            executePipe(parse,parsePipe);
        }
    }

    return 0;

}



