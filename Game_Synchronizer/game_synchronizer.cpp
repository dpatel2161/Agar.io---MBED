#include "game_synchronizer.h"

extern Serial pc;

void GS_init(GSYNC* gs, uLCD_4DGL* lcd, MMA8452* accel, DigitalIn* up, DigitalIn* right, DigitalIn* down, DigitalIn* left, int mode, bool player) { 
 
    gs->p1_p2 = player; 
 
    // Save a pointer to the local lcd.
    gs->LCD = lcd;
    gs->LCD->baudrate(BAUD_1000000);
    
    // Save a pointer to the local accelerometer.
    gs->acc = accel;
    
    // Set up the local buttons.
    gs->pb_u = up;
    gs->pb_r = right;
    gs->pb_d = down;
    gs->pb_l = left;
    
    // Save the play mode. (Multi-player or Single-player)
    gs->play_mode = mode;
    
    // Initialize the idx into the send_buffer to zero.
    gs->buffer_idx1 = 0;
    gs->buffer_idx2 = 0;
    
    // p2_inputs is the array containing player two's button (and accelerometer) values.
    // Initialize it to zero.
    memset(gs->p1_inputs, 0, sizeof(gs->p1_inputs));
    memset(gs->p2_inputs, 0, sizeof(gs->p2_inputs));
    
    switch (gs->p1_p2) {
        case PLAYER1:                               // If I am p1...
        
            // If play_mode is set to multi-player, establish the ethernet connection.
            if(MULTI_PLAYER == gs->play_mode) {
                gs->eth = new EthernetInterface();
                gs->eth->init(PLAYER1_IP, "255.255.255.0", "0.0.0.0"); 
                gs->eth->connect();
                
                gs->sock = new TCPSocketConnection();
                while(gs->sock->connect(PLAYER2_IP, SERVER_PORT) < 0) {
                    //pc.printf("Trying to connect.\n");
                }
            }
            break;
        case PLAYER2:                               // If I am p2...
            
            // If I am player 2, play_mode doesn't matter. I have to assume it's 
            // set to multi-player and try to connect to p1.
            gs->eth = new EthernetInterface();
            gs->eth->init(PLAYER2_IP, "255.255.255.0", "0.0.0.0"); 
            gs->eth->connect();

            gs->server = new TCPSocketServer();
            gs->server->bind(SERVER_PORT);
            gs->server->listen();
            gs->sock = new TCPSocketConnection();
            gs->server->accept(*(gs->sock));
            gs->sock->set_blocking(false, 1500);
            break;   
    }
}

// Yes, this sucks. If you're smart, find a good way to do variable args and show me!
// Look into templates!

void  GS_draw(GSYNC* gs, char screen, int CMD) {GS_draw(gs, screen, CMD, 0,0,0,0,0,0,0, 0); }
void  GS_draw(GSYNC* gs, char screen, int CMD, int a) { GS_draw(gs, screen, CMD, a, 0,0,0,0,0,0, 1); }
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b) { GS_draw(gs, screen, CMD, a, b, 0,0,0,0,0, 2); }
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c) { GS_draw(gs, screen, CMD, a, b, c, 0,0,0,0, 3); }
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d) { GS_draw(gs, screen, CMD, a, b, c, d, 0,0,0, 4); }
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d, int e) { GS_draw(gs, screen, CMD, a, b, c, d, e, 0,0, 5); }
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d, int e, int f) { GS_draw(gs, screen, CMD, a, b, c, d, e, f, 0, 6); }
void  GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d, int e, int f, int g) { GS_draw(gs, screen, CMD, a, b, c, d, e, f, g, 7); }

