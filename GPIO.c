// P15 - PROGRAM GPIO MEM CONTROL              //
// 4 LED Marquee with 3 button control         //
// B0 (GPIO22): move LEDs right to left        //
// B1 (GPIO10): move LEDs left to right        //
// B2 (GPIO27): stop the program               //
// PI 4 - GPIO memory starts at 0xFE200000     //
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

main(void) {
    // USE VIRTUAL MEMORY SPACE FOR PI 4 //
    unsigned int BASE = 0xFE200000;

    // CREATE GPIO - A 4 BYTE        //
    // POINTER. INCREMENTING         //
    // GPIO BY 1 INCREASES THE       //
    // POINTER ADDRESS BY 4          //
    volatile unsigned int *GPIO;
    int MEM, MASK;
    int BUTTON = 0;
    int pos = 0;   // CURRENT LED POSITION 0~3 //
    int dir = 0;   // DIRECTION: 0=STOP 1=RIGHT TO LEFT 2=LEFT TO RIGHT //

    // TEST FOR ROOT ACCESS //
    if (getuid() != 0) {
        printf("ROOT PRIVILEGES REQUIRED\n");
        return 1;
    }

    // OPEN MEMORY INTERFACE //
    if ((MEM = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        printf("CANNOT OPEN MEMORY INTERFACE\n");
        return 2;
    }

    // SET POINTER GPIO TO //
    // CONTROL MEMORY      //
    // BASE ADDRESS (BASE) //
    GPIO = (unsigned int *)mmap(0, getpagesize(), PROT_READ | PROT_WRITE,
        MAP_SHARED, MEM, BASE);

    if ((unsigned int)GPIO < 0) {
        printf("MEMORY MAPPING FAILED\n");
        return 3;
    }

    // ===================================================== //
    // SET GPIO 2, 3, 4 FOR OUTPUT IN ONE GO                 //
    // ALL THREE ARE IN GPFSEL0 (GPIO+0)                     //
    // GPIO2 = bit6~8   GPIO3 = bit9~11   GPIO4 = bit12~14  //
    // CLEAR bit6~14 FIRST                                   //
    // 1111 1111 1111 1111 1000 0000 0011 1111               //
    MASK = 0xFFFF803F;
    *(GPIO + 0) = *(GPIO + 0) & MASK;
    // SET bit6=1(GPIO2) bit9=1(GPIO3) bit12=1(GPIO4)        //
    // 0000 0000 0000 0001 0010 0100 0100 0000               //
    MASK = 0x00001240;
    *(GPIO + 0) = *(GPIO + 0) | MASK;

    // SET GPIO 17 FOR OUTPUT //
    // GPFSEL1 (GPIO+1), GPIO17 occupies bit21~23 //
    // 1111 1111 0001 1111 1111 1111 1111 1111     //
    // 0000 0000 0010 0000 0000 0000 0000 0000     //
    MASK = 0xFF1FFFFF;
    *(GPIO + 1) = *(GPIO + 1) & MASK;
    MASK = 0x00200000;
    *(GPIO + 1) = *(GPIO + 1) | MASK;

    // ===================================================== //
    // SET BUTTON GPIO PINS FOR INPUT                        //
    // ===================================================== //

    // SET GPIO 10 FOR INPUT (B1 - LEFT TO RIGHT) //
    // GPFSEL1 (GPIO+1), GPIO10 occupies bit0~2   //
    // 1111 1111 1111 1111 1111 1111 1111 1000     //
    MASK = 0xFFFFFFF8;
    *(GPIO + 1) = *(GPIO + 1) & MASK;

    // SET GPIO 22 FOR INPUT (B0 - RIGHT TO LEFT) //
    // GPFSEL2 (GPIO+2), GPIO22 occupies bit6~8   //
    // 1111 1111 1111 1111 1111 1111 0001 1111     //
    MASK = 0xFFFFFF1F;
    *(GPIO + 2) = *(GPIO + 2) & MASK;

    // SET GPIO 27 FOR INPUT (B2 - EXIT)          //
    // GPFSEL2 (GPIO+2), GPIO27 occupies bit21~23 //
    // 1111 1111 0001 1111 1111 1111 1111 1111     //
    MASK = 0xFF1FFFFF;
    *(GPIO + 2) = *(GPIO + 2) & MASK;

    // ===================================================== //
    // TURN ALL LEDs OFF INITIALLY                           //
    // ===================================================== //

    // SET GPIO 17 LOW //
    // 0000 0000 0000 0010 0000 0000 0000 0000 //
    MASK = 0x00020000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    // REMOVE LOW COMMAND //
    // 1111 1111 1111 1101 1111 1111 1111 1111 //
    MASK = 0xFFFDFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 4 LOW //
    // 0000 0000 0000 0000 0000 0000 0001 0000 //
    MASK = 0x00000010;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    // REMOVE LOW COMMAND //
    // 1111 1111 1111 1111 1111 1111 1110 1111 //
    MASK = 0xFFFFFFEF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 3 LOW //
    // 0000 0000 0000 0000 0000 0000 0000 1000 //
    MASK = 0x00000008;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    // REMOVE LOW COMMAND //
    // 1111 1111 1111 1111 1111 1111 1111 0111 //
    MASK = 0xFFFFFFF7;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 2 LOW //
    // 0000 0000 0000 0000 0000 0000 0000 0100 //
    MASK = 0x00000004;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    // REMOVE LOW COMMAND //
    // 1111 1111 1111 1111 1111 1111 1111 1011 //
    MASK = 0xFFFFFFFB;
    *(GPIO + 10) = *(GPIO + 10) & MASK;

    // ===================================================== //
    // MAIN LOOP                                             //
    // ===================================================== //
    do {
        // CHECK B0 (GPIO22) FOR HIGH - RIGHT TO LEFT //
        // 0000 0000 0100 0000 0000 0000 0000 0000    //
        MASK = 0x00400000;
        if (*(GPIO + 13) & MASK) {
            dir = 1;
        }

        // CHECK B1 (GPIO10) FOR HIGH - LEFT TO RIGHT //
        // 0000 0000 0000 0000 0000 0100 0000 0000    //
        MASK = 0x00000400;
        if (*(GPIO + 13) & MASK) {
            dir = 2;
        }

        // TURN ALL LEDs OFF //
        // SET GPIO 17 LOW //
        // 0000 0000 0000 0010 0000 0000 0000 0000 //
        MASK = 0x00020000;
        *(GPIO + 10) = *(GPIO + 10) | MASK;
        // REMOVE LOW COMMAND //
        // 1111 1111 1111 1101 1111 1111 1111 1111 //
        MASK = 0xFFFDFFFF;
        *(GPIO + 10) = *(GPIO + 10) & MASK;
        // SET GPIO 4 LOW //
        // 0000 0000 0000 0000 0000 0000 0001 0000 //
        MASK = 0x00000010;
        *(GPIO + 10) = *(GPIO + 10) | MASK;
        // REMOVE LOW COMMAND //
        // 1111 1111 1111 1111 1111 1111 1110 1111 //
        MASK = 0xFFFFFFEF;
        *(GPIO + 10) = *(GPIO + 10) & MASK;
        // SET GPIO 3 LOW //
        // 0000 0000 0000 0000 0000 0000 0000 1000 //
        MASK = 0x00000008;
        *(GPIO + 10) = *(GPIO + 10) | MASK;
        // REMOVE LOW COMMAND //
        // 1111 1111 1111 1111 1111 1111 1111 0111 //
        MASK = 0xFFFFFFF7;
        *(GPIO + 10) = *(GPIO + 10) & MASK;
        // SET GPIO 2 LOW //
        // 0000 0000 0000 0000 0000 0000 0000 0100 //
        MASK = 0x00000004;
        *(GPIO + 10) = *(GPIO + 10) | MASK;
        // REMOVE LOW COMMAND //
        // 1111 1111 1111 1111 1111 1111 1111 1011 //
        MASK = 0xFFFFFFFB;
        *(GPIO + 10) = *(GPIO + 10) & MASK;

        // SET CURRENT LED HIGH //
        if (pos == 0) {
            // SET GPIO 2 HIGH //
            // 0000 0000 0000 0000 0000 0000 0000 0100 //
            MASK = 0x00000004;
            *(GPIO + 7) = *(GPIO + 7) | MASK;
            usleep(200000);
            // REMOVE HIGH COMMAND //
            // 1111 1111 1111 1111 1111 1111 1111 1011 //
            MASK = 0xFFFFFFFB;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        } else if (pos == 1) {
            // SET GPIO 3 HIGH //
            // 0000 0000 0000 0000 0000 0000 0000 1000 //
            MASK = 0x00000008;
            *(GPIO + 7) = *(GPIO + 7) | MASK;
            usleep(200000);
            // REMOVE HIGH COMMAND //
            // 1111 1111 1111 1111 1111 1111 1111 0111 //
            MASK = 0xFFFFFFF7;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        } else if (pos == 2) {
            // SET GPIO 4 HIGH //
            // 0000 0000 0000 0000 0000 0000 0001 0000 //
            MASK = 0x00000010;
            *(GPIO + 7) = *(GPIO + 7) | MASK;
            usleep(200000);
            // REMOVE HIGH COMMAND //
            // 1111 1111 1111 1111 1111 1111 1110 1111 //
            MASK = 0xFFFFFFEF;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        } else if (pos == 3) {
            // SET GPIO 17 HIGH //
            // 0000 0000 0000 0010 0000 0000 0000 0000 //
            MASK = 0x00020000;
            *(GPIO + 7) = *(GPIO + 7) | MASK;
            usleep(200000);
            // REMOVE HIGH COMMAND //
            // 1111 1111 1111 1101 1111 1111 1111 1111 //
            MASK = 0xFFFDFFFF;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        }

        // MOVE TO NEXT POSITION //
        if (dir == 1) {
            // RIGHT TO LEFT: 0->1->2->3->0 //
            pos = (pos + 1) % 4;
        } else if (dir == 2) {
            // LEFT TO RIGHT: 3->2->1->0->3 //
            pos = (pos - 1 + 4) % 4;
        }

        // CHECK GPIO 27 FOR HIGH - EXIT //
        // 0000 1000 0000 0000 0000 0000 0000 0000 //
        MASK = 0x08000000;
        BUTTON = *(GPIO + 13) & MASK;

    } while (BUTTON == 0);

    // TURN ALL LEDs OFF BEFORE EXIT //
    // SET GPIO 17 LOW //
    MASK = 0x00020000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFFFDFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 4 LOW //
    MASK = 0x00000010;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFFFFFFEF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 3 LOW //
    MASK = 0x00000008;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFFFFFFF7;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 2 LOW //
    MASK = 0x00000004;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFFFFFFFB;
    *(GPIO + 10) = *(GPIO + 10) & MASK;

    close(MEM);
}