#include "misc.h"

typedef struct blob {
    float old_x, old_y;
    float posx, posy;
    float vx, vy;
    float rad;
    int color;    
    bool valid;
    bool delete_now;
    bool filled;
} BLOB;

void BLOB_init(BLOB* b);
void BLOB_init(BLOB* b, int rad);
void BLOB_init(BLOB* b, int rad, int color);
void BLOB_init(BLOB* b, int rad, int color, bool filled);


void BLOB_constrain2world(BLOB* b);
void BLOB_print(BLOB b);
float BLOB_dist2(BLOB b1, BLOB b2);