void GS_draw(GSYNC* gs, char screen, int CMD, int a, int b, int c, int d, int e, int f, int g, char nArgs){
    
    // I haven't had time to deal with overflows of the buffer. If you are pushing tons of draw calls into the buffer,
    // you could overrun it. This will cause bad things. (At a minimum, your stuff won't be drawn.)
    // If you have this problem, try calling update in the middle of your draw calls to flush the buffer.
    // Alternatively, you can increase ETH_PACKET_SIZE.
    
    if(nArgs > 7) { 
        //pc.printf("Error in call to _draw(): nArgs > 7 not allowed!\n");
        return;
    }
    
    if(screen == SCREEN_P1 || screen == SCREEN_BOTH) {
        gs->buffer1[gs->buffer_idx1] = CMD;
        if(nArgs >= 1) gs->buffer1[gs->buffer_idx1+1] = a;
        if(nArgs >= 2) gs->buffer1[gs->buffer_idx1+2] = b;
        if(nArgs >= 3) gs->buffer1[gs->buffer_idx1+3] = c;
        if(nArgs >= 4) gs->buffer1[gs->buffer_idx1+4] = d;
        if(nArgs >= 5) gs->buffer1[gs->buffer_idx1+5] = e;
        if(nArgs >= 6) gs->buffer1[gs->buffer_idx1+6] = f;
        if(nArgs >= 7) gs->buffer1[gs->buffer_idx1+7] = g;
        
        gs->buffer_idx1 += nArgs+1;
    }
    if(screen == SCREEN_P2 || screen == SCREEN_BOTH) {
        gs->buffer2[gs->buffer_idx2] = CMD;
        if(nArgs >= 1) gs->buffer2[gs->buffer_idx2+1] = a;
        if(nArgs >= 2) gs->buffer2[gs->buffer_idx2+2] = b;
        if(nArgs >= 3) gs->buffer2[gs->buffer_idx2+3] = c;
        if(nArgs >= 4) gs->buffer2[gs->buffer_idx2+4] = d;
        if(nArgs >= 5) gs->buffer2[gs->buffer_idx2+5] = e;
        if(nArgs >= 6) gs->buffer2[gs->buffer_idx2+6] = f;
        if(nArgs >= 7) gs->buffer2[gs->buffer_idx2+7] = g;
        
        gs->buffer_idx2 += nArgs+1;
    }
}

void GS_puts(GSYNC* gs, char screen, char* str, int strlen) {
    
    if(screen == SCREEN_P1 || screen == SCREEN_BOTH) {
        gs->buffer1[gs->buffer_idx1] = PUTS_CMD;
        gs->buffer1[gs->buffer_idx1+1] = strlen;
        for(int i = 0; i < strlen; i++) {
            gs->buffer1[i+gs->buffer_idx1+2] = str[i];
            
        }
        gs->buffer_idx1 += strlen + 2;
    }
    if(screen == SCREEN_P2 || screen == SCREEN_BOTH) {
        gs->buffer2[gs->buffer_idx2] = PUTS_CMD;
        gs->buffer2[gs->buffer_idx2+1] = strlen;
        for(int i = 0; i < strlen; i++) {
            gs->buffer2[i+gs->buffer_idx2+2] = str[i];
            
        }
        gs->buffer_idx2 += strlen + 2;
    }
}

void GS_nop(GSYNC* gs, char screen)                                                          { GS_draw(gs, screen, NOP_CMD); }
void GS_background_color(GSYNC* gs, char screen, int color)                                  { GS_draw(gs, screen, BG_COLOR_CMD, color); }
void GS_filled_circle(GSYNC* gs, char screen, int x , int y , int radius, int color)         { GS_draw(gs, screen, FILLED_CIRCLE_CMD, x+64, y+64, radius, color); }
void GS_triangle(GSYNC* gs, char screen, int a, int b, int c, int d , int e, int f, int col) { GS_draw(gs, screen, TRI_CMD, a+64, b+64, c+64, d+64, e+64, f+64, col); }
void GS_rectangle(GSYNC* gs, char screen, int a, int b, int c, int d, int col)               { GS_draw(gs, screen, RECT_CMD, a+64, b+64, c+64, d+64, col); }
void GS_filled_rectangle(GSYNC* gs, char screen, int a, int b, int c, int d, int col)        { GS_draw(gs, screen, FILLED_RECT_CMD, a+64, b+64, c+64, d+64, col); }
void GS_pixel(GSYNC* gs, char screen, int a, int b, int col)                                 { GS_draw(gs, screen, PIX_CMD, a+64, b+64, col); }
void GS_cls(GSYNC* gs, char screen)                                                          { GS_draw(gs, screen, CLS_CMD); }
void GS_locate(GSYNC* gs, char screen, int x, int y)                                         { GS_draw(gs, screen, LOCATE_CMD, x, y); }
void GS_putc(GSYNC* gs, char screen, char a)                                                 { GS_draw(gs, screen, PUTC_CMD, (int)a); }
void GS_textbackground_color(GSYNC* gs, char screen, int col)                                { GS_draw(gs, screen, TEXT_BACKGROUND_COLOR_CMD, col); }

void GS_line(GSYNC* gs, char screen, int sx, int sy, int ex, int ey, int color) { 
    //if(sx > 0 && sx < 128 && sy > 0 && sy < 128 && ex > 0 && ex < 128 && ey > 0 && ey < 128)    // line lies entirely within the rect.
    // Could add some culling if you have too...
    GS_draw(gs, screen, LINE_CMD, sx+64, sy+64, ex+64, ey+64, color); 
}
void GS_circle(GSYNC* gs, char screen, int x , int y , int radius, int color) {
    if(x + 64 - radius > 127 || x + 64 + radius < 0 ||  y + 64 - radius > 127 || y + 64 + radius < 0) { return; }           // Don't buffer circles that don't intersect the screen.
    GS_draw(gs, screen, CIRCLE_CMD, x+64, y+64, radius, color);
}

