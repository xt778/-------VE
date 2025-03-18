#include "bsp_adc.h"



static uint8_t FLAG_ADC1InitOK = 0;                     // ADC1��ʼ����ǣ�0��δ��ʼ����1���ѳ�ʼ��
static uint16_t adcValue = 0;                           // ADC�ɼ����ƽ��ֵ�����ڼ����ѹֵ



static void delay_us(uint32_t us)                       // ����һ��us��ʱ������������ֲʱ���ⲿ�ļ�����;
{
    while (us--)                                        // ע�⣬�������Ǽ�����ʱ���Ǿ�׼��ʱ
        for (uint32_t i = 0; i < 7; i++);               // 72MHzϵͳʱ���£���Լ���ٸ���ѭ����ʱ1us
}



/******************************************************************************
 * ��  ���� configADC1
 * ��  �ܣ� ��ʼ��ADC
 * ��  ����
 * ����ֵ�� 
 *
 * ��  ע�� 1-ADC�¶�ͨ���Ĳ���ʱ�䣬оƬ�ֲ����Ƽ�Ϊ17.1us, ԶС�ڴ�ֵʱ���ݳ���
 *
 ******************************************************************************/
static void  configADC1(void)
{
    RCC->APB2ENR  |= 1 << 9;           // ADC1ʱ��ʹ��
    RCC->APB2RSTR |= 1 << 9;           // ADC1��λ
    RCC->APB2RSTR &= ~(1 << 9);        // ��λ����

    RCC->CFGR &= ~(3 << 14);           // ��Ƶ��������
    RCC->CFGR |= 2 << 14;              // SYSCLK/DIV2=72/6=12M ADCʱ������Ϊ12M,ADC���ʱ�Ӳ��ܳ���14M!

    ADC1->CR1  = 0 ;                   // ��0 , CR1���ù���ģʽ
    ADC1->CR1 |= 0 << 8;               // ɨ��ģʽ(���ͨ��)  0:�ر�   1��ʹ��
    ADC1->CR1 |= 0 << 16;              // ��������ģʽ

    ADC1->CR2  = 0;                    // ��0, CR2���ù�������
    ADC1->CR2 |= 0 << 1;               // ����ת��   0������ת��   1������ת��
    ADC1->CR2 |= 0 << 11;              // �Ҷ���
    ADC1->CR2 |= 7 << 17;              // �������ת��
    ADC1->CR2 |= 1 << 20;              // ʹ�����ⲿ����(SWSTART)!!!    ����ʹ��һ���¼�������
    ADC1->CR2 |= 0 << 23;              // ʹ���ڲ�оƬ�ڲ��¶�ͨ��16���ڲ�Vrefintͨ��17

    ADC1->SQR1 = 0 ;                   // ��0
    ADC1->SQR1 |= 0 << 20;             // ��Ҫת����ͨ������ �״�����=N+1

    ADC1->SMPR1 = 0x00FDB6DB;          // ͨ��17~10�������ڣ�ͨ��16��17�Ĳ���ʱ������Ϊ239.5���ڣ���(1-12MHz)*239.5����=19.9us(�ֲ��Ƽ�ֵ17us), ͨ��10~15����ʱ������Ϊ28.5���ڣ���2.3us
    ADC1->SMPR2 = 0x1B6DB6DB;          // ͨ�� 9~ 0��������, ͨ��9~0����ʱ������Ϊ28.5���ڣ���2.3us

    ADC1->CR2  |= 1 << 0;              // ����ADת����
    ADC1->CR2  |= 1 << 3;              // ʹ�ܸ�λУ׼
    while (ADC1->CR2 & 1 << 3);        // �ȴ�У׼����, ��λ��������ò���Ӳ���������У׼�Ĵ�������ʼ�����λ�������
    ADC1->CR2  |= 1 << 2;              // ����ADУ׼
    while (ADC1->CR2 & 1 << 2);        // �ȴ�У׼����, ��λ��������ò���Ӳ���������У׼�Ĵ�������ʼ�����λ�������

    delay_us(5);                      // ����ʱ�������ȶ�

    // ����DMA
    RCC->AHBENR |= 1 << 0;             // DMA1ʱ��ʹ��
    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;  // �����ַ
    DMA1_Channel1->CMAR = (uint32_t)&adcValue;  // �ڴ��ַ
    DMA1_Channel1->CNDTR = 1;          // ���ݴ�����
    DMA1_Channel1->CCR = 0x00002520;   // ����DMA����ģʽ

    ADC1->CR2 |= 1 << 8;               // ADC1 DMAʹ��

    FLAG_ADC1InitOK = 1;               // ��ʼ���󣬱�־��1
}



