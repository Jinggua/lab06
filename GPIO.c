// P15 - PROGRAM GPIO MEM CONTROL //
// Blink GPIO-17 until a HIGH on GPIO-27. //
// PI 3 - GPIO memory starts at 0x3F200000 //
// PI 4 - GPIO memory starts at 0xFE200000 //
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
main(void) {
  // USE VIRTUAL MEMORY SPACE FOR PI 3 //
  unsigned int BASE = 0x3F200000;
  // unsigned int BASE = 0xFE200000;
  // CREATE GPIO - A 4 BYTE //
  // POINTER. INCREMENTING //
  // GPIO BY 1 INCREASES THE //
  // POINTER ADDRESS BY 4 //
  volatile unsigned int *GPIO;
  int MEM, i, MASK;
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
  // CONTROL MEMORY //
  // BASE ADDRESS (BASE) //
  GPIO = (unsigned int *)mmap(0, getpagesize(), PROT_READ | PROT_WRITE,
  MAP_SHARED, MEM, BASE);
  if ((unsigned int)GPIO < 0) {
  printf("MEMORY MAPPING FAILED\n");
  return 3;
  }
  // SET GPIO 17 FOR OUTPUT //
  // 1111 1111 0001 1111 1111 1111 1111 1111 //
  // 0000 0000 0010 0000 0000 0000 0000 0000 //
  MASK = 0xFF1FFFFF;
  *(GPIO + 1) = *(GPIO + 1) & MASK;
  MASK = 0x00200000;
  *(GPIO + 1) = *(GPIO + 1) | MASK;
  // SET GPIO 27 FOR INPUT //
  // 1111 1111 0001 1111 1111 1111 1111 1111 //
  MASK = 0xFF1FFFFF;
  *(GPIO + 2) = *(GPIO + 2) & MASK;
  do {
  // SET GPIO 17 HIGH //
  // 0000 0000 0000 0010 0000 0000 0000 0000 //
  MASK = 0x00020000;
  *(GPIO + 7) = *(GPIO + 7) | MASK;
  usleep(100000);
  // REMOVE HIGH COMMAND //
  // 1111 1111 1111 1101 1111 1111 1111 1111 //
  MASK = 0xFFFDFFFF;
  *(GPIO + 7) = *(GPIO + 7) & MASK;
  // SET GPIO 17 LOW //
  // 0000 0000 0000 0010 0000 0000 0000 0000 //
  MASK = 0x00020000;
  *(GPIO + 10) = *(GPIO + 10) | MASK;
  usleep(100000);
  // REMOVE LOW COMMAND //
  // 1111 1111 1111 1101 1111 1111 1111 1111 //
  MASK = 0xFFFDFFFF;
  *(GPIO + 10) = *(GPIO + 10) & MASK;
  // CHECK GPIO 27 FOR HIGH //
  // 0000 1000 0000 0000 0000 0000 0000 0000 //
  MASK = 0x08000000;
  BUTTON = *(GPIO + 13) & MASK;
  } while (BUTTON == 0);
  close(MEM);
}