#include <stm32f10x.h>
#include "stm32f10x_conf.h"  
#include "stm32f10x_tim.h" 
#include "system_f103.h"
#include "bsp_usart.h"
#include "bsp_adc.h"
#include "bsp_lcd_ili9341.h"  
#include "bsp_w25qxx.h"    

#define INPUT_SCALE_FACTOR (5.0f / 3.3f)
static uint16_t wave_pos = 0;         // 当前波形位置（适配320宽度）
static uint8_t screen_filled = 0;     // 屏幕填充标志
static void delay_ms(uint32_t ms)                       // 定义一个ms延时函数，减少移植时对外部文件依赖;
{
    while (ms--)                                        // 注意，本函数是简易延时，非精准延时
        for (uint32_t i = 0; i < 8000; i++);            // 72MHz系统时钟下，大约多少个空循环耗时1ms
}

#include <stdint.h>


uint8_t MapVoltageToY(float voltage_scaled) 
{
    // voltage_scaled范围0~5V映射到屏幕Y坐标（240像素）
    float y_float = (5.0f - voltage_scaled) * (120.0f / 5.0f)+70; // 0V→239, 5V→0
    y_float = (y_float < 0) ? 0 : (y_float > 239) ? 239 : y_float; // 强制限幅
    return (uint8_t)(y_float + 0.5f); 
}

void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        float v1 = ADC1_GetVoltage(GPIOA, GPIO_Pin_1);  // 0~3.3V
        float voltage_scaled = (v1) * (5.0f / 3.3f); // 转换为-2.5V~+2.5V
        char volt_str[16];
        sprintf(volt_str, "%+6.2fV", voltage_scaled - 2.5);
        LCD_String(20, 25, volt_str, 24, WHITE, BLACK);
        LCD_String(20, 8, "电压为", 16, WHITE, BLACK);

        uint16_t current_y = MapVoltageToY(voltage_scaled); // 修改后的Y坐标映射

        static int prev_x = -1;  // 初始化为无效值，表示无前一个点
        static int prev_y = -1;
        static int current_x = 0;
        static int screen_cleared = 0; // 新增变量，标记屏幕是否刚刚清空

        if (!screen_filled) {
            current_x = wave_pos;  // 当前点的x坐标
        } else {
            LCD_Fill(0, 0, 320, 240, BLACK); // 清空屏幕
            wave_pos = 0; // 重置波形位置
            screen_filled = 0; // 重置屏幕填充标志
            current_x = wave_pos;
            screen_cleared = 1; // 标记屏幕刚刚清空
        }

        // 如果有前一个点且屏幕未刚刚清空，则绘制线段
        if (prev_x != -1 && !screen_cleared) {
            int start_x = prev_x;
            // 使用LCD的画线函数连接前一点和当前点
            LCD_Line(start_x, prev_y, current_x, current_y, WHITE);
        }

        // 绘制当前点（覆盖线段终点以确保显示）
        LCD_DrawPoint(current_x, current_y, WHITE);

        // 更新前一个点的坐标
        prev_x = current_x;
        prev_y = current_y;

        if (!screen_filled) {
            wave_pos++;
            if (wave_pos >= 320) {
                screen_filled = 1;
                wave_pos = 319;  // 保持最大值，后续不再使用该变量
            }
        }

        screen_cleared = 0; // 重置屏幕清空标记
    }
}

void TIM2_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 2999;  // 调整定时器周期
    TIM_TimeBaseStructure.TIM_Prescaler = 7199;  // 调整预分频器
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

int main(void)
{

    USART1_Init(115200);                                         // 串口初始化：USART1(115200-N-8-1), 且工程已把printf重定向至USART1输出
    System_SwdMode();                                            // 设置芯片调试方式(SWD); 关闭JTAG只保留SWD; 目的:释放PB3、PB4、PA15，只需PA13、PA14
    W25qx_Init();                                                // 设备W25Q128： 引脚初始化，SPI初始化，测试连接, 测试是否存在字库
    LCD_Init();                                                  // 显示屏初始化
	  LCD_DisplayDir(5);                                  // 横屏模式，width=320，height=240
    TIM2_Init();                                                 // 初始化定时器2
	
    while (1)                                           // 死循环，不能让main函数运行结束，否则会产生硬件错误
    {
        // 主循环中不再需要延时和处理ADC采样
    }
}
