#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cilk/cilk.h>
#include "mmio.h"
#include "converter.h"

struct timespec t_start, t_end;

int main(int argc, char *argv[])
{
    char *str = argv[1];
    int  *indexes;
    int  *indices;

    int N = cooReader(str,  &indexes, &indices)-1;//reverted

    int *colors=(int*)malloc(N*sizeof(int));
   
    int active = 1;

    for(int i=N; i>0; i--){
        colors[N-i]=i;
    }
    
    clock_gettime(CLOCK_REALTIME, &t_start);

    while (active) {
        active = 0;

        cilk_for (int i = 0; i < N; ++i) {
            for (int j = indexes[i]; j < indexes[i+1]; ++j) {
                
                int cmin;
                if(colors[i]<colors[indices[j]]){
                    cmin = colors[i];
                }
                else{
                    cmin = colors[indices[j]];
                }


                if(colors[i] != cmin){
                    colors[i] = cmin;
                    active = 1;
                }
                if(colors[indices[j]] != cmin){
                    colors[indices[j]] = cmin;
                    active = 1;
                }
            }
        }
    }

    clock_gettime(CLOCK_REALTIME, &t_end);


    int numCC = 0;
    int *uniqueFlags = (int*)calloc(N, sizeof(int));

    for (int i = 0; i < N; i++) {
        int color = colors[i];
        if (!uniqueFlags[color]) {
            uniqueFlags[color] = 1;
            numCC++;
        }
    }

    printf("Number of connected components: %d\n", numCC);

    double duration = ((t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_nsec - t_start.tv_nsec) / 1000) / 1000000.0;
    printf("~ V3 duration: %lf seconds\n", duration);

    
    char *workers_env = getenv("CILK_NWORKERS");
    int workers = (workers_env != NULL) ? atoi(workers_env) : 0;

    //save results in csv
    FILE *f = fopen("results.csv", "a");
    fprintf(f, "opencilk,%d,%lf,%s,%d\n", workers, duration, str,N);
    fclose(f);
    

    free(colors);
    free(uniqueFlags);

    return 0; 
}