// Returns the 16bpp color of the screen at location (x, y)
int  GS_read_pixel(GSYNC* gs, int x, int y) { 
    return gs->LCD->read_pixel(x, y);
}

// Convert 24bpp colors to 16bpp colors
int CONVERT_24_TO_16_BPP(int col_24) {
    int b = col_24 & 0xFF;
    int g = (col_24 >> 8) & 0xFF;
    int r = (col_24 >> 16)& 0xFF;
    
    r >>= 3;
    g >>= 2;
    b >>= 3;
    
    return r<<11 | g<<5 | b;
}

// Compare two colors (16bpp or 24bpp)
bool pixel_eq(int color1, int color2) {
    return (CONVERT_24_TO_16_BPP(color1) == color2 || 
            CONVERT_24_TO_16_BPP(color2) == color1 ||
            color1 == color2);
}

// This allows the Player 2 code to set its inputs.
// Accelerometer values are stored in 16:16 fixed point representation.
void GS_set_p2_inputs(GSYNC* gs) { 
    if(gs->p1_p2 == PLAYER1) { return; }
    
    gs->p2_inputs[0] = !(*(gs->pb_u));
    gs->p2_inputs[1] = !(*(gs->pb_r));
    gs->p2_inputs[2] = !(*(gs->pb_d));
    gs->p2_inputs[3] = !(*(gs->pb_l));

    double acc_x, acc_y, acc_z;
    gs->acc->readXYZGravity(&acc_x,&acc_y,&acc_z);
    
    gs->p2_inputs[4] = static_cast<int>(acc_x * 65536);
    gs->p2_inputs[5] = static_cast<int>(acc_y * 65536);
    gs->p2_inputs[6] = static_cast<int>(acc_z * 65536);
}

// Use this to update Player 1's inputs.
// Accelerometer values are stored in 16:16 fixed point representation.
void GS_set_p1_inputs(GSYNC* gs) {
    if(gs->p1_p2 == PLAYER2) { return; }
    
    gs->p1_inputs[0] = !(*(gs->pb_u));
    gs->p1_inputs[1] = !(*(gs->pb_r));
    gs->p1_inputs[2] = !(*(gs->pb_d));
    gs->p1_inputs[3] = !(*(gs->pb_l));

    double acc_x, acc_y, acc_z;
    gs->acc->readXYZGravity(&acc_x,&acc_y,&acc_z);
    
    gs->p1_inputs[4] = (int)(acc_x * 65536);
    gs->p1_inputs[5] = (int)(acc_y * 65536);
    gs->p1_inputs[6] = (int)(acc_z * 65536);
}


void GS_get_p1_accel_data(GSYNC* gs, float* ax, float* ay, float* az) {
    *ax = (float) gs->p1_inputs[4] / 65536.0;
    *ay = (float) gs->p1_inputs[5] / 65536.0;
    *az = (float) gs->p1_inputs[6] / 65536.0;
}

void GS_get_p2_accel_data(GSYNC* gs, float* ax, float* ay, float* az) {
    *ax = (float) gs->p2_inputs[4] / 65536.0;
    *ay = (float) gs->p2_inputs[5] / 65536.0;
    *az = (float) gs->p2_inputs[6] / 65536.0;
}

int* GS_get_p1_buttons(GSYNC* gs) {
    return gs->p1_inputs;
}

int* GS_get_p2_buttons(GSYNC* gs) {
    return gs->p2_inputs;
}

