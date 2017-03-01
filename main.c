#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "primitives.h"
#include "raytracing.h"
#include "main.h"
#define OUT_FILENAME "out.ppm"

#define ROWS 512
#define COLS 512
#define THREADNUM 4

pthread_mutex_t gMutex;

void GetPixelIndex(unsigned long *reValue)
{
    static unsigned long iPixels=-1;
    pthread_mutex_lock(&gMutex);
    iPixels+=1;
    pthread_mutex_unlock(&gMutex);
    *reValue= iPixels;
}

static void write_to_ppm(FILE *outfile, uint8_t *pixels,
                         int width, int height)
{
    fprintf(outfile, "P6\n%d %d\n%d\n", width, height, 255);
    fwrite(pixels, 1, height * width * 3, outfile);
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}


int main()
{
    uint8_t *pixels;
    light_node lights = NULL;
    rectangular_node rectangulars = NULL;
    sphere_node spheres = NULL;
    color background = { 0.0, 0.1, 0.1 };
    struct timespec start, end;
    struct wrapped WrappedInfo;
    pthread_t allThreads[THREADNUM];
#include "use-models.h"

    /* allocate by the given resolution */
    pixels = malloc(sizeof(unsigned char) * ROWS * COLS * 3);
    if (!pixels) exit(-1);

    printf("# Rendering scene\n");
    /* do the ray tracing with the given geometry */
    clock_gettime(CLOCK_REALTIME, &start);
    /* Initilize the global mutex*/
    pthread_mutex_init(&gMutex,NULL);
    /* Wrap the information for thread */
    WrappedInfo.pix=pixels;
    memcpy(WrappedInfo.bac,background,sizeof(color));
    WrappedInfo.rec=rectangulars;
    WrappedInfo.sph=spheres;
    WrappedInfo.lig=lights;
    WrappedInfo.vie=&view;
    WrappedInfo.maxWid=COLS;
    WrappedInfo.maxHei=ROWS;

    for(int ithread=0; ithread <THREADNUM ; ithread++) {
        WrappedInfo.threadID=ithread;
        pthread_create(&allThreads[ithread],NULL,raytracing,(void*)&WrappedInfo);
    }
    for(int ithread=0; ithread <THREADNUM ; ithread++) {
        pthread_join(allThreads[ithread],NULL);
    }

    clock_gettime(CLOCK_REALTIME, &end);
    {
        FILE *outfile = fopen(OUT_FILENAME, "wb");
        write_to_ppm(outfile, pixels, ROWS, COLS);
        fclose(outfile);
    }

    delete_rectangular_list(&rectangulars);
    delete_sphere_list(&spheres);
    delete_light_list(&lights);
    free(pixels);
    printf("Done!\n");
    printf("Execution time of raytracing() : %lf sec\n", diff_in_second(start, end));
    return 0;
}
