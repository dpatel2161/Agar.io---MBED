// Student Side Shell Code
// For the baseline, anywhere you see ***, you have code to write.

#include "mbed.h"
#include "playSound.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "game_synchronizer.h"
#include "misc.h"
#include "blob.h"
#include "rtos.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>

#include <cmath>

#define NUM_BLOBS 22


void myMain(void);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

DigitalIn pb_u(p21);                        // Up Button
DigitalIn pb_r(p22);                        // Right Button
DigitalIn pb_d(p23);                        // Down Button
DigitalIn pb_l(p24);                        // Left Button

Serial pc(USBTX, USBRX);                    // Serial connection to PC. Useful for debugging!
MMA8452 acc(p28, p27, 100000);              // Accelerometer (SDA, SCL, Baudrate)
uLCD_4DGL uLCD(p9,p10,p11);                 // LCD (tx, rx, reset)
SDFileSystem sd(p5, p6, p7, p8, "sd");      // SD  (mosi, miso, sck, cs)
AnalogOut DACout(p18);                      // speaker
wave_player player(&DACout);                // wav player
GSYNC game_synchronizer;                    // Game_Synchronizer
GSYNC* sync = &game_synchronizer;           //
Timer frame_timer;                          // Timer

int score1 = 0;                             // Player 1's score.
int score2 = 0;                             // Player 2's score.
float ACC_THRESHOLD =  0.50;
bool hard = false;
bool run = false;
int player1 = 0;
int player2 = 0;

//thread to make sound when blob eats another
void eatSound(void const *args)
{
    char buzzer[] = "/sd/wavfiles/CARTOON.wav";
    playSound(buzzer); // buzzer is a char*
}

//power up for 5 sec when blob eats a filled blob
void speedPowerUp(void const *args)
{
    pc.printf("POWERUP\n");
    ACC_THRESHOLD = ACC_THRESHOLD * 5;
    char buzzer[] = "/sd/wavfiles/SPEED.wav";
    playSound(buzzer); // buzzer is a char*
    osDelay(5000);
    ACC_THRESHOLD = ACC_THRESHOLD / 5;
}
osThreadDef(speedPowerUp, osPriorityNormal, DEFAULT_STACK_SIZE);
osThreadDef(eatSound, osPriorityNormal, DEFAULT_STACK_SIZE);



// ***
// Display a pretty game menu on the player 1 mbed.
// Do a good job, and make it look nice. Give the player
// an option to play in single- or multi-player mode.
// Use the buttons to allow them to choose.
int game_menu(void)
{
    uLCD.background_color(BGRD_COL);
    uLCD.textbackground_color(BGRD_COL);
    uLCD.cls();
    uLCD.locate(0,0);
    uLCD.printf("AGAR.GT\n");
    uLCD.printf("\n");
    uLCD.printf("UP for 1P - EASY\n");
    uLCD.printf("LEFT for 1P - HARD\n");
    uLCD.printf("RIGHT for 1P RUN\n");
    uLCD.printf("DOWN for 2P\n");

    uLCD.circle(120, 80, 10, BLACK);
    uLCD.circle(60, 95, 30, RED);
    uLCD.circle(10, 80, 10, BLACK);
    uLCD.circle(24, 110, 6, BLACK);
    uLCD.circle(100, 100, 3, BLACK);



    // ***
    // Spin until a button is pressed. Depending which button is pressed,

    // return either SINGLE_PLAYER or MULTI_PLAYER.
    char buzzer[] = "/sd/wavfiles/START.wav";
    playSound(buzzer); // buzzer is a char*
    hard = false;
    run = false;
    while(1) {

        if(!pb_u) {
            return SINGLE_PLAYER;
        }

        if(!pb_d) {
            return MULTI_PLAYER;
        }

        if(!pb_r) {
            run = true;
            pc.printf("Run mode\n");         // Let us know you made it this far

            return SINGLE_PLAYER;
        }

        if(!pb_l) {
            hard = true;
            pc.printf("Hard mode\n");         // Let us know you made it this far

            return SINGLE_PLAYER;
        }


    }

    return SINGLE_PLAYER;

}

