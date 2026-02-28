// P15 - PROGRAM GPIO MEM CONTROL              //
// 4 LED Marquee with 3 button control         //
// B0 (GPIO5) : move LEDs right to left        //
// B1 (GPIO6) : move LEDs left to right        //
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
    // SET GPIO 22, 23, 24, 25 FOR OUTPUT IN ONE GO          //
    // ALL FOUR ARE IN GPFSEL2 (GPIO+2)                      //
    // GPIO22 = bit6~8   GPIO23 = bit9~11                    //
    // GPIO24 = bit12~14  GPIO25 = bit15~17                  //
    // CLEAR bit6~17 FIRST                                   //
    // 1111 1111 1111 1100 0000 0000 0011 1111               //
    MASK = 0xFFFC003F;
    *(GPIO + 2) = *(GPIO + 2) & MASK;
    // SET bit6=1(GPIO22) bit9=1(GPIO23)                     //
    //     bit12=1(GPIO24) bit15=1(GPIO25)                   //
    // 0000 0000 0000 1001 0010 0100 0100 0000               //
    MASK = 0x00009240;
    *(GPIO + 2) = *(GPIO + 2) | MASK;

    // ===================================================== //
    // SET BUTTON GPIO PINS FOR INPUT                        //
    // B0=GPIO5, B1=GPIO6 IN GPFSEL0 (GPIO+0)               //
    // GPIO5 = bit15~17   GPIO6 = bit18~20                  //
    // CLEAR bit15~20 (000=INPUT, no need to OR)             //
    // 1111 1111 1000 0000 0111 1111 1111 1111               //
    MASK = 0xFF807FFF;
    *(GPIO + 0) = *(GPIO + 0) & MASK;

    // B2=GPIO27 IN GPFSEL2 (GPIO+2), bit21~23              //
    // 1111 1111 0001 1111 1111 1111 1111 1111               //
    MASK = 0xFF1FFFFF;
    *(GPIO + 2) = *(GPIO + 2) & MASK;

    // ===================================================== //
    // TURN ALL LEDs OFF INITIALLY                           //
    // ===================================================== //

    // SET GPIO 22 LOW //
    // 0000 0000 0100 0000 0000 0000 0000 0000 //
    MASK = 0x00400000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    // REMOVE LOW COMMAND //
    // 1111 1111 1011 1111 1111 1111 1111 1111 //
    MASK = 0xFFBFFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 23 LOW //
    // 0000 0000 1000 0000 0000 0000 0000 0000 //
    MASK = 0x00800000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    // REMOVE LOW COMMAND //
    // 1111 1111 0111 1111 1111 1111 1111 1111 //
    MASK = 0xFF7FFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 24 LOW //
    // 0000 0001 0000 0000 0000 0000 0000 0000 //
    MASK = 0x01000000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    // REMOVE LOW COMMAND //
    // 1111 1110 1111 1111 1111 1111 1111 1111 //
    MASK = 0xFEFFFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 25 LOW //
    // 0000 0010 0000 0000 0000 0000 0000 0000 //
    MASK = 0x02000000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    // REMOVE LOW COMMAND //
    // 1111 1101 1111 1111 1111 1111 1111 1111 //
    MASK = 0xFDFFFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;

    // ===================================================== //
    // MAIN LOOP                                             //
    // ===================================================== //
    do {
        // CHECK B0 (GPIO5) FOR HIGH - RIGHT TO LEFT //
        // 0000 0000 0000 0000 0000 0000 0010 0000   //
        MASK = 0x00000020;
        if (*(GPIO + 13) & MASK) {
            dir = 1;
        }

        // CHECK B1 (GPIO6) FOR HIGH - LEFT TO RIGHT //
        // 0000 0000 0000 0000 0000 0000 0100 0000   //
        MASK = 0x00000040;
        if (*(GPIO + 13) & MASK) {
            dir = 2;
        }

        // TURN ALL LEDs OFF //
        // SET GPIO 22 LOW //
        // 0000 0000 0100 0000 0000 0000 0000 0000 //
        MASK = 0x00400000;
        *(GPIO + 10) = *(GPIO + 10) | MASK;
        // REMOVE LOW COMMAND //
        // 1111 1111 1011 1111 1111 1111 1111 1111 //
        MASK = 0xFFBFFFFF;
        *(GPIO + 10) = *(GPIO + 10) & MASK;
        // SET GPIO 23 LOW //
        // 0000 0000 1000 0000 0000 0000 0000 0000 //
        MASK = 0x00800000;
        *(GPIO + 10) = *(GPIO + 10) | MASK;
        // REMOVE LOW COMMAND //
        // 1111 1111 0111 1111 1111 1111 1111 1111 //
        MASK = 0xFF7FFFFF;
        *(GPIO + 10) = *(GPIO + 10) & MASK;
        // SET GPIO 24 LOW //
        // 0000 0001 0000 0000 0000 0000 0000 0000 //
        MASK = 0x01000000;
        *(GPIO + 10) = *(GPIO + 10) | MASK;
        // REMOVE LOW COMMAND //
        // 1111 1110 1111 1111 1111 1111 1111 1111 //
        MASK = 0xFEFFFFFF;
        *(GPIO + 10) = *(GPIO + 10) & MASK;
        // SET GPIO 25 LOW //
        // 0000 0010 0000 0000 0000 0000 0000 0000 //
        MASK = 0x02000000;
        *(GPIO + 10) = *(GPIO + 10) | MASK;
        // REMOVE LOW COMMAND //
        // 1111 1101 1111 1111 1111 1111 1111 1111 //
        MASK = 0xFDFFFFFF;
        *(GPIO + 10) = *(GPIO + 10) & MASK;

        // SET CURRENT LED HIGH //
        if (pos == 0) {
            // SET GPIO 22 HIGH //
            // 0000 0000 0100 0000 0000 0000 0000 0000 //
            MASK = 0x00400000;
            *(GPIO + 7) = *(GPIO + 7) | MASK;
            usleep(200000);
            // REMOVE HIGH COMMAND //
            // 1111 1111 1011 1111 1111 1111 1111 1111 //
            MASK = 0xFFBFFFFF;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        } else if (pos == 1) {
            // SET GPIO 23 HIGH //
            // 0000 0000 1000 0000 0000 0000 0000 0000 //
            MASK = 0x00800000;
            *(GPIO + 7) = *(GPIO + 7) | MASK;
            usleep(200000);
            // REMOVE HIGH COMMAND //
            // 1111 1111 0111 1111 1111 1111 1111 1111 //
            MASK = 0xFF7FFFFF;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        } else if (pos == 2) {
            // SET GPIO 24 HIGH //
            // 0000 0001 0000 0000 0000 0000 0000 0000 //
            MASK = 0x01000000;
            *(GPIO + 7) = *(GPIO + 7) | MASK;
            usleep(200000);
            // REMOVE HIGH COMMAND //
            // 1111 1110 1111 1111 1111 1111 1111 1111 //
            MASK = 0xFEFFFFFF;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        } else if (pos == 3) {
            // SET GPIO 25 HIGH //
            // 0000 0010 0000 0000 0000 0000 0000 0000 //
            MASK = 0x02000000;
            *(GPIO + 7) = *(GPIO + 7) | MASK;
            usleep(200000);
            // REMOVE HIGH COMMAND //
            // 1111 1101 1111 1111 1111 1111 1111 1111 //
            MASK = 0xFDFFFFFF;
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
    // SET GPIO 22 LOW //
    MASK = 0x00400000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFFBFFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 23 LOW //
    MASK = 0x00800000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFF7FFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 24 LOW //
    MASK = 0x01000000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFEFFFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 25 LOW //
    MASK = 0x02000000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFDFFFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;

    close(MEM);
}