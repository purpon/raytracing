#ifndef __MAIN_H
#define __MAIN_H

struct wrapped {
    uint8_t *pix;
    color bac;
    rectangular_node rec;
    sphere_node sph;
    light_node lig;
    const viewpoint *vie;
    int maxWid;
    int maxHei;
    int threadID;
};

extern pthread_mutex_t gMutex;
void GetPixelIndex(unsigned long*);

#endif