// Initialize the game hardware.
// Call game_menu to find out which mode to play the game in (Single- or Multi-Player)
// Initialize the game synchronizer.
void game_init(void)
{

    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 0;

    pb_u.mode(PullUp);
    pb_r.mode(PullUp);
    pb_d.mode(PullUp);
    pb_l.mode(PullUp);

    pc.printf("\033[2J\033[0;0H");              // Clear the terminal screen.
    pc.printf("I'm alive! Player 1\n");         // Let us know you made it this far

    // game_menu MUST return either SINGLE_PLAYER or MULTI_PLAYER
    int num_players = game_menu();
    pc.printf("%d players\n", num_players);         // Let us know you made it this far

    GS_init(sync, &uLCD, &acc, &pb_u, &pb_r, &pb_d, &pb_l, num_players, PLAYER1); // Connect to the other player.

    pc.printf("Initialized...\n");              // Let us know you finished initializing.
    srand(time(NULL));                          // Seed the random number generator.

    GS_cls(sync, SCREEN_BOTH);
    GS_update(sync);
}

// Display who won!
void game_over(int winner)
{
    // Pause forever.
    GS_cls(sync, SCREEN_P2);
    GS_puts(&game_synchronizer, SCREEN_P2 , "Game over", 10);

    //depending on winner display message
    if (winner == 0) {
        GS_puts(&game_synchronizer, SCREEN_P2 , "Player 1 wins", 14);
        player1++;
    } else if (winner == 2) {
        GS_puts(&game_synchronizer, SCREEN_P2 , "Game ends in tie", 17);
    } else if (winner == 1) {
        GS_puts(&game_synchronizer, SCREEN_P2 , "Player 2 wins", 14);
        player2++;
    }
    GS_update(sync);
    //make player1 screen look nice
    uLCD.background_color(BGRD_COL);
    uLCD.textbackground_color(BGRD_COL);
    uLCD.cls();
    uLCD.locate(0,0);
    uLCD.printf("GAME OVER\n");
    uLCD.printf("\n");
    uLCD.printf("WINNER IS Player %d\n", winner + 1);
    uLCD.printf("RECORD: \n");
    uLCD.printf("Player 1 %d\n", player1);
    uLCD.printf("Player 2 %d\n", player2);

    uLCD.circle(120, 100, 10, BLACK);
    uLCD.circle(60, 95, 30, RED);
    uLCD.circle(10, 90, 10, BLACK);
    uLCD.circle(24, 100, 6, BLACK);
    uLCD.circle(100, 110, 3, BLACK);

    //play game over sound
    char buzzer[] = "/sd/wavfiles/GAMEOVER.wav";
    playSound(buzzer); // buzzer is a char*



    pc.printf("Winner is Player %d\n", winner);         // Let us know you made it this far
    myMain(); //recursive call to keep history of winners
    while(1);
}

// Take in a pointer to the blobs array. Iterate over the array
// and initialize each blob with BLOB_init(). Set the first blob to (for example) blue
// and the second blob to (for example) red. Set the color(s) of the food blobs however you like.
// Make the radius of the "player blobs" equal and larger than the radius of the "food blobs".
void generate_blobs(BLOB* blobs)
{
    BLOB_init(blobs, 10, P1_COL);
    BLOB_init(blobs + 1, 10, P2_COL);


    for (int i = 2; i < NUM_BLOBS ; i++) {
        if (i%5 == 0) {
            if (i%10 == 0) {
                BLOB_init(blobs + i,  (1 +  2*static_cast <float> (rand()) / static_cast <float> (RAND_MAX)), FOOD_COL, true);
            } else {
                BLOB_init(blobs + i, (1 +  2*static_cast <float> (rand()) / static_cast <float> (RAND_MAX)), P1_COL);
            }
        } else if (i%6 == 0) {
            if (i%12 == 0) {
                BLOB_init(blobs + i, (1 +  2*static_cast <float> (rand()) / static_cast <float> (RAND_MAX)), P2_COL);
            }
        } else if (i%2 == 0) {
            BLOB_init(blobs + i, (1 +  2*static_cast <float> (rand()) / static_cast <float> (RAND_MAX)), FOOD_COL, true);
        } else {
            BLOB_init(blobs + i, 1, FOOD_COL);
        }

    }

}

