#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>

//create global variable semaphore for access inside function
sem_t mutex;
pthread_t *threads;
typedef struct {
    int tid;
    char* file;
} thread_data;

//find prime number in each file
int main(int argc, char **argv){
    char *dirname = argv[1];
    int thread_count = atoi(argv[2]);

    if(thread_count == 0){
        printf("No threads allocated. Terminating.\n");
        return 0;
    }
    thread_data td[thread_count];
    DIR *directory = opendir(dirname);
    struct dirent *entry;
    int filecount = 0;
    char* filenames = malloc(340);
    while((entry = readdir(directory)) != NULL){
        snprintf(filenames[filecount], sizeof(filenames), "myDir/%s", entry->d_name);
        filecount++;
    }
    sem_init(&mutex, 0, thread_count);
    threads = malloc(sizeof(pthread_t)*thread_count);
    for(int i = 0; i < thread_count; i++){
        pthread_t thread;
        thread_data data;
        data.tid = i;
        data.file = filenames;
        td[i] = data;
        threads[i] = thread;
    }
    for(int i = 0; i < thread_count; i++){
        pthread_create(&threads[i], NULL, countPrime, (void*) filenames);
    }
}  

void* countPrime(void* f){
    FILE *files[30] = (FILE*)f;


}