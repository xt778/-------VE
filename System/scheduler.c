/***********************************************************************************************************************************
 ** 【文件名称】  system_f103.c
 ** 【编写人员】  魔女开发板团队
 ** 【更新分享】  Q群文件夹       1126717453
 ** 【淘    宝】  魔女开发板      https://demoboard.taobao.com
 ***********************************************************************************************************************************
 ** 【使用说明】  1- 平台无关
 **               2- 全局函数：
 **                  vScheduler_TickCnt()   任务轮询值处理；本函数插在SysTick中断服务函数中，作用：任务轮询判断值;
 **                  vScheduler_Run()       任务轮询; while中不断调用本函数，以执行任务;
 **               3- 移植方法：
 **                  在SysTick中断服务函数的文件中：#include "scheduler.h"，以使其可调用本文件函数
 **                  vScheduler_TickCnt()放到SysTick中断服务函数中，如果Systick不是1ms中断一次，则要作代码修改;
 **                  vScheduler_Run()放到main的while循环中；
 **                  要间隔执行的代码，按需放到各vTask_xxms()函数中；
 **                  注意：每个vTask_xxms()任务函数的总运行时长，不要超过本函数的轮询时长；可使用System_TestRunTimes()测试运行时长
 **
 ** 【更新记录】  2020-04-21  创建
 **               2021-02-25  完善注释
 **  
==================================================================================================================================*/
#include "scheduler.h"
#include "hardware.h"




/*****************************************************************************
 ** 本地变量、函数声明定义
 *****************************************************************************/
static u16 usTick_1ms     =0;
static u16 usTick_2ms     =0;
static u16 usTick_5ms     =0;
static u16 usTick_10ms    =0;
static u16 usTick_50ms    =0;
static u16 usTick_100ms   =0;
static u16 usTick_500ms   =0;
static u16 usTick_1000ms = 0;

void vTask_1ms(void);
void vTask_2ms(void);
void vTask_5ms(void);
void vTask_10ms(void);
void vTask_50ms(void);
void vTask_100ms(void);
void vTask_500ms(void);
void vTask_1000ms(void);



/*============================================================================
 * 函  数：vScheduler_TickCnt
 * 功  能：任务轮询值处理; 本函数插在SysTick中断服务函数中，作用：任务轮询判断值
 * 参  数：
 * 返回值：
 * 备  注： 魔女开发板团队 2020年04月21日
============================================================================*/
void Scheduler_TickCnt(void)
{
    ++usTick_1ms;   
    ++usTick_2ms;   
    ++usTick_5ms;   
    ++usTick_10ms;  
    ++usTick_50ms;  
    ++usTick_100ms; 
    ++usTick_500ms; 
    ++usTick_1000ms;
}



/**============================================================================
 ** 函  数：vScheduler_Run
 ** 功  能：任务轮询; while中不断调用本函数，以执行任务
 ** 参  数：
 ** 返回值：
 ** 备  注：魔女开发板团队 2020年04月21日
=============================================================================*/
void Scheduler_Run(void)
{
    if( usTick_1ms   >=1)    { usTick_1ms=0;      vTask_1ms();   }   // 每1ms   执行1次的任务
    if( usTick_2ms   >=2)    { usTick_2ms=0;      vTask_2ms();   }   // 每2ms   执行1次的任务
    if( usTick_5ms   >=5)    { usTick_5ms=0;      vTask_5ms();   }   // 每5ms   执行1次的任务
    if( usTick_10ms  >=10)   { usTick_10ms=0;     vTask_10ms();  }   // 每10ms  执行1次的任务
    if( usTick_50ms  >=50)   { usTick_50ms=0;     vTask_50ms();  }   // 每50ms  执行1次的任务
    if( usTick_100ms >=100)  { usTick_100ms=0;    vTask_100ms(); }   // 每100ms 执行1次的任务
    if( usTick_500ms >=500)  { usTick_500ms=0;    vTask_500ms(); }   // 每500ms 执行1次的任务
    if( usTick_1000ms>=1000) { usTick_1000ms=0;   vTask_1000ms();}   // 每1000ms执行1次 的任务
}



/*****************************************************************************
 ** 1ms
 ** 每间隔1ms，被vScheduler_Run()调用一次
 ****************************************************************************/
void vTask_1ms(void)
{   
    // 每1ms执行一次的代码 
                
    XPT2046_TouchHandler();           // 触摸屏状态检测：1:检测是否有按下，2:如果有按下，就处理好按下的坐标                  
    
}



/*****************************************************************************
 ** 2ms
 ** 每间隔2ms，被vScheduler_Run()调用一次
 ****************************************************************************/
void vTask_2ms(void)
{  
    
    
    
}



