#include "blob.h"
#include "mbed.h"
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>

extern Serial pc;


// Randomly initialize a blob's position, velocity, color, radius, etc.
// Set the valid flag to true and the delete_now flag to false.
// delete_now is basically the derivative of valid. It goes true for one
// fram when the blob is deleted, and then it is reset to false in the next frame
// when that blob is deleted.
void BLOB_init(BLOB* b)
{
    //std::srand(time(NULL)); // use current time as seed
    b->posx = (std::rand() % WORLD_WIDTH) - WORLD_WIDTH/2;
    b->posy = (std::rand() % WORLD_HEIGHT) - WORLD_HEIGHT/2;
    b->vx = std::rand() % 10 + 1;
    b->vy = std::rand()% 10 + 1;
    b->rad = std::rand() % 10 + 1;
    b->valid = true;
    b->delete_now = false;
    b->filled = false;
    pc.printf("X pos: %f\n", b-> posx);         // Let us know you made it this far
    pc.printf("Y pos: %f\n", b-> posy);         // Let us know you made it this far


}


// Take in a blob and determine whether it is inside the world.
// If the blob has escaped the world, put it back on the edge
// of the world and negate its velocity so that it bounces off
// the boundary. Use WORLD_WIDTH and WORLD_HEIGHT defined in "misc.h"
void BLOB_constrain2world(BLOB* b)
{
    if (b->posx > WORLD_WIDTH/2) {
        b->vx = -100000000;

    } else if (b->posx < WORLD_WIDTH/-2)  {
        b->vx = 100000000;

    }
    if (b->posy > WORLD_HEIGHT/2) {
        b->vy = -100000000;

    } else if (b->posy < WORLD_HEIGHT/-2)  {
        b->vy = 100000000;

    }
}

// Randomly initialize a blob. Then set the radius to the provided value.
void BLOB_init(BLOB* b, int rad)
{
    BLOB_init(b);
    b->rad = rad;
}

// Randomly initialize a blob. Then set the radius and color to the
// provided values.
void BLOB_init(BLOB* b, int rad, int color)
{
    BLOB_init(b);
    b->rad = rad;
    b->color = color;
}

// Randomly initialize a blob. Then set the radius, color, and filled status to the
// provided values.
void BLOB_init(BLOB* b, int rad, int color, bool filled)
{
    BLOB_init(b);
    b->rad = rad;
    b->color = color;
    b->filled = filled;
}

// For debug purposes, you can use this to print a blob's properties to your computer's serial monitor.
void BLOB_print(BLOB b)
{
    pc.printf("(%f, %f) <%f, %f> Color: 0x%x\n", b.posx, b.posy, b.vx, b.vy, b.color);
}

// Return the square of the distance from b1 to b2
float BLOB_dist2(BLOB b1, BLOB b2)
{
    float x1 = b1.posx;
    float y1 = b1.posy;
    float x2 = b2.posx;
    float y2 = b2.posy;
    float beforesqrt = pow(x2-x1, 2) + pow(y2-y1,2);
    return sqrt(beforesqrt);
}