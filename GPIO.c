/*
 * File: GPIO.c
 * Author: Dianyao Su
 * Date: 2026/02/27
 * Description: 4 LED Marquee with 3 button control using direct GPIO memory access.
 *              B0 (GPIO5) : move LEDs right to left
 *              B1 (GPIO6) : move LEDs left to right
 *              B2 (GPIO27): stop the program
 *              LEDs: GPIO22, GPIO23, GPIO24, GPIO25
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
    // USE VIRTUAL MEMORY SPACE FOR PI 4
    unsigned int BASE = 0xFE200000;

    // CREATE GPIO POINTER - EACH INCREMENT ADVANCES BY 4 BYTES
    volatile unsigned int *GPIO;
    int MEM, MASK;
    int BUTTON = 0;
    int pos = 0;   // CURRENT LED POSITION 0~3
    int dir = 0;   // 0=STOP 1=LEFT TO RIGHT 2=RIGHT TO LEFT

    // TEST FOR ROOT ACCESS
    if (getuid() != 0) {
        printf("ROOT PRIVILEGES REQUIRED\n");
        return 1;
    }

    // OPEN MEMORY INTERFACE
    if ((MEM = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        printf("CANNOT OPEN MEMORY INTERFACE\n");
        return 2;
    }

    // MAP GPIO MEMORY TO USER SPACE
    GPIO = (unsigned int *)mmap(0, getpagesize(), PROT_READ | PROT_WRITE,
        MAP_SHARED, MEM, BASE);

    if ((unsigned int)GPIO < 0) {
        printf("MEMORY MAPPING FAILED\n");
        return 3;
    }

    // SET GPIO 22, 23, 24, 25 AS OUTPUT
    // GPFSEL2 (GPIO+2), CLEAR BITS 6-17
    MASK = 0xFFFC003F;
    *(GPIO + 2) = *(GPIO + 2) & MASK;
    // SET 001 FOR EACH GPIO
    MASK = 0x00009240;
    *(GPIO + 2) = *(GPIO + 2) | MASK;

    // SET GPIO 27 AS INPUT (B2 EXIT)
    MASK = 0xFF1FFFFF;
    *(GPIO + 2) = *(GPIO + 2) & MASK;

    // SET GPIO 5, 6 AS INPUT (B0, B1)
    MASK = 0xFF807FFF;
    *(GPIO + 0) = *(GPIO + 0) & MASK;

    // TURN ALL LEDs OFF INITIALLY
    MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
    *(GPIO + 10) = MASK;

    do {
        // READ BUTTON STATES
        unsigned int gpio_level = *(GPIO + 13);
        
        // CHECK GPIO5 LOW - RIGHT TO LEFT
        if ((gpio_level & 0x00000020) == 0) {
            dir = 2;  // RIGHT TO LEFT
        }
        // CHECK GPIO6 LOW - LEFT TO RIGHT
        else if ((gpio_level & 0x00000040) == 0) {
            dir = 1;  // LEFT TO RIGHT
        }
        
        // TURN ALL LEDs OFF
        MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
        *(GPIO + 10) = MASK;
        
        usleep(1000);
        
        // LIGHT CURRENT LED
        if (pos == 0) {
            MASK = 0x00400000;  // GPIO22 (LED1)
            *(GPIO + 7) = MASK;
            usleep(200000);
        } else if (pos == 1) {
            MASK = 0x00800000;  // GPIO23 (LED2)
            *(GPIO + 7) = MASK;
            usleep(200000);
        } else if (pos == 2) {
            MASK = 0x01000000;  // GPIO24 (LED3)
            *(GPIO + 7) = MASK;
            usleep(200000);
        } else if (pos == 3) {
            MASK = 0x02000000;  // GPIO25 (LED4)
            *(GPIO + 7) = MASK;
            usleep(200000);
        }
        
        // MOVE TO NEXT POSITION BASED ON DIRECTION
        if (dir == 1) {  // LEFT TO RIGHT
            pos = (pos + 1) % 4;
        } else if (dir == 2) {  // RIGHT TO LEFT
            pos = (pos - 1 + 4) % 4;
        }
        
        // CHECK EXIT BUTTON (GPIO27 HIGH)
        MASK = 0x08000000;
        BUTTON = *(GPIO + 13) & MASK;
        
    } while (BUTTON == 0);

    // TURN ALL LEDs OFF BEFORE EXIT
    MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
    *(GPIO + 10) = MASK;

    close(MEM);
}