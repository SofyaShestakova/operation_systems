#include <stdio.h>
#include<sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define A 316*2^20
#define B 0x7153C850
#define C malloc
#define D 84
#define E 197*2^20
#define F nocache
#define G 112
#define H random
#define I 146
#define J min
#define K sem

void* memory_start;

void createPartOfMemory() {
    //до аллокации
    char *start = (char *) malloc(A);
    //после аллокации
    FILE *urandom = fopen("/dev/urandom", "r");
    void *thread_func() { fread((void *) start, 1, A, urandom); }
    pthread_t threads[D];
    for (int i = 0; i < D; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }
    for (int i = 0; i < D; i++) {
        pthread_join(&threads[i], NULL);
    }
    fclose(urandom);
    // после заполнения участка данными
    memory_start = start;
}

void destroyPartOfMemory(){
    free(memory_start);
}

void write_files() {
    int block_size = G;
    int files_amount = A / E + (A % E > 0 ? 1 : 0);
    int file_size = E;
    int file_blocks_amount = E/G + (E% G > 0 ? 1 : 0);

    int cur_file_idx = 0; // index of a current file
    for (;;) {
        if(cur_file_idx==file_size) break;
        char result_name[13];
        snprintf(result_name,13, "lab_os_%d.bin", cur_file_idx);
        int flag = O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT;
        FILE * file  = fopen(result_name, flag);
        for(int i=0;i<file_blocks_amount;i++){
            fwrite( memory_start + (cur_file_idx*file_blocks_amount+i)*block_size, 1, block_size, file);
        }
        cur_file_idx++;
    }
}


void main() {
        createPartOfMemory();
        write_files();
        destroyPartOfMemory();

    }