/*
 * File: Marquee.c
 * Author: Dianyao Su
 * Date: 2026/02/27
 * Description: KEYBOARD LED MARQUEE EFFECT USING WIRINGPI LIBRARY.
 *              PRESS R TO MOVE LEDS RIGHT TO LEFT,
 *              PRESS L TO MOVE LEDS LEFT TO RIGHT,
 *              PRESS Q TO QUIT THE PROGRAM.
 */

// 4 LED Marquee with 3 button control - WiringPi Version
// B0 (GPIO5) : move LEDs right to left
// B1 (GPIO6) : move LEDs left to right
// B2 (GPIO27): stop the program
// LEDs: GPIO22, GPIO23, GPIO24, GPIO25

#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>

int main(void) {
    // Initialize WiringPi with GPIO numbering
    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi setup failed\n");
        return 1;
    }
    
    // Define pins (using BCM GPIO numbers)
    int LED1 = 22;   // GPIO22
    int LED2 = 23;   // GPIO23
    int LED3 = 24;   // GPIO24
    int LED4 = 25;   // GPIO25
    
    int B0 = 5;      // GPIO5  (right to left)
    int B1 = 6;      // GPIO6  (left to right)
    int B2 = 27;     // GPIO27 (exit)
    
    int leds[4] = {LED1, LED2, LED3, LED4};
    int pos = 0;
    int dir = 0;     // STOP  = 0 LEFT TO RIGHT = 1 RIGHT TO LEFT = 2
    
    // Set LED pins as output and turn them off
    for (int i = 0; i < 4; i++) {
        pinMode(leds[i], OUTPUT);
        digitalWrite(leds[i], LOW);
    }
    
    // Set button pins as input with pull-up resistors
    pinMode(B0, INPUT);
    pinMode(B1, INPUT);
    pinMode(B2, INPUT);
    
    pullUpDnControl(B0, PUD_UP);
    pullUpDnControl(B1, PUD_UP);
    pullUpDnControl(B2, PUD_UP);
    
    printf("Program started...\n");
    printf("Switch left (GPIO5): right to left\n");
    printf("Switch right (GPIO6): left to right\n");
    printf("Press B2 (GPIO27): exit program\n");
    
    do {
        // Read button states (0=pressed, 1=released with pull-up)
        int b0_state = digitalRead(B0);
        int b1_state = digitalRead(B1);
        int b2_state = digitalRead(B2);
        
        // Check direction switches (active low)
        if (b0_state == 0) {      // Switch left (GPIO5 low)
            dir = 2;               // Right to left
        } else if (b1_state == 0) { // Switch right (GPIO6 low)
            dir = 1;               // Left to right
        }
        
        // Check exit button
        if (b2_state == 0) {
            printf("Exit button pressed\n");
            break;
        }
        
        // Turn off all LEDs
        for (int i = 0; i < 4; i++) {
            digitalWrite(leds[i], LOW);
        }
        
        // Light current LED
        digitalWrite(leds[pos], HIGH);
        usleep(200000);  // 200ms delay
        
        // Move to next position based on direction
        if (dir == 1) {  // Left to right
            pos = (pos + 1) % 4;
        } else if (dir == 2) {  // Right to left
            pos = (pos - 1 + 4) % 4;
        }
        
    } while (1);

    // Turn off all LEDs before exit
    for (int i = 0; i < 4; i++) {
        digitalWrite(leds[i], LOW);
    }
    
    printf("Program exited\n");
    return 0;
}