/*****************************************************************************
 ** 5ms
 ** 每间隔5ms，被vScheduler_Run()调用一次
 ****************************************************************************/
void vTask_5ms(void)
{    
    
    
}



/*****************************************************************************
 ** 10ms
 ** 每间隔10ms，被vScheduler_Run()调用一次
 ****************************************************************************/
void vTask_10ms(void)
{    
    
    
    
}

/*****************************************************************************
 ** 50ms
 ** 每间隔50ms，被vScheduler_Run()调用一次
 ****************************************************************************/
void vTask_50ms(void)
{       

    
    // 4: ESP8266接收处理            
//    if(xUSART.USART2ReceivedFlag==1)                             // 检查ESP8266所用的UART4是否收到数据
//    {
//        sprintf(strTemp, "\rESP8266接收到第%3d次数据：%s\r", ++CNT_ESP8266, (char *)xUSART.USART2ReceivedBuffer);   // 格式化字符串        
//        USART1_SendString(strTemp);                              // 通过UART1把数据输出到电脑上位机 
//        xUSART.USART2ReceivedFlag=0;                             // 处理完后，把RS485所用的UART4接收到数据标志清0，以方便下一帧数据的接收
//    } // end if ESP8266    

}



/*****************************************************************************
 ** 100ms
 ** 每间隔100ms，被vScheduler_Run()调用一次
 ****************************************************************************/
void vTask_100ms(void)
{   
    static char strTemp[100];
    static uint16_t CNT_RS485=0;
    static uint16_t CNT_ESP8266=0;
     
    if(xUSART.USART1ReceivedFlag ==1)
    {
        // 1：打印到串口上位机              
        sprintf(strTemp, "USART1接收到新一帧数据：%s\r\n", (char*)xUSART.USART1ReceivedBuffer);
        USART1_SendString (strTemp);
        // 2: 通过CAN总线发出       
        CAN1_SendData((uint8_t*)xUSART.USART1ReceivedBuffer, 0x1234);
        sprintf(strTemp, "\r\nCAN发送的数据:>>> %s\r\n", (char*)xUSART.USART1ReceivedBuffer);
        USART1_SendString (strTemp);
        // 3: 通过RS485总线发出
        UART4_SendString((char*)xUSART.USART1ReceivedBuffer);
        sprintf(strTemp, "\r\nRS485发送的数据:>>> %s\r\n", (char*)xUSART.USART1ReceivedBuffer);
        USART1_SendString (strTemp);
        // 4: 通过ESP8266发出
        //USART2_SendString((char*)xUSART.USART1ReceivedBuffer);     
        // 清理USART1的接收标志        
        xUSART.USART1ReceivedFlag =0;
    }    
    
    // 2: CAN报文接收处理         
    if(xCAN.RxFlag ==1)                       // 通过接收标志xCan.RxFlag，判断是否收到新报文
    {
        static char strCAN[9];
        LED_BLUE_TOGGLE ;
        printf("\r\nCAN接收到数据:<<< ");  
        for(uint8_t i=0; i<8; i++)   
            strCAN[i]=xCAN.RxData.Data[i];  
        strCAN[8]=0;
        printf("%s\r\n",strCAN);                     
        xCAN.RxFlag = 0;                      // 处理完数据了，把接收标志清０ 
        // 判断是否测试工具发过来的数据
        if(strcmp(strCAN, "CAN_Test") ==0)
        {
            CAN1_SendData((uint8_t*)"CAN_OK", 0x1234);              
        }        
    } 
    
    // 3：RS485接收处理           
    if(xUSART.UART4ReceivedFlag==1)                             // 检查RS485所用的UART4是否收到数据
    {
        sprintf(strTemp, "\rRS485接收到第%3d次数据：%s\r", ++CNT_RS485, (char *)xUSART.UART4ReceivedBuffer);   // 格式化字符串        
        USART1_SendString(strTemp);                              // 通过UART1把数据输出到电脑上位机 
        xUSART.UART4ReceivedFlag=0;                             // 处理完后，把RS485所用的UART4接收到数据标志清0，以方便下一帧数据的接收
        // 判断是否测试工具发过来的数据
        if(strcmp((char*)xUSART.UART4ReceivedBuffer, "RS485_Test") ==0)
        {
            UART4_SendString("RS485_OK");             
        }
    }// end if RS485
    
}



/*****************************************************************************
 ** 500ms
 ** 每间隔500ms，被vScheduler_Run()调用一次
 ****************************************************************************/
void vTask_500ms(void)
{
    LED_RED_TOGGLE;             // 红色LED 每1秒闪灭一次，以监察系统正常工作         

}



/*****************************************************************************
 ** 1000ms
 ** 每间隔1000ms，被vScheduler_Run()调用一次
 ****************************************************************************/
void vTask_1000ms(void)
{    
    
    
}


