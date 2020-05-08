/*Code by:
 * NELSON MORAIS
 * HS Osnabrück
 * Betreibssysteme Praktikum 06
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <string.h>
#include "include/web_request.c"
#include <sys/time.h>


#define maxChar 256
#define QUEUESIZE 30
#define anzahlThreads 8

int threadsize = 10;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
int count = 1;

typedef struct {
    char *buf[QUEUESIZE];
    long head, tail;
    int full, empty, eof;
} queue;

typedef struct {
    queue *q;
    int id;
} argWrite;

typedef struct {
    char *dateiname;
    queue *q;
} argRead;



int queueSize(queue *q) {
    int rtr = (q->tail) - (q->head);
    return rtr;
}

char *queuePrint(queue *q) {
    int i = q->head;
    int j = q->tail;
    char *tmp;
    while (i != j) {
        tmp = q->buf[i++];
        printf("%s\n", tmp);
    }
}

queue *queueInit(void) {
    queue *q;

    q = (queue *) malloc(sizeof(queue));
    if (q == NULL) return (NULL);

    q->empty = 1;
    q->full = 0;
    q->head = 0;
    q->tail = 0;
    q->eof = 0;

    return (q);
}

void queueDelete(queue *q) {
    free(q);
}

void queueDel(queue *q, char *out) {
    *out = (char) q->buf[q->head];

    q->head++;
    if (q->head == QUEUESIZE)
        q->head = 0;
    if (q->head == q->tail)
        q->empty = 1;
    q->full = 0;


}

char *queueRead(queue *q) {

   if(!q->empty) {
       char *out = q->buf[q->head];

       q->head++;

       if (q->head == q->tail)
           q->empty = 1;
       q->full = 0;


       return out;
   }
}

void queueAdd(queue *q, char *in) {

    if(q->tail < QUEUESIZE ) {
        q->buf[q->tail] = strdup(in);
        q->tail++;
    }
    if (q->tail == QUEUESIZE)
        q->full = 1;
    q->empty = 0;
}

char strParse(char *input, char **parse) {

    for (int i = 0; i < maxChar; i++) {
        parse[i] = strsep(&input, " ");
        if (parse[i] == NULL) {
            break;
        }
        if (strlen(parse[i]) == 0) {
            i--;
        }
    }
}

void *readFd(void *p) {

    argRead *p2 = (argRead *) p;

    FILE *fd_in;
    fd_in = fopen(p2->dateiname, "r");
    if (fd_in == NULL) {
        perror("failed to open input file");
        exit(EXIT_FAILURE);
    }

    char url[maxChar];
    char *pos;

    while (fgets(url, sizeof(url), fd_in) != NULL) {
        if ((pos = strchr(url, '\n')) != NULL) {
            *pos = '\0';
        }
        pthread_mutex_lock(&lock);
        queueAdd(p2->q, url);

        pthread_mutex_unlock(&lock);
        pthread_mutex_destroy(&lock);
    }
    p2->q->eof = 1;
    pthread_cond_signal(&condition_var);


    return NULL;
}

void *writeFd(void *p) {
    argWrite *tmp = (argWrite *) p;
    int id = tmp->id;
    char *argv[2];


    argv[0] = "--webreq-delay 0";
    argv[1] = "--webreq-path download";

    webreq_init(2, argv);




    while (1) {


        pthread_mutex_lock(&lock);
        if(tmp->q->empty && !tmp->q->eof) {
            pthread_cond_wait(&condition_var, &lock);
        }
        if(tmp->q->empty && tmp->q->eof){

            pthread_cond_signal(&condition_var);
            pthread_mutex_unlock(&lock);
            pthread_exit("");
            }
        char *url = strdup(queueRead(tmp->q));
        char *downloadUrl = strdup(url);
        pthread_mutex_unlock(&lock);
        pthread_mutex_destroy(&lock);


        strtok(url, "/");
        char *domain = strtok(NULL, "/");

        char filename[64];
        snprintf(filename, sizeof(filename), "%i_%i_%s.html", count++,id, domain);
        printf("Thread id: %d",id);
        printf(" Downloading URL: %s\n", downloadUrl);

        webreq_download(downloadUrl, filename);

}


    }





int main() {
    char input[maxChar];
    char *parse[maxChar];
    queue *q = NULL;
    q = queueInit();


    printf("Filename:");
    fgets(input, MAX_INPUT, stdin);
    input[strcspn(input, "\n")] = '\0';
    if ((void *) input[0] == NULL || *input == ' ') {
        printf("No input\n");
        exit(0);
    } else {
        strParse(input, parse);
    }


    argRead *ar = (argRead * )malloc(sizeof(argRead));

    ar->dateiname = parse[0];
    ar->q = q;

    pthread_t th;
    pthread_create(&th, NULL, readFd, ar);



    printf("Anzahl threads: %i\n",anzahlThreads);
    pthread_t threadArr[anzahlThreads];


    struct timeval tvbegin, tvend;
    gettimeofday(&tvbegin,NULL);





    for (int i = 0; i < anzahlThreads; i++) {
        printf("Create thread %d\n", i+1);
        argWrite *aw = (argWrite*) malloc(sizeof(argWrite));
        aw->q = q;
        aw->id = i + 1;
        pthread_create(&threadArr[i], NULL, writeFd, aw);
    }

    for (int i = 0; i < anzahlThreads; i++) {
        pthread_join(threadArr[i], NULL);
    }
    pthread_join(th, NULL);

    gettimeofday(&tvend, NULL);

    printf("%lu", (tvend.tv_sec - tvbegin.tv_sec)*1000 +(tvend.tv_usec-tvbegin.tv_usec)/1000); //print duration
    printf("ms\n");

    return 0;

}