int main(void)
{
    myMain();
}

void myMain(void)
{

    int* p1_buttons;
    int* p2_buttons;

    float ax1, ay1, az1;
    float ax2, ay2, az2;

    // Ask the user to choose (via pushbuttons)
    // to play in single- or multi-player mode.
    // Use their choice to correctly initialize the game synchronizer.
    game_init();

    // Keep an array of blobs. Use blob 0 for player 1 and
    // blob 1 for player 2.
    BLOB blobs[NUM_BLOBS];

    // Pass in a pointer to the blobs array. Iterate over the array
    // and initialize each blob with BLOB_init(). Set the radii and colors
    // anyway you see fit.
    generate_blobs(blobs);
    pc.printf("Done generating blobs\n");         // Let us know you made it this far

    int radius1 = 0;
    int radius2 = 0;
    double maxV = 5000000;
    float vlength1;
    float vx1;
    float vy1;
    float vx2;
    float vy2;
    while(true) {


        if (game_synchronizer.play_mode == 0) {
            for (int i = 1; i < NUM_BLOBS ; i++) {
                if (blobs[i].valid == true) {
                    break;
                }
                if (i == NUM_BLOBS - 1) {
                    game_over(0);
                }
            }
        } else {
            // multiplayer
            for (int i = 1; i < NUM_BLOBS ; i++) {
                if (blobs[i].valid == true) {
                    break;
                }
                if (i == NUM_BLOBS - 1) {
                    if (blobs[0].rad > blobs[1].rad) {
                        game_over(0);
                    } else if (blobs[0].rad < blobs[1].rad) {
                        game_over(1);
                    } else {
                        game_over(2);
                    }
                }
            }


        }
        // In single-player, check to see if the player has eaten all other blobs.

        // In multi-player mode, check to see if the players have tied by eating half the food each.
        // ***
        if (blobs[0].rad != radius1) {
            radius1 = blobs[0].rad;
            GS_cls(&game_synchronizer, SCREEN_P1);
            char printScore[10];
            sprintf (printScore, "Score: %d", (int) blobs[0].rad - 10);
            GS_puts(&game_synchronizer, SCREEN_P1 , printScore, 10);
        }

        if (blobs[1].rad != radius2) {
            radius2 = blobs[1].rad;
            GS_cls(&game_synchronizer, SCREEN_P2);
            char printScore2[10];
            sprintf (printScore2, "Score: %d", (int) blobs[1].rad - 10);
            GS_puts(&game_synchronizer, SCREEN_P2 , printScore2, 10);
        }
        // In both single- and multi-player modes, display the score(s) in an appropriate manner.
        // ***


        // Use the game synchronizer API to get the button values from both players' mbeds.
        p1_buttons = GS_get_p1_buttons(sync);
        p2_buttons = GS_get_p2_buttons(sync);

        // Use the game synchronizer API to get the accelerometer values from both players' mbeds.
        GS_get_p1_accel_data(sync, &ax1, &ay1, &az1);
        GS_get_p2_accel_data(sync, &ax2, &ay2, &az2);


//        if (maxV < game_synchronizer.p1_inputs[4]) {
//            maxV = std::abs(game_synchronizer.p1_inputs[4]);
//        }
        blobs[0].vx = 100* ACC_THRESHOLD * game_synchronizer.p1_inputs[4] *.0000002;
        blobs[0].vy = 100* ACC_THRESHOLD * game_synchronizer.p1_inputs[5] *.0000002;

        if (blobs[0].vx > ACC_THRESHOLD) {
            blobs[0].vx = ACC_THRESHOLD;
        } else if (blobs[0].vx < ACC_THRESHOLD*-1) {
            blobs[0].vx = -1* ACC_THRESHOLD;
        }

        if (blobs[0].vy > ACC_THRESHOLD) {
            blobs[0].vy = ACC_THRESHOLD;
        } else if (blobs[0].vy < ACC_THRESHOLD*-1) {
            blobs[0].vy = -1* ACC_THRESHOLD;
        }

        for (int i = 2; i < NUM_BLOBS; i++) {
            blobs[i].vx = -.5 +  static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            blobs[i].vy =  -.5 + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

            //pc.printf("Velocity x %f\n",  blobs[i].vx);         // Let us know you made it this far
            //pc.printf("Velocity y %f\n",  blobs[i].vy);         // Let us know you made it this far
        }


        if (game_synchronizer.play_mode == 1) {
            blobs[1].vx = 100* ACC_THRESHOLD * game_synchronizer.p2_inputs[4] *.0000002;
            blobs[1].vy = 100* ACC_THRESHOLD * game_synchronizer.p2_inputs[5] *.0000002;

            if (blobs[1].vx > ACC_THRESHOLD) {
                blobs[1].vx = ACC_THRESHOLD;
            } else if (blobs[1].vx < ACC_THRESHOLD*-1) {
                blobs[1].vx = -1* ACC_THRESHOLD;
            }

            if (blobs[1].vy > ACC_THRESHOLD) {
                blobs[1].vy = ACC_THRESHOLD;
            } else if (blobs[1].vy < ACC_THRESHOLD*-1) {
                blobs[1].vy = -1* ACC_THRESHOLD;
            }

            vlength1 = sqrt((blobs[1].vx * blobs[1].vx) + (blobs[1].vy * blobs[1].vy));
            vx1 = blobs[1].rad * blobs[1].vx / vlength1; //start x-point of arrow
            vy1 = blobs[1].rad * blobs[1].vy / vlength1; //start y-point of arrow
            vx2 = 1.5 * blobs[1].rad * blobs[1].vx / vlength1; //start x-point of arrow
            vy2 = 1.5 * blobs[1].rad * blobs[1].vy / vlength1; //start y-point of arrow
            GS_line(&game_synchronizer, SCREEN_P2, vx1, vy1, vx2, vy2, ARROW_COL);
        }


        // If the magnitude of the p1 x and/or y accelerometer values exceed ACC_THRESHOLD,
        // set the blob 0 velocities to be proportional to the accelerometer values.
        // ***

        // If in multi-player mode and the magnitude of the p2 x and/or y accelerometer values exceed ACC_THRESHOLD,
        // set the blob 0 velocities to be proportional to the accelerometer values.
        // ***

        float time_step = .01;

        // Undraw the world bounding rectangle (use BGRD_COL).
        // ***
        GS_rectangle(&game_synchronizer, SCREEN_P1, (WORLD_WIDTH/-2) - blobs[0].posx , (WORLD_HEIGHT/2) - blobs[0].posy , (WORLD_WIDTH/2) - blobs[0].posx , (WORLD_HEIGHT/-2) - blobs[0].posy , BGRD_COL);

        if (game_synchronizer.play_mode == 1) {
            GS_rectangle(&game_synchronizer, SCREEN_P2, (WORLD_WIDTH/-2) - blobs[1].posx , (WORLD_HEIGHT/2) - blobs[1].posy , (WORLD_WIDTH/2) - blobs[1].posx , (WORLD_HEIGHT/-2) - blobs[1].posy , BLACK_COL);
        }

        //constrain blobs to world
        for (int i = 0; i < NUM_BLOBS; i++) {
            BLOB_constrain2world(&blobs[i]);
        }
        vlength1 = sqrt((blobs[0].vx * blobs[0].vx) + (blobs[0].vy * blobs[0].vy));
        vx1 = blobs[0].rad * blobs[0].vx / vlength1; //start x-point of arrow
        vy1 = blobs[0].rad * blobs[0].vy / vlength1; //start y-point of arrow
        vx2 = 1.5 * blobs[0].rad * blobs[0].vx / vlength1; //start x-point of arrow
        vy2 = 1.5 * blobs[0].rad * blobs[0].vy / vlength1; //start y-point of arrow
        GS_line(&game_synchronizer, SCREEN_P1, vx1, vy1, vx2, vy2, ARROW_COL);
        if (blobs[0].valid || blobs[0].delete_now) {

            GS_circle(&game_synchronizer, SCREEN_P1, 0 , 0 , blobs[0].rad, BGRD_COL);
            blobs[0].delete_now = false;

        }
        if (blobs[1].valid || blobs[1].delete_now) {
            GS_circle(&game_synchronizer, SCREEN_P1, blobs[1].posx - blobs[0].posx , blobs[1].posy - blobs[0].posy, blobs[1].rad, BGRD_COL);
            blobs[1].delete_now = false;

        }
        for (int i = 2; i < NUM_BLOBS; i++) {

            if (blobs[i].valid || blobs[i].delete_now) {
                if (blobs[i].filled) {
                    GS_filled_circle(&game_synchronizer, SCREEN_P1, blobs[i].posx - blobs[0].posx , blobs[i].posy - blobs[0].posy , 20*blobs[i].rad / blobs[0].rad , BGRD_COL);
                } else {
                    GS_circle(&game_synchronizer, SCREEN_P1, blobs[i].posx - blobs[0].posx , blobs[i].posy - blobs[0].posy , 20*blobs[i].rad / radius1 , BGRD_COL);
                }
                blobs[i].delete_now = false;
            }
            // If the current blob is valid, or it was deleted in the last frame, (delete_now is true), then draw a background colored circle over
            // the old position of the blob. (If delete_now is true, reset delete_now to false.)
            // ***
        }
        // Use the blob positions and velocities, as well as the time_step to compute the new position of the blob.
        // ***
        if (game_synchronizer.play_mode == 1) {
            if (blobs[0].valid || blobs[0].delete_now) {
                GS_circle(&game_synchronizer, SCREEN_P2, blobs[0].posx - blobs[1].posx , blobs[0].posy - blobs[1].posy , blobs[0].rad, BLACK_COL);
                blobs[0].delete_now = false;
            }
        }



        if (game_synchronizer.play_mode == 1) {
            if (blobs[0].valid || blobs[0].delete_now) {
                GS_circle(&game_synchronizer, SCREEN_P2, blobs[0].posx - blobs[1].posx , blobs[0].posy - blobs[1].posy , blobs[0].rad, BLACK_COL);
                blobs[0].delete_now = false;
            }
            if (blobs[1].valid || blobs[1].delete_now) {
                GS_circle(&game_synchronizer, SCREEN_P2, 0 , 0, blobs[1].rad, BLACK_COL);
                blobs[1].delete_now = false;
            }
            for (int i = 2; i < NUM_BLOBS; i++) {
                if (blobs[i].valid || blobs[i].delete_now) {
                    GS_circle(&game_synchronizer, SCREEN_P2, blobs[i].posx - blobs[1].posx , blobs[i].posy - blobs[1].posy , 20*blobs[i].rad / blobs[1].rad , BLACK_COL);
                    blobs[i].delete_now = false;
                }
            }

        }
        //AI if hard mode
        if (hard) {
            blobs[1].rad = blobs[0].rad * 2;
            if (BLOB_dist2(blobs[0],blobs[1]) > 10) {

                blobs[1].vx = 10*(blobs[0].posx - blobs[1].posx)/1000;
                blobs[1].vy = 10*(blobs[0].posy - blobs[1].posy)/1000;
            } else {

                blobs[1].vx = (blobs[0].posx - blobs[1].posx)/1000;
                blobs[1].vy = (blobs[0].posy - blobs[1].posy)/1000;
            }
            pc.printf("blob 0 x: %f\n", blobs[0].posx);
            pc.printf("blob 1 x: %f\n", blobs[1].posx);
            pc.printf("blob 1 vx: %f\n", blobs[1].vx);


        }
        for (int i = 0; i < NUM_BLOBS; i++) {

            blobs[i].old_x = blobs[i].posx;
            blobs[i].old_y = blobs[i].posy;
            blobs[i].posx = blobs[i].old_x + 1000*blobs[i].vx * time_step;
            blobs[i].posy = blobs[i].old_y + 1000*blobs[i].vy * time_step;
        }

        // If the current blob is blob 0, iterate over all blobs and check if they are close enough to eat or be eaten.
        // In multi-player mode, if the player 0 blob is eaten, player 1 wins and vise versa.
        // ***

        // If the current blob is blob 1 and we are playing in multi-player mode, iterate over all blobs and check
        // if they are close enough to eat or be eaten. In multi-player mode, if the player 1 blob is eaten, player 0 wins and vise versa.
        // ***
        for (int i = 1; i < NUM_BLOBS; i++) {
            float distance = BLOB_dist2(blobs[0], blobs[i]);

            if (distance > (blobs[0].rad + blobs[i].rad)) {
                distance = distance;


            } else if (distance <= (blobs[0].rad - blobs[i].rad) && blobs[i].valid && (blobs[i].color != blobs[0].color)) {
                // Inside
                if (run) {
                    game_over(1);
                }
                if (i == 1) {
                    game_over(0);
                }
                blobs[0].rad = blobs[0].rad + blobs[i].rad;
                blobs[i].delete_now = true;
                blobs[i].valid = false;
                if (blobs[i].filled) {
                    osThreadCreate(osThread(speedPowerUp), NULL);
                } else {
                    osThreadCreate(osThread(eatSound), NULL);

                }
            } else {
                if (run && (blobs[i].color != blobs[0].color)) {
                    game_over(1);
                }
            }
        }
        for (int i = 0; i < NUM_BLOBS; i++) {
            float distance = BLOB_dist2(blobs[1], blobs[i]);

            if (distance > (blobs[1].rad + blobs[i].rad)) {
                distance = distance; //do nothing

            } else if (distance <= (blobs[1].rad - blobs[i].rad) && blobs[i].valid && (blobs[i].color != blobs[1].color)) {
                // Inside
                if (i == 0) {
                    game_over(1);
                }
                blobs[1].rad = blobs[1].rad + blobs[i].rad;
                blobs[i].delete_now = true;
                blobs[i].valid = false;
                pc.printf("EATEN %f\n",  i);         // Let us know you made it this far
                osThreadCreate(osThread(eatSound), NULL);

            }
            if (i==0) {
                if (isnan(distance)) {
                    game_over(1); //in hard mode game over if 0/0 distance
                }

                i++;
            }
        }

        //draw blobs on P1 screen
        if (blobs[0].valid) {
            GS_circle(&game_synchronizer, SCREEN_P1, 0 , 0 , blobs[0].rad, P1_COL);
        }
        if (blobs[1].valid) {
            GS_circle(&game_synchronizer, SCREEN_P1, blobs[1].posx - blobs[0].posx , blobs[1].posy - blobs[0].posy, blobs[1].rad, P2_COL);
        }

        for (int i = 2; i < NUM_BLOBS; i++) {

            if (blobs[i].valid) {
                if (blobs[i].filled) {
                    GS_filled_circle(&game_synchronizer, SCREEN_P1, blobs[i].posx - blobs[0].posx , blobs[i].posy - blobs[0].posy , 20*blobs[i].rad / blobs[0].rad , blobs[i].color);
                } else {
                    GS_circle(&game_synchronizer, SCREEN_P1, blobs[i].posx - blobs[0].posx , blobs[i].posy - blobs[0].posy , 20*blobs[i].rad / blobs[0].rad , blobs[i].color);
                }
            }
            // Iterate over all blobs and draw them at their newly computed positions. Reference their positions to the player blobs.
            // That is, on screen 1, center the world on blob 0 and reference all blobs' position to that of blob 0.
            // On screen 2, center the world on blob 1 and reference all blobs' position tho that of blob 1.
            // ***
        }
        //draw directional arrow
        vlength1 = sqrt((blobs[0].vx * blobs[0].vx) + (blobs[0].vy * blobs[0].vy));
        vx1 = blobs[0].rad * blobs[0].vx / vlength1; //start x-point of arrow
        vy1 = blobs[0].rad * blobs[0].vy / vlength1; //start y-point of arrow
        vx2 = 1.5 * blobs[0].rad * blobs[0].vx / vlength1; //start x-point of arrow
        vy2 = 1.5 * blobs[0].rad * blobs[0].vy / vlength1; //start y-point of arrow
        GS_line(&game_synchronizer, SCREEN_P1, vx1, vy1, vx2, vy2, BGRD_COL);

        // Redraw the world boundary rectangle.
        // ***
        GS_rectangle(&game_synchronizer, SCREEN_P1, (WORLD_WIDTH/-2) - blobs[0].posx , (WORLD_HEIGHT/2) - blobs[0].posy , (WORLD_WIDTH/2) - blobs[0].posx , (WORLD_HEIGHT/-2) - blobs[0].posy , BORDER_COL);

        if (game_synchronizer.play_mode == 1) {
            //draw arrow
            vlength1 = sqrt((blobs[1].vx * blobs[1].vx) + (blobs[1].vy * blobs[1].vy));
            vx1 = blobs[1].rad * blobs[1].vx / vlength1; //start x-point of arrow
            vy1 = blobs[1].rad * blobs[1].vy / vlength1; //start y-point of arrow
            vx2 = 1.5 * blobs[1].rad * blobs[1].vx / vlength1; //start x-point of arrow
            vy2 = 1.5 * blobs[1].rad * blobs[1].vy / vlength1; //start y-point of arrow
            GS_line(&game_synchronizer, SCREEN_P2, vx1, vy1, vx2, vy2, BLACK_COL);

            //draw blobs
            if (blobs[0].valid) {
                GS_circle(&game_synchronizer, SCREEN_P2, blobs[0].posx - blobs[1].posx , blobs[0].posy - blobs[1].posy , blobs[0].rad, P1_COL);
            }
            if (blobs[1].valid) {
                GS_circle(&game_synchronizer, SCREEN_P2, 0 , 0, blobs[1].rad, P2_COL);
            }
            for (int i = 2; i < NUM_BLOBS; i++) {
                if (blobs[i].valid) {
                    GS_circle(&game_synchronizer, SCREEN_P2, blobs[i].posx - blobs[1].posx , blobs[i].posy - blobs[1].posy , 20*blobs[i].rad / blobs[1].rad , blobs[i].color);
                }
            }
            GS_rectangle(&game_synchronizer, SCREEN_P2, (WORLD_WIDTH/-2) - blobs[1].posx , (WORLD_HEIGHT/2) - blobs[1].posy , (WORLD_WIDTH/2) - blobs[1].posx , (WORLD_HEIGHT/-2) - blobs[1].posy , BORDER_COL);
        }
        // Update the screens by calling GS_update.
        GS_update(sync);

        // If a button on either side is pressed, the corresponding LED on the player 1 mbed will toggle.
        // This is just for debug purposes, and to know that your button pushes are being registered.
        led1 = p1_buttons[0] ^ p2_buttons[0];
        led2 = p1_buttons[1] ^ p2_buttons[1];
        led3 = p1_buttons[2] ^ p2_buttons[2];
        led4 = p1_buttons[3] ^ p2_buttons[3];
    }

}