void GS_update(GSYNC* gs) {
    
    GS_nop(gs, SCREEN_BOTH);
    GS_set_p1_inputs(gs);
    
    int buffer_size1 = gs->buffer_idx1-1;
    gs->buffer_idx1 = 0;
    int buffer_size2 = gs->buffer_idx2-1;
    gs->buffer_idx2 = 0;  
    
      
    if(gs->p1_p2 == PLAYER1 && MULTI_PLAYER == gs->play_mode) {
        gs->sock->set_blocking(true, 100);
        gs->sock->send_all((char*)gs->buffer2, (buffer_size2+1)*sizeof(gs->buffer2[0]));
        
        int n = gs->sock->receive((char*)gs->p2_inputs, sizeof(gs->p2_inputs));
        //if(n < 0) {pc.printf("RECEIVE ERROR.\n");}          
    
    }else if(gs->p1_p2 == PLAYER2) {    
        gs->sock->set_blocking(true, 100);
        int n = gs->sock->receive((char*)gs->buffer1, sizeof(gs->buffer1));  
        //if(n < 0) {pc.printf("RECEIVE ERROR.\n");}
        gs->buffer1[n] = '\0';   
        buffer_size1 = n/sizeof(gs->buffer1[0]) - 1;
        
        gs->sock->send_all((char*)gs->p2_inputs, sizeof(gs->p2_inputs));          
    }
    
        
    int idx = 0;
    while(idx < buffer_size1) {
        char cmd = gs->buffer1[idx];
        idx++;
        
        int str_length;
        char* str;
        //pc.printf("CMD: %d\n", cmd);
        switch(cmd) {
            case NOP_CMD:
                break;
            case CLS_CMD:
                gs->LCD->cls();
                //pc.printf("%d Clear the screen!\n",idx);
                break;
            case BG_COLOR_CMD:
                gs->LCD->background_color(gs->buffer1[idx]);
                //pc.printf("Change the background to 0x%X\n", buffer[idx]);
                idx += 1;
                break;
            case LINE_CMD:
                //pc.printf("LINE: (%d, %d) - (%d, %d) COLOR: 0x%X\n", buffer[idx], buffer[idx+1], buffer[idx+2], buffer[idx+3], buffer[idx+4]);
                gs->LCD->line(gs->buffer1[idx], gs->buffer1[idx+1], gs->buffer1[idx+2], gs->buffer1[idx+3], gs->buffer1[idx+4]);
                idx += 5;
                break;
            case CIRCLE_CMD:
                //pc.printf("CIRCLE: (%d, %d), r=%d\n", buffer[idx], buffer[idx+1], buffer[idx+2]);
                gs->LCD->circle(gs->buffer1[idx], gs->buffer1[idx+1], gs->buffer1[idx+2], gs->buffer1[idx+3]);
                idx += 4;
                break;
            case FILLED_CIRCLE_CMD:
                //pc.printf("CIRCLE: (%d, %d), r=%d\n", buffer[idx], buffer[idx+1], buffer[idx+2]);
                gs->LCD->filled_circle(gs->buffer1[idx], gs->buffer1[idx+1], gs->buffer1[idx+2], gs->buffer1[idx+3]);
                idx += 4;
                break;
            case TRI_CMD:
                //pc.printf("CIRCLE: (%d, %d), r=%d\n", buffer[idx], buffer[idx+1], buffer[idx+2]);
                gs->LCD->triangle(gs->buffer1[idx], gs->buffer1[idx+1], gs->buffer1[idx+2], gs->buffer1[idx+3], gs->buffer1[idx+4], gs->buffer1[idx+5], gs->buffer1[idx+6]);
                idx += 7;
                break;
            case RECT_CMD:
                gs->LCD->rectangle(gs->buffer1[idx], gs->buffer1[idx+1], gs->buffer1[idx+2], gs->buffer1[idx+3], gs->buffer1[idx+4]);
                idx += 5;
                break;
            case FILLED_RECT_CMD:
                gs->LCD->filled_rectangle(gs->buffer1[idx], gs->buffer1[idx+1], gs->buffer1[idx+2], gs->buffer1[idx+3], gs->buffer1[idx+4]);
                idx += 5;
                break;
            case PIX_CMD:
                gs->LCD->pixel(gs->buffer1[idx], gs->buffer1[idx+1], gs->buffer1[idx+2]);
                idx += 3;
                break;
            case PUTS_CMD:
                str_length = gs->buffer1[idx];
                str = new char[str_length];
                for(int i=0; i<str_length; i++) {
                    str[i] = (char)gs->buffer1[idx+1+i];
                }
                gs->LCD->puts(str);
                idx += str_length+1;
                break;
            case LOCATE_CMD:
                gs->LCD->locate(gs->buffer1[idx], gs->buffer1[idx+1]);
                idx += 2;
                break;
            case PUTC_CMD:
                gs->LCD->putc((char)gs->buffer1[idx]);
                idx += 1;
                break;
            case TEXT_BACKGROUND_COLOR_CMD:
                gs->LCD->textbackground_color(gs->buffer1[idx]);
                idx += 1;
                break;
            default:
                //pc.printf("UNKNOWN CMD %d: This could get ugly!\n", cmd);
                idx += 0;
                break;
        }
        wait_us(500);       // Chill. Don't want to overrun the screen!
    }
    
}

void GS_close(GSYNC* gs) {            
    gs->sock->close();
    gs->eth->disconnect();  
    delete gs->sock;
    delete gs->server;
    delete gs->eth; 
    delete gs->pb_u;
    delete gs->pb_r;
    delete gs->pb_d;
    delete gs->pb_l;
}
