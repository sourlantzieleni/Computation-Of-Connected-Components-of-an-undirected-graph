
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "mmio.h"
#include "converter.h"

// Struct to pass data to each thread
typedef struct {
    int thread_id;   
    int start, end;  
    int *indexes;
    int *indices;
    int *colors;
    int *changed;  
} 
thread_data_t;

struct timespec t_start, t_end; 

void *color_thread(void *arg) {
    thread_data_t *data = (thread_data_t*)arg;
    int local_changed = 0;

    for(int i = data->start; i < data->end; i++) {

        for(int j = data->indexes[i]; j < data->indexes[i+1]; j++) {

            int cmin;

            if (data->colors[i] < data->colors[data->indices[j]]) {
                cmin = data->colors[i];
            } else {
                cmin = data->colors[data->indices[j]];
            }

            if (data->colors[i] != cmin) {
                data->colors[i] = cmin;
                local_changed = 1;
            }

            if (data->colors[data->indices[j]] != cmin) {
                data->colors[data->indices[j]] = cmin;
                local_changed = 1;
            }
        }
        
    }

    *(data->changed) = local_changed;
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    char *str = argv[1];
    int *indexes;
    int *indices;

    int N = cooReader(str, &indexes, &indices) - 1;

    int *colors = (int*)malloc(N * sizeof(int));

    for(int i = 0; i < N; i++) colors[i] = i;

    int num_threads = 16;
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];

    int active = 1;

    clock_gettime(CLOCK_REALTIME, &t_start);

    while(active) {
        active = 0;
        int thread_changed[num_threads];
        for(int t = 0; t < num_threads; t++) thread_changed[t] = 0;

        int chunk = (N + num_threads - 1) / num_threads;

        for(int t = 0; t < num_threads; t++) {
            thread_data[t].thread_id = t;
            thread_data[t].start = t * chunk;
            thread_data[t].end = (t+1) * chunk;
            if(thread_data[t].end > N) thread_data[t].end = N;

            thread_data[t].indexes = indexes;
            thread_data[t].indices = indices;
            thread_data[t].colors = colors;
            thread_data[t].changed = &thread_changed[t];

            pthread_create(&threads[t], NULL, color_thread, &thread_data[t]);
        }

        for(int t = 0; t < num_threads; t++)
            pthread_join(threads[t], NULL);

        for(int t = 0; t < num_threads; t++)
            if(thread_changed[t]) active = 1;
    }

    clock_gettime(CLOCK_REALTIME, &t_end);


    int numCC = 0;
    int *uniqueFlags = (int*)calloc(N, sizeof(int));

    for(int i = 0; i < N; i++) {
        if(!uniqueFlags[colors[i]]) {
            uniqueFlags[colors[i]] = 1;
            numCC++;
        }
    }

    printf("Number of connected components: %d\n", numCC);

    double duration = ((t_end.tv_sec - t_start.tv_sec) * 1000000 +
                       (t_end.tv_nsec - t_start.tv_nsec) / 1000) / 1000000.0;
    printf("~ CC duration: %lf seconds\n", duration);

    //save results in csv
    FILE *f = fopen("results.csv", "a");
    fprintf(f, "pthreads,%d,%lf,%s,%d\n", num_threads, duration, str, N);
    fclose(f);


    free(uniqueFlags);
    free(colors);

    return 0;
}
