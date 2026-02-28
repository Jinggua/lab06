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

    // SET GPIO 22, 23, 24, 25 FOR OUTPUT //
    // GPFSEL2 (GPIO+2), CLEAR bit6~17    //
    MASK = 0xFFFC003F;
    *(GPIO + 2) = *(GPIO + 2) & MASK;
    // SET 001 FOR EACH GPIO
    MASK = 0x00009240;
    *(GPIO + 2) = *(GPIO + 2) | MASK;

    // SET GPIO 27 FOR INPUT (B2 EXIT)
    MASK = 0xFF1FFFFF;
    *(GPIO + 2) = *(GPIO + 2) & MASK;

    // SET GPIO 5, 6 FOR INPUT (B0, B1)
    MASK = 0xFF807FFF;
    *(GPIO + 0) = *(GPIO + 0) & MASK;

    // TURN ALL LEDs OFF INITIALLY
    MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
    *(GPIO + 10) = MASK;

    do {
        // TURN ALL LEDs OFF
        MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
        *(GPIO + 10) = MASK;
        
        usleep(1000);
        
        // SET CURRENT LED HIGH
        if (pos == 0) {
            MASK = 0x00400000;  // GPIO22
            *(GPIO + 7) = MASK;
            printf("LED1 (GPIO22) 亮\n");
            usleep(500000);
        } else if (pos == 1) {
            MASK = 0x00800000;  // GPIO23
            *(GPIO + 7) = MASK;
            printf("LED2 (GPIO23) 亮\n");
            usleep(500000);
        } else if (pos == 2) {
            MASK = 0x01000000;  // GPIO24
            *(GPIO + 7) = MASK;
            printf("LED3 (GPIO24) 亮\n");
            usleep(500000);
        } else if (pos == 3) {
            MASK = 0x02000000;  // GPIO25
            *(GPIO + 7) = MASK;
            printf("LED4 (GPIO25) 亮\n");
            usleep(500000);
        }
        
        // 移动到下一个LED (0->1->2->3->0)
        pos = (pos + 1) % 4;
        
        // 检查退出按钮
        MASK = 0x08000000;
        BUTTON = *(GPIO + 13) & MASK;
        
    } while (BUTTON == 0);

    // TURN ALL LEDs OFF BEFORE EXIT
    MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
    *(GPIO + 10) = MASK;

    close(MEM);
}