#ifndef GAME_SYNC_H__
#define GAME_SYNC_H__

#include <cmath>
#include "uLCD_4DGL.h"
#include "EthernetInterface.h"
#include "MMA8452.h"

#define PLAYER1 0
#define PLAYER2 1

#define SCREEN_P1 0
#define SCREEN_P2 1
#define SCREEN_BOTH 2

#define SINGLE_PLAYER 0
#define MULTI_PLAYER  1

#define PLAYER1_IP      "192.168.2.1"
#define PLAYER2_IP      "192.168.2.2"
#define SERVER_PORT                7
#define ETH_PACKET_SIZE         1024

#define U_BUTTON 0
#define R_BUTTON 1
#define D_BUTTON 2
#define L_BUTTON 3

enum Command {
    NOP_CMD,
    CLS_CMD,
    BG_COLOR_CMD,
    LINE_CMD,
    CIRCLE_CMD,
    FILLED_CIRCLE_CMD,
    TRI_CMD,
    RECT_CMD,
    FILLED_RECT_CMD,
    PIX_CMD,
    LOCATE_CMD,
    PUTC_CMD,
    PUTS_CMD,
    TEXT_BACKGROUND_COLOR_CMD
};


typedef struct Game_Synchronizer {
    
    int p1_p2;
    
    int play_mode;
    int buffer1[ETH_PACKET_SIZE];
    int buffer_idx1;
    
    int buffer2[ETH_PACKET_SIZE];
    int buffer_idx2;
    
    TCPSocketServer* server;
    TCPSocketConnection*  sock;
    EthernetInterface* eth;
    
    uLCD_4DGL* LCD;
    MMA8452* acc; 
    
    int p2_inputs[7];
    int p1_inputs[7];
    
    DigitalIn* pb_u;  // up button
    DigitalIn* pb_r;  // right button
    DigitalIn* pb_d;  // down button
    DigitalIn* pb_l;  // left button
} GSYNC;


void GS_init(GSYNC* gs, uLCD_4DGL*, MMA8452*, DigitalIn*, DigitalIn*, DigitalIn*, DigitalIn*, int, bool player);

// Yes, this sucks. If you're smart, find a good way to do variable args and show me!
// Look into template metaprogramming!

void  GS_draw(GSYNC* gs, char screen, int CMD);
void  GS_draw(GSYNC* gs, char screen, int CMD, int a);
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b);
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c);
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d);
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d, int e);
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d, int e, int f);
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d, int e, int f, int g); 
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d, int e, int f, int g, char nArgs);

void GS_nop(GSYNC* gs, char screen);
void GS_background_color(GSYNC* gs, char screen, int color);
void GS_line(GSYNC* gs, char screen, int sx, int sy, int ex, int ey, int color);
void GS_circle(GSYNC* gs, char screen, int x , int y , int radius, int color);
void GS_filled_circle(GSYNC* gs, char screen, int x , int y , int radius, int color);
void GS_triangle(GSYNC* gs, char screen, int a, int b, int c, int d , int e, int f, int col);
void GS_rectangle(GSYNC* gs, char screen, int a, int b, int c, int d, int col);
void GS_filled_rectangle(GSYNC* gs, char screen, int a, int b, int c, int d, int col);
void GS_pixel(GSYNC* gs, char screen, int a, int b, int col);
void GS_cls(GSYNC* gs, char screen);
void GS_locate(GSYNC* gs, char screen, int x, int y);
void GS_puts(GSYNC* gs, char screen, char*, int);
void GS_putc(GSYNC* gs, char screen, char);
void GS_textbackground_color(GSYNC* gs, char screen, int col);

// Reads don't need to be done on the slave side. Hopefully both sides match!
int  read_pixel(int x, int y);
int  CONVERT_24_TO_16_BPP(int col_24);
bool pixel_eq(int color1, int color2);

void GS_set_p1_inputs(GSYNC*);
void GS_set_p2_inputs(GSYNC*);

int* GS_get_p1_buttons(GSYNC*);
int* GS_get_p2_buttons(GSYNC*);

void GS_get_p1_accel_data(GSYNC*, float*, float*, float*);
void GS_get_p2_accel_data(GSYNC*, float*, float*, float*);

void GS_update(GSYNC* gs);

void GS_close(GSYNC* gs);


#endif 