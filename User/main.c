#include <stm32f10x.h>
#include "stm32f10x_conf.h"  
#include "stm32f10x_tim.h" 
#include "system_f103.h"
#include "bsp_usart.h"
#include "bsp_adc.h"
#include "bsp_lcd_ili9341.h"  
#include "bsp_w25qxx.h"    

#define INPUT_SCALE_FACTOR (5.0f / 3.3f)
static uint16_t wave_pos = 0;         // ��ǰ����λ�ã�����320��ȣ�
static uint8_t screen_filled = 0;     // ��Ļ����־
static void delay_ms(uint32_t ms)                       // ����һ��ms��ʱ������������ֲʱ���ⲿ�ļ�����;
{
    while (ms--)                                        // ע�⣬�������Ǽ�����ʱ���Ǿ�׼��ʱ
        for (uint32_t i = 0; i < 8000; i++);            // 72MHzϵͳʱ���£���Լ���ٸ���ѭ����ʱ1ms
}

#include <stdint.h>


uint8_t MapVoltageToY(float voltage_scaled) 
{
    // voltage_scaled��Χ0~5Vӳ�䵽��ĻY���꣨240���أ�
    float y_float = (5.0f - voltage_scaled) * (120.0f / 5.0f)+70; // 0V��239, 5V��0
    y_float = (y_float < 0) ? 0 : (y_float > 239) ? 239 : y_float; // ǿ���޷�
    return (uint8_t)(y_float + 0.5f); 
}

void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

        float v1 = ADC1_GetVoltage(GPIOA, GPIO_Pin_1);  // 0~3.3V
        float voltage_scaled = (v1) * (5.0f / 3.3f); // ת��Ϊ-2.5V~+2.5V
        char volt_str[16];
        sprintf(volt_str, "%+6.2fV", voltage_scaled - 2.5);
        LCD_String(20, 25, volt_str, 24, WHITE, BLACK);
        LCD_String(20, 8, "��ѹΪ", 16, WHITE, BLACK);

        uint16_t current_y = MapVoltageToY(voltage_scaled); // �޸ĺ��Y����ӳ��

        static int prev_x = -1;  // ��ʼ��Ϊ��Чֵ����ʾ��ǰһ����
        static int prev_y = -1;
        static int current_x = 0;
        static int screen_cleared = 0; // ���������������Ļ�Ƿ�ո����

        if (!screen_filled) {
            current_x = wave_pos;  // ��ǰ���x����
        } else {
            LCD_Fill(0, 0, 320, 240, BLACK); // �����Ļ
            wave_pos = 0; // ���ò���λ��
            screen_filled = 0; // ������Ļ����־
            current_x = wave_pos;
            screen_cleared = 1; // �����Ļ�ո����
        }

        // �����ǰһ��������Ļδ�ո���գ�������߶�
        if (prev_x != -1 && !screen_cleared) {
            int start_x = prev_x;
            // ʹ��LCD�Ļ��ߺ�������ǰһ��͵�ǰ��
            LCD_Line(start_x, prev_y, current_x, current_y, WHITE);
        }

        // ���Ƶ�ǰ�㣨�����߶��յ���ȷ����ʾ��
        LCD_DrawPoint(current_x, current_y, WHITE);

        // ����ǰһ���������
        prev_x = current_x;
        prev_y = current_y;

        if (!screen_filled) {
            wave_pos++;
            if (wave_pos >= 320) {
                screen_filled = 1;
                wave_pos = 319;  // �������ֵ����������ʹ�øñ���
            }
        }

        screen_cleared = 0; // ������Ļ��ձ��
    }
}

void TIM2_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 2999;  // ������ʱ������
    TIM_TimeBaseStructure.TIM_Prescaler = 7199;  // ����Ԥ��Ƶ��
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

    USART1_Init(115200);                                         // ���ڳ�ʼ����USART1(115200-N-8-1), �ҹ����Ѱ�printf�ض�����USART1���
    System_SwdMode();                                            // ����оƬ���Է�ʽ(SWD); �ر�JTAGֻ����SWD; Ŀ��:�ͷ�PB3��PB4��PA15��ֻ��PA13��PA14
    W25qx_Init();                                                // �豸W25Q128�� ���ų�ʼ����SPI��ʼ������������, �����Ƿ�����ֿ�
    LCD_Init();                                                  // ��ʾ����ʼ��
	  LCD_DisplayDir(5);                                  // ����ģʽ��width=320��height=240
    TIM2_Init();                                                 // ��ʼ����ʱ��2
	
    while (1)                                           // ��ѭ����������main�������н�������������Ӳ������
    {
        // ��ѭ���в�����Ҫ��ʱ�ʹ���ADC����
    }
}