// ��ʼ������Ҫ������
static void adc_GPIOInit(GPIO_TypeDef *GPIOx, uint32_t PINx)
{
    // ʹ���������Ŷ˿�ʱ�ӣ�ʹ�ö˿��жϵķ���ʹ��ʱ��, ��ʹ������ֲ������
    if (GPIOx == GPIOA)  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    if (GPIOx == GPIOB)  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    if (GPIOx == GPIOC)  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    if (GPIOx == GPIOD)  RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
    if (GPIOx == GPIOE)  RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;
#if defined (STM32F10X_HD)
    if (GPIOx == GPIOF)  RCC->APB2ENR |= RCC_APB2ENR_IOPFEN;
    if (GPIOx == GPIOG)  RCC->APB2ENR |= RCC_APB2ENR_IOPGEN;
#endif
    // �����������Ϊģ������ģʽ; ADC1��15��ͨ�����ֲ���PA��PB��PC��0~7����
    if (PINx == GPIO_Pin_0)   GPIOx->CRL &= 0xFFFFFFF0;  // GPIO_Pin_0��ģ������
    if (PINx == GPIO_Pin_1)   GPIOx->CRL &= 0xFFFFFF0F;  // GPIO_Pin_1��ģ������
    if (PINx == GPIO_Pin_2)   GPIOx->CRL &= 0xFFFFF0FF;  // GPIO_Pin_2��ģ������
    if (PINx == GPIO_Pin_3)   GPIOx->CRL &= 0xFFFF0FFF;  // GPIO_Pin_3��ģ������
    if (PINx == GPIO_Pin_4)   GPIOx->CRL &= 0xFFF0FFFF;  // GPIO_Pin_4��ģ������
    if (PINx == GPIO_Pin_5)   GPIOx->CRL &= 0xFF0FFFFF;  // GPIO_Pin_5��ģ������
    if (PINx == GPIO_Pin_6)   GPIOx->CRL &= 0xF0FFFFFF;  // GPIO_Pin_6��ģ������
    if (PINx == GPIO_Pin_6)   GPIOx->CRL &= 0x0FFFFFFF;  // GPIO_Pin_7��ģ������
}



// ������ADC1��2��ͨ��GPIO�˿ڡ����źţ�����ͨ����
// ���أ�������0~15��ʧ�ܣ�99
static uint8_t adc_PinConvertChannel(GPIO_TypeDef *GPIOx, uint32_t PINx)
{
    if ((GPIOx == GPIOA) && (PINx == GPIO_Pin_0))    return  0; // PA0 = ͨ�� 0
    if ((GPIOx == GPIOA) && (PINx == GPIO_Pin_1))    return  1; // PA1 = ͨ�� 1
    if ((GPIOx == GPIOA) && (PINx == GPIO_Pin_2))    return  2; // PA2 = ͨ�� 2
    if ((GPIOx == GPIOA) && (PINx == GPIO_Pin_3))    return  3; // PA3 = ͨ�� 3
    if ((GPIOx == GPIOA) && (PINx == GPIO_Pin_4))    return  4; // PA4 = ͨ�� 4
    if ((GPIOx == GPIOA) && (PINx == GPIO_Pin_5))    return  5; // PA5 = ͨ�� 5
    if ((GPIOx == GPIOA) && (PINx == GPIO_Pin_6))    return  6; // PA6 = ͨ�� 6
    if ((GPIOx == GPIOA) && (PINx == GPIO_Pin_7))    return  7; // PA7 = ͨ�� 7
    if ((GPIOx == GPIOB) && (PINx == GPIO_Pin_0))    return  8; // PB0 = ͨ�� 8
    if ((GPIOx == GPIOB) && (PINx == GPIO_Pin_1))    return  9; // PB1 = ͨ�� 9
    if ((GPIOx == GPIOC) && (PINx == GPIO_Pin_0))    return 10; // PC0 = ͨ��10
    if ((GPIOx == GPIOC) && (PINx == GPIO_Pin_1))    return 11; // PC1 = ͨ��11
    if ((GPIOx == GPIOC) && (PINx == GPIO_Pin_2))    return 12; // PC2 = ͨ��12
    if ((GPIOx == GPIOC) && (PINx == GPIO_Pin_3))    return 13; // PC3 = ͨ��13
    if ((GPIOx == GPIOC) && (PINx == GPIO_Pin_4))    return 14; // PC4 = ͨ��14
    if ((GPIOx == GPIOC) && (PINx == GPIO_Pin_5))    return 15; // PC5 = ͨ��15
    return 99; // ���û�ҵ� ƥ��Ĳ������ͷ��ش����
}


