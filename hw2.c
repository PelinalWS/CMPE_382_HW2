#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>

void* countPrime(void* f);
int isPrime(int n);

//create global variable semaphores for access inside function
sem_t mutex;    //mutex is for the max count of reading
sem_t limit;    //limit is to limit the access for the file-list reading
sem_t write_lock;
struct thread_data{
    int tid;        //is the thread id
    int pathindex;  //is the length of the path up to the filename
    char *file;     //is the concatanated name of the file with full relative path
};

int main(int argc, char **argv){        //reads argv as array of char arguments and saves the amount in argc, this will be fixed in this case
    char *dirname = argv[1];            //is the name of the directory
    int thread_count = atoi(argv[2]);   //is the amount of threads, changed from char to int

    if(thread_count == 0){              //if there are no threads, the program should not work
        printf("No threads allocated. Terminating.\n");
        return 0;
    }
    DIR *directory = opendir(dirname);  //opens the directory from the argument to read file names
    struct dirent *entry;               //a dirent struct is needed to access the entries
    sem_init(&mutex, 0, thread_count);  //initialize mutex for a max count of up to thread count
    sem_init(&limit, 0, 1);             //initialize limit to 0-1
    sem_init(&write_lock, 0, 1);
    pthread_t tid[thread_count];                        //create the threads as an array
    struct thread_data* data[thread_count];             //create the array for data needed in the functions
    FILE *fileList = fopen("./file_names.txt", "w");    //create a txt file to write the entry names in the directory
    while((entry = readdir(directory)) != NULL){        //while the entries are not finished
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
            fprintf(fileList, "%s/%s\n", dirname, entry->d_name);   //write into the file the name of entries
        }
    }
    fclose(fileList);                                   //the written file is at its last index, to return to the top we open it again with read access
    FILE *files = fopen("./file_names.txt", "r");       //opened the same file with read access instead of write access
	for(int i = 0; i < thread_count; i++){              //initialize the thread_data array
        data[i] = malloc(sizeof(struct thread_data));
        data[i]->tid = i+1;
        data[i]->file = malloc(strlen(dirname)+10);
        data[i]->pathindex = strlen(dirname) + 1;       //this is argv[1] + 1, is the same for all but it is better to pass an int value instead of char*
	}
    int t_index = 0;            //this is the thread index
    while(1){
        sem_wait(&mutex);                   //wait on the mutex until one of the threads are done operating
        sem_wait(&limit);                   //only a single thread should read the file at a time
        if(fscanf(files, "%s", data[t_index]->file)!=1){      //read the file and save the filename into textfile
        	break;
        }
        sem_post(&limit);                   //allow another thread to read
        pthread_create(&tid[t_index], NULL, countPrime, data[t_index]);
        t_index = (t_index + 1) % thread_count;
    }
    fclose(files);  //close file after use
    for(int i = 0; i < thread_count; i++){
        pthread_join(tid[i], NULL);
        free(data[i]);
    }
    closedir(directory);
}

void* countPrime(void* i){
    struct thread_data* data = (struct thread_data*)i;  //recast into thread_data
    int tid = data->tid;                    //get the specific tid for the output
    int filetext = data->pathindex;         //get the index in the string that the individual text name starts at
    char *path = data->file;                //get the text's path to open file
    FILE *file = fopen(path, "r");          //open file
    int val;                                //read values into val
    int prime = 0;                          //increment in case val is a prime number
    while(fscanf(file, "%d", &val) == 1){   //read until EOF
        if(isPrime(val)){                   //function call as a condition to increment prime
            prime++;
        }
    }
    sem_wait(&write_lock);
    printf("Thread %d has found %d primes in %s\n", tid, prime, path + filetext); //output
    sem_post(&write_lock);
    fclose(file);                           //close the file that's not going to be used anymore
    sem_post(&mutex);                       //free up a slot in the mutex so that a thread can pass through again.
}

int isPrime(int n){
    if(n <= 1) return 0;                //base case
    if(n == 2 || n == 3) return 1;      //most numbers can be divisible by 2 or 3,
    if(n % 2 || n % 3) return 0;        //it is more time efficient to check them first without opening a loop
    int known_prime[] = {5,7,11,13,17,19,23,29,31,37,39,41,43,47,53};
    for(int i = 0; i < sizeof(known_prime)/sizeof(int); i++){
        if(i*i > n) return 1;
        if(n == known_prime[i]) return 1;
        if(n % known_prime[i] == 0) return 0;
    }
    //it is more time efficient to manually check the primes for lower count primes rather than searching for them
    //as it is very unlikely that the number is a product of 2 very large numbers.
    for(int i = 59; i * i <= n; i += 2){//prime numbers start being divisable at 2, if the square of the supposed dividents is bigger, it shouldn't be divisable with a bigger number
        if((n % i) == 0) return 0;      //this is inefficient at cases with high ceilings for val.
    }
    return 1;                           //if not returned yet, it must be prime.
}
