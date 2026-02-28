// P15 - PROGRAM GPIO MEM CONTROL              //
// 4 LED Marquee with 3 button control         //
// B0 (GPIO5) : move LEDs left to right        // 修改了注释
// B1 (GPIO6) : move LEDs right to left        // 修改了注释
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
    int dir = 0;   // 0=STOP 1=LEFT TO RIGHT 2=RIGHT TO LEFT //

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
    // 1111 1111 1111 1100 0000 0000 0011 1111 //
    MASK = 0xFFFC003F;
    *(GPIO + 2) = *(GPIO + 2) & MASK;
    // SET 001 FOR EACH GPIO               //
    // 0000 0000 0000 1001 0010 0100 0100 0000 //
    MASK = 0x00009240;
    *(GPIO + 2) = *(GPIO + 2) | MASK;

    // SET GPIO 27 FOR INPUT (B2 EXIT)     //
    // GPFSEL2 (GPIO+2), bit21~23          //
    // 1111 1111 0001 1111 1111 1111 1111 1111 //
    MASK = 0xFF1FFFFF;
    *(GPIO + 2) = *(GPIO + 2) & MASK;

    // SET GPIO 5, 6 FOR INPUT (B0, B1)    //
    // GPFSEL0 (GPIO+0), bit15~20          //
    // 1111 1111 1000 0000 0111 1111 1111 1111 //
    MASK = 0xFF807FFF;
    *(GPIO + 0) = *(GPIO + 0) & MASK;

    // TURN ALL LEDs OFF INITIALLY //
    MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;  // 0x03C00000
    *(GPIO + 10) = MASK;

    do {
        // CHECK B0 (GPIO5) FOR HIGH - LEFT TO RIGHT //
        MASK = 0x00000020;  // GPIO5的位
        if (*(GPIO + 13) & MASK) {
            dir = 1;  // 设置为从左到右
            printf("LEFT TO RIGHT\n");  // 可选的调试信息
        }

        // CHECK B1 (GPIO6) FOR HIGH - RIGHT TO LEFT //
        MASK = 0x00000040;  // GPIO6的位
        if (*(GPIO + 13) & MASK) {
            dir = 2;  // 设置为从右到左
            printf("RIGHT TO LEFT\n");  // 可选的调试信息
        }

        // TURN ALL LEDs OFF //
        MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
        *(GPIO + 10) = MASK;

        // 短暂延迟确保GPCLR生效
        usleep(1000);

        // SET CURRENT LED HIGH //
        if (pos == 0) {
            // SET GPIO 22 HIGH //
            MASK = 0x00400000;
            *(GPIO + 7) = MASK;
            usleep(200000);
            // REMOVE HIGH COMMAND //
            MASK = 0xFFBFFFFF;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        } else if (pos == 1) {
            // SET GPIO 23 HIGH //
            MASK = 0x00800000;
            *(GPIO + 7) = MASK;
            usleep(200000);
            MASK = 0xFF7FFFFF;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        } else if (pos == 2) {
            // SET GPIO 24 HIGH //
            MASK = 0x01000000;
            *(GPIO + 7) = MASK;
            usleep(200000);
            MASK = 0xFEFFFFFF;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        } else if (pos == 3) {
            // SET GPIO 25 HIGH //
            MASK = 0x02000000;
            *(GPIO + 7) = MASK;
            usleep(200000);
            MASK = 0xFDFFFFFF;
            *(GPIO + 7) = *(GPIO + 7) & MASK;
        }

        // MOVE TO NEXT POSITION BASED ON DIRECTION //
        if (dir == 1) {
            // LEFT TO RIGHT: 0->1->2->3->0 //
            pos = (pos + 1) % 4;
        } else if (dir == 2) {
            // RIGHT TO LEFT: 3->2->1->0->3 //
            pos = (pos - 1 + 4) % 4;
        }
        // 如果dir == 0，位置不变（停止）

        // CHECK GPIO 27 FOR HIGH - EXIT //
        MASK = 0x08000000;
        BUTTON = *(GPIO + 13) & MASK;

    } while (BUTTON == 0);

    // TURN ALL LEDs OFF BEFORE EXIT //
    MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
    *(GPIO + 10) = MASK;

    close(MEM);
}