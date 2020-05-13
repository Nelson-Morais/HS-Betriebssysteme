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
    int numberOfCmd = 6 ;
    char *listOfCmd[numberOfCmd];
    int switchVar = 0;
    int errno;
    char *variable;


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
            if (parse[1] != NULL && parse[2] != NULL) {
                errno = setenv(parse[1], parse[2], 1);
            } else {
                printf("SYSTEM: Missing arguments. -- set 'arg1' 'arg2'\n");
            }
            if (errno != 0) {
                printf("SYSTEM: Cant't find Variable to edit.\n");
            }
            return 1;

        case 4:
            variable = getenv(parse[1]);
            if (variable != NULL) {
                printf("%s", parse[1]);
                printf(" = %s\n", variable);
            } else {
                printf("SYSTEM: Cant find Variable to export.\n");
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


int strHandler(char *input, char **parse) {

    strParse(input, parse);

    if (cmdHandler(parse)) {
        return 1;
    }
    return 0;
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
    user = getUser();
    host = getHost();

    initSettings();


    printf("Welcome to OhGee-Shell\n");
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


        type = strHandler(input, parse);

        if (type == 0) {
            execute(parse);
        }
    }

    return 0;

}



