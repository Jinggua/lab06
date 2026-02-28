// 4 LED Marquee with 3 button control - WiringPi Version
// 完全保持原有引脚不变：
// B0 (GPIO5)  : move LEDs right to left
// B1 (GPIO6)  : move LEDs left to right
// B2 (GPIO27) : stop the program
// LEDs: GPIO22, GPIO23, GPIO24, GPIO25

#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>

int main(void) {
    // 初始化 WiringPi (使用GPIO编号模式)
    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi setup failed\n");
        return 1;
    }
    
    // 定义引脚 (使用BCM GPIO编号，和您原来完全一样)
    int LED1 = 22;   // GPIO22
    int LED2 = 23;   // GPIO23
    int LED3 = 24;   // GPIO24
    int LED4 = 25;   // GPIO25
    
    int B0 = 5;      // GPIO5  (右到左)
    int B1 = 6;      // GPIO6  (左到右)
    int B2 = 27;     // GPIO27 (退出)
    
    int leds[4] = {LED1, LED2, LED3, LED4};
    int pos = 0;
    int dir = 0;     // 0=STOP 1=LEFT TO RIGHT 2=RIGHT TO LEFT
    
    // 设置引脚模式
    for (int i = 0; i < 4; i++) {
        pinMode(leds[i], OUTPUT);
        digitalWrite(leds[i], LOW);  // 初始关闭
    }
    
    pinMode(B0, INPUT);
    pinMode(B1, INPUT);
    pinMode(B2, INPUT);
    
    // 如果您的开关是公共端接GND，启用内部上拉电阻
    pullUpDnControl(B0, PUD_UP);
    pullUpDnControl(B1, PUD_UP);
    pullUpDnControl(B2, PUD_UP);
    
    printf("程序开始运行...\n");
    printf("开关拨向左 (GPIO5): 从右到左\n");
    printf("开关拨向右 (GPIO6): 从左到右\n");
    printf("按下B2 (GPIO27): 退出程序\n");
    
    do {
        // 读取按钮状态 (0=按下, 1=松开，因为启用了上拉)
        int b0_state = digitalRead(B0);
        int b1_state = digitalRead(B1);
        
        // 调试信息
        printf("GPIO5: %d, GPIO6: %d\n", b0_state, b1_state);
        
        // 检测开关状态 (低电平有效)
        if (b0_state == 0) {      // 开关拨向左 (GPIO5接地)
            dir = 2;               // 从右到左
            printf("方向: 从右到左\n");
        } else if (b1_state == 0) { // 开关拨向右 (GPIO6接地)
            dir = 1;               // 从左到右
            printf("方向: 从左到右\n");
        }
        // 如果都不按下，dir保持原值
        
        // 关闭所有LED
        for (int i = 0; i < 4; i++) {
            digitalWrite(leds[i], LOW);
        }
        
        // 点亮当前位置的LED
        digitalWrite(leds[pos], HIGH);
        usleep(200000);  // 200ms
        
        // 根据方向移动位置
        if (dir == 1) {  // 从左到右
            pos = (pos + 1) % 4;
        } else if (dir == 2) {  // 从右到左
            pos = (pos - 1 + 4) % 4;
        }
        
    } while (digitalRead(B2) == 1);  // B2按下为0时退出
    
    // 关闭所有LED
    for (int i = 0; i < 4; i++) {
        digitalWrite(leds[i], LOW);
    }
    
    printf("程序退出\n");
    return 0;
}