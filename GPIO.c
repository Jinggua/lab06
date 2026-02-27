// TEST - ALL 4 LEDs ON                        //
// GPIO2, GPIO3, GPIO4, GPIO17 all HIGH        //
// Press GPIO27 to exit                        //
// PI 4 - GPIO memory starts at 0xFE200000     //
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
    // USE VIRTUAL MEMORY SPACE FOR PI 4 //
    unsigned int BASE = 0xFE200000;

    volatile unsigned int *GPIO;
    int MEM, MASK;
    int BUTTON = 0;

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

    // SET GPIO 2, 3, 4 FOR OUTPUT IN ONE GO //
    // GPFSEL0 (GPIO+0)                      //
    // GPIO2 = bit6~8  GPIO3 = bit9~11  GPIO4 = bit12~14 //
    // CLEAR bit6~14                          //
    // 1111 1111 1111 1111 1000 0000 0011 1111 //
    MASK = 0xFFFF803F;
    *(GPIO + 0) = *(GPIO + 0) & MASK;
    // SET bit6=1 bit9=1 bit12=1 (001 001 001) //
    // 0000 0000 0000 0001 0010 0100 0100 0000  //
    MASK = 0x00001240;
    *(GPIO + 0) = *(GPIO + 0) | MASK;

    // SET GPIO 17 FOR OUTPUT //
    // GPFSEL1 (GPIO+1), GPIO17 = bit21~23    //
    // 1111 1111 0001 1111 1111 1111 1111 1111 //
    // 0000 0000 0010 0000 0000 0000 0000 0000 //
    MASK = 0xFF1FFFFF;
    *(GPIO + 1) = *(GPIO + 1) & MASK;
    MASK = 0x00200000;
    *(GPIO + 1) = *(GPIO + 1) | MASK;

    // SET GPIO 27 FOR INPUT //
    // GPFSEL2 (GPIO+2), GPIO27 = bit21~23    //
    // 1111 1111 0001 1111 1111 1111 1111 1111 //
    MASK = 0xFF1FFFFF;
    *(GPIO + 2) = *(GPIO + 2) & MASK;

    // SET ALL 4 LEDs HIGH //
    // SET GPIO 2 HIGH //
    // 0000 0000 0000 0000 0000 0000 0000 0100 //
    MASK = 0x00000004;
    *(GPIO + 7) = *(GPIO + 7) | MASK;
    // REMOVE HIGH COMMAND //
    // 1111 1111 1111 1111 1111 1111 1111 1011 //
    MASK = 0xFFFFFFFB;
    *(GPIO + 7) = *(GPIO + 7) & MASK;

    // SET GPIO 3 HIGH //
    // 0000 0000 0000 0000 0000 0000 0000 1000 //
    MASK = 0x00000008;
    *(GPIO + 7) = *(GPIO + 7) | MASK;
    // REMOVE HIGH COMMAND //
    // 1111 1111 1111 1111 1111 1111 1111 0111 //
    MASK = 0xFFFFFFF7;
    *(GPIO + 7) = *(GPIO + 7) & MASK;

    // SET GPIO 4 HIGH //
    // 0000 0000 0000 0000 0000 0000 0001 0000 //
    MASK = 0x00000010;
    *(GPIO + 7) = *(GPIO + 7) | MASK;
    // REMOVE HIGH COMMAND //
    // 1111 1111 1111 1111 1111 1111 1110 1111 //
    MASK = 0xFFFFFFEF;
    *(GPIO + 7) = *(GPIO + 7) & MASK;

    // SET GPIO 17 HIGH //
    // 0000 0000 0000 0010 0000 0000 0000 0000 //
    MASK = 0x00020000;
    *(GPIO + 7) = *(GPIO + 7) | MASK;
    // REMOVE HIGH COMMAND //
    // 1111 1111 1111 1101 1111 1111 1111 1111 //
    MASK = 0xFFFDFFFF;
    *(GPIO + 7) = *(GPIO + 7) & MASK;

    printf("ALL LEDs ON - press GPIO27 to exit\n");

    // WAIT FOR GPIO27 BUTTON //
    do {
        // CHECK GPIO 27 FOR HIGH //
        // 0000 1000 0000 0000 0000 0000 0000 0000 //
        MASK = 0x08000000;
        BUTTON = *(GPIO + 13) & MASK;
        usleep(100000);
    } while (BUTTON == 0);

    // TURN ALL LEDs OFF BEFORE EXIT //
    // SET GPIO 2 LOW //
    MASK = 0x00000004;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFFFFFFFB;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 3 LOW //
    MASK = 0x00000008;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFFFFFFF7;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 4 LOW //
    MASK = 0x00000010;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFFFFFFEF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;
    // SET GPIO 17 LOW //
    MASK = 0x00020000;
    *(GPIO + 10) = *(GPIO + 10) | MASK;
    MASK = 0xFFFDFFFF;
    *(GPIO + 10) = *(GPIO + 10) & MASK;

    close(MEM);
}