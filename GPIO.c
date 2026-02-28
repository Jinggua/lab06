// P15 - PROGRAM GPIO MEM CONTROL - WITH LED AND BUTTON TEST //
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
    int dir = 0;   // 0=STOP 1=LEFT TO RIGHT 2=RIGHT TO LEFT

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
    
    printf("Program Started - Button Test Mode\n");
    printf("Format: GPIO5 GPIO6 GPIO27 | Direction | Position\n");

    do {
        // 读取所有GPIO状态
        unsigned int gpio_level = *(GPIO + 13);
        
        int b0 = (gpio_level & 0x00000020) ? 1 : 0;
        int b1 = (gpio_level & 0x00000040) ? 1 : 0;
        int b2 = (gpio_level & 0x08000000) ? 1 : 0;
        
        // 显示按钮状态
        printf("GPIO5:%d GPIO6:%d GPIO27:%d | ", b0, b1, b2);
        
        // 根据实际观察到的按钮状态来设置方向
        // 暂时注释掉原来的方向检测，改为手动观察
        /*
        if (b0 == 0) {
            dir = 2;
            printf("DIR:R->L ");
        } else if (b1 == 0) {
            dir = 1;
            printf("DIR:L->R ");
        } else {
            printf("DIR:STOP ");
        }
        */
        printf("DIR:-- ");
        
        // TURN ALL LEDs OFF
        MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
        *(GPIO + 10) = MASK;
        
        usleep(1000);
        
        // SET CURRENT LED HIGH
        if (pos == 0) {
            MASK = 0x00400000;  // GPIO22 (LED1)
            *(GPIO + 7) = MASK;
            printf("POS:0(LED1)\n");
            usleep(200000);
        } else if (pos == 1) {
            MASK = 0x00800000;  // GPIO23 (LED2)
            *(GPIO + 7) = MASK;
            printf("POS:1(LED2)\n");
            usleep(200000);
        } else if (pos == 2) {
            MASK = 0x01000000;  // GPIO24 (LED3)
            *(GPIO + 7) = MASK;
            printf("POS:2(LED3)\n");
            usleep(200000);
        } else if (pos == 3) {
            MASK = 0x02000000;  // GPIO25 (LED4)
            *(GPIO + 7) = MASK;
            printf("POS:3(LED4)\n");
            usleep(200000);
        }
        
        // 临时让LED自动循环，方便观察
        pos = (pos + 1) % 4;
        
        // 检查退出按钮 - 暂时禁用
        // MASK = 0x08000000;
        // BUTTON = *(GPIO + 13) & MASK;
        
    } while (1);  // 无限循环，按Ctrl+C退出

    // TURN ALL LEDs OFF BEFORE EXIT
    MASK = 0x00400000 | 0x00800000 | 0x01000000 | 0x02000000;
    *(GPIO + 10) = MASK;

    close(MEM);
}