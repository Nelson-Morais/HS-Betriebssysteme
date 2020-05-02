#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <string.h>
#include "include/web_request.c"

#define maxChar 256
#define QUEUESIZE 20

int threadsize = 10;
char input[maxChar];
char *parse[maxChar];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char *buf[QUEUESIZE];
    long head, tail;
    int full, empty;

} queue;

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

    return (q);
}

void queueDelete(queue *q) {
    free(q);
}

void queueDel(queue *q, char *out) {
    *out = q->buf[q->head];

    q->head++;
    if (q->head == QUEUESIZE)
        q->head = 0;
    if (q->head == q->tail)
        q->empty = 1;
    q->full = 0;

    return;
}

char *queueRead(queue *q) {
    char *out = q->buf[q->head];

    q->head++;
    if (q->head == QUEUESIZE)
        q->head = 0;
    if (q->head == q->tail)
        q->empty = 1;
    q->full = 0;

    return out;
}

void queueAdd(queue *q, char *in) {
    q->buf[q->tail] = strdup(in);
    q->tail++;
    if (q->tail == QUEUESIZE)
        q->tail = 0;
    if (q->tail == q->head)
        q->full = 1;
    q->empty = 0;

    return;
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

void *readFd(void *q) {

    queue *q2 = (queue *) q;

    FILE *fd_in;
    fd_in = fopen(parse[0], "r");
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
        queueAdd(q2, url);
        pthread_mutex_unlock(&lock);

    }

    return NULL;
}

void *writeFd(void *q) {
    queue *tmp = (queue *) q;
    char *argv[2];
    argv[0] = "--webreq-delay 100";
    argv[1] = "--webreq-path download";

    webreq_init(2, argv);
    int i = 0;

    printf("Thread ist hier %i",i+1);
    pthread_mutex_lock(&lock);
    while (!(tmp->empty)) {
        char *url = strdup(queueRead(q));
        char *downloadUrl = strdup(url);
        strtok(url, "/");
        char *domain = strtok(NULL, "/");

        char filename[64];
        snprintf(filename, sizeof(filename), "%i_%s.html", i++, domain);
        printf("Downloading URL: %s", downloadUrl, filename);
        printf(" Thread ist raus %i",i+1);
        webreq_download(downloadUrl,filename);


    }
    pthread_mutex_unlock(&lock);
    printf("Thread ist raus %i",i+1);


}


void multiThread(void (*f)(void*), int n){

    pthread_t threadArr[n];

    for(int i = 0; i < sizeof(threadArr);i++){
        pthread_create(&threadArr[i],NULL,f,NULL);
    }
    for(int i = 0; i < sizeof(threadArr); i++){
        pthread_join(&threadArr[i],NULL);
    }

}

int main() {
    char **args;
    pthread_t th;


    queue *q = NULL;

    printf("Filename:");
    fgets(input, MAX_INPUT, stdin);
    input[strcspn(input, "\n")] = '\0';
    if (input[0] == NULL || *input == ' ') {


    } else {
        strParse(input, parse);
    }

    q = queueInit();

    pthread_create(&th, NULL, readFd, q);
    pthread_join(th, NULL);


    // code ohne threads //
    //readFd(q);



    multiThread(writeFd(q),2);

//    queuePrint(q);
    return 0;

}