// Ϊ��ֲʱ��������ṹ�������ù�ϵ������������Ϊȫ�ֺ���
// ��ͨ�����ж�β�ֵ��������ƽ��ֵ
// ������  uint8_t channel: ͨ��ֵ����ѡ��Χ0~17
//         uint8_t times:   ��ֲ����
// ����ֵ����β�ֵ���ƽ��ֵ��ֵ��Χ��0~4095
static uint16_t getADC1Value(uint8_t channel, uint8_t times)
{
    uint32_t valueSUM = 0;                        // ADC�Ķ��ȡֵ�ۼӽ��

    if (FLAG_ADC1InitOK == 0)  configADC1();      // ����ADC1�Ĺ���ģʽ

    if (channel == 16 || channel == 17)           // ����ǵ�16��17ͨ������ʹ��TSVREFEλ
        ADC1->CR2 |= 1 << 23;                     // ʹ�� �¶ȴ�����

    ADC1->SQR3 = channel;                         // ��1��Ҫת����ͨ��,�ڲ��ο���ѹת��ֵͨ��

    for (u8 t = 0; t < times; t++)                // �����ֵ�����ۼӳɽ��ֵ
    {
        DMA1_Channel1->CNDTR = 1;                 // �������ݴ�����
        DMA1_Channel1->CCR |= 1 << 0;             // ����DMA����
        ADC1->CR2 |= 1 << 22;                     // ��������ת��ͨ��
        while (!(DMA1->ISR & 1 << 1));            // �ȴ�DMA�������
        DMA1->IFCR |= 1 << 1;                     // ���DMA������ɱ�־
        valueSUM = valueSUM + adcValue;           // ���ӱ��β�ֵ
        delay_us(2);                              // ��Ϊ��ʱһ��
    }

    return valueSUM / times;                      // ����ƽ��ֵ
}




/******************************************************************************
 * ��  ���� ADC1_GetVoltage
 * ��  �ܣ� ��ȡͨ�������ϵĵ�ѹֵ
 *
 * ��  ���� GPIO_TypeDef*  GPIOx   GPIO�˿�, ��ѡ������GPIOA ~ GPIOG
 *          uint32_t       PINx    ���ţ�    ��ѡ������GPIO_Pin_0 ~ GPIO_Pin_15
 *
 * ����ֵ�� flloat ��ѹֵ����Χ��0~3.3V
 *
 * ��  ע�� 1-�˺�����ֱ�ӵ��ü��ɣ��������ⲿ��ADC��GPIO��ʼ��; �����ؽ����ѹֵ��
 *          2-�����е�1�ε��ô˺����������ڹ�����ʱԼ2ms����Ϊ����Ҫ��ADC����ʼ��;
 *          3-��2���𣬵��ô˺�����ÿ�κ�ʱԼ: 25us;
 *          4-��ϵͳʵʱ��Ҫ��ϸߣ�������жϡ�DMA;��
 *          
 ******************************************************************************/
float ADC1_GetVoltage(GPIO_TypeDef *GPIOx, uint32_t PINx)
{
    static uint8_t  channel  = 0;                 // ͨ����
    static uint16_t adcValue = 0;                 // ADC�ɼ����ƽ��ֵ�����ڼ����ѹֵ

    channel = adc_PinConvertChannel(GPIOx, PINx); // �������ţ��ж�����ADC1��ͨ��
    if (channel == 99)  return 0;                 // ����������ź������˳�������0

    adc_GPIOInit(GPIOx, PINx);                    // ��ʼ������Ϊģ������ģʽ
    adcValue = getADC1Value(channel, 3);          // ��β�ֵ���, ���ص�ƽ��ֵ

    return (float)adcValue / 1240.9;              // ���ص�ѹֵ; �� adcValue*(3.3/4095)�ļ�ֵ;
}




