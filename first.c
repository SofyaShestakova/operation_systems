#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include<sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <limits.h>

#define A (316<<20)
#define B 0x7153C850
#define C malloc
#define D 84
#define E (197<<20)
#define F nocache
#define G 112
#define H random
#define I 146
#define J min
#define K sem


sem_t semaphore;

void *memory_start;

char result_name[13];
const int file_size = E;
const int files_amount = A / E + (A % E > 0 ? 1 : 0);

void create_part_of_memory() {
    int mem_size = A;
    int threads_amount = D;

    int mem_size_per_thread = mem_size / threads_amount;

    void *start = (char *) malloc(mem_size);
    FILE *urandom = fopen("/dev/urandom", "r");

    int thread_idx = 0;
    void *thread_func() {
        sem_wait(&semaphore);
        int cur_thread_idx = thread_idx++;

        void *mem_start = start + cur_thread_idx * mem_size_per_thread;
        int effective_mem_size = mem_size_per_thread;

        fread((void *) start, 1, effective_mem_size, urandom);

        sem_post(&semaphore);
    }

    pthread_t threads[D];
    for (int i = 0; i < D; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }
    for (int i = 0; i < D; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(urandom);
    memory_start = start;
}

void destroy_part_of_memory() {
    free(memory_start);
}

void write_files() {
    int block_size = G;
    int file_blocks_amount = E / G + (E % G > 0 ? 1 : 0);

    int cur_file_idx = 0; // index of a current file
    for (;;) {
        if (cur_file_idx == files_amount) break;

        sem_wait(&semaphore);

        snprintf(result_name, 13, "lab_os_%d.bin", cur_file_idx);

        int fd = open(result_name, (O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT));
        for (int i = 0; i < file_blocks_amount; i++) {
            write(fd, memory_start + (cur_file_idx * file_blocks_amount + i) * block_size, block_size);
        }
        close(fd);

        sem_post(&semaphore);

        cur_file_idx++;
    }
}

void read_and_search_min() {
    int threads_amount = I;
    pthread_t new_threads[threads_amount];
    int min = INT_MAX;

    void *thread_func() {
        int *numbers = malloc(file_size);
        for (int i = 0; i < files_amount; i++) {
            snprintf(result_name, 13, "lab_os_%d.bin", i);
            FILE *file = fopen(result_name, "r");

            int read_numbers = fread((void *) numbers, 1, file_size, file) / sizeof(int);
            for (int j = 0; j < read_numbers; ++j) {
                int number = numbers[j];
                sem_wait(&semaphore);
                if (number < min) {
                    min = number;
                }
                sem_post(&semaphore);
            }
        }
    }

    for (int i = 0; i < threads_amount; i++) {
        pthread_create(&new_threads[i], NULL, *thread_func, NULL);
    }
    for (int i = 0; i < threads_amount; i++) {
        pthread_join(new_threads[i], NULL);
    }
}

int main() {
    sem_init(&semaphore, 0, 1);

    create_part_of_memory();
    write_files();
    read_and_search_min();
    destroy_part_of_memory();

    sem_destroy(&semaphore);

    return 0;
}