#ifndef      __BSP_ILI9341_LCD_H
#define         __BSP_ILI9341_LCD_H
/***********************************************************************************************************************************
    *   @file        lcd_ili9341.h
    *   @date     2020-12-13        
    *   @author   魔女开发板   老周
    *   @brief    更新下载     更新Q群文件夹：262901124
    *   @taobao   淘宝硬件     ttps://demoboard.taobao.com/
    ********************************************************************************************************************************
    *   【实验平台】 
    *    魔女开发板_STM32F103VE + KEIL5.27 + 2.8寸显示屏_ILI9341
    *
    *   【移植说明】 
    *    1：本代码使用在F103VC上，使用FSMC模拟8080接口进行，注意引脚的修改
    *    2：汉字的显示，使用开发板上的外部FLASH中字库
    *
    *   【FSMC_Bank说明】
    *    共有4个Bank
    *    64MB:FSMC_Bank1_NORSRAM1:0X6000 0000 ~ 0X63FF FFFF
    *    64MB:FSMC_Bank1_NORSRAM2:0X6400 0000 ~ 0X67FF FFFF
    *    64MB:FSMC_Bank1_NORSRAM3:0X6800 0000 ~ 0X6BFF FFFF
    *    64MB:FSMC_Bank1_NORSRAM4:0X6C00 0000 ~ 0X6FFF FFFF
    *    每个 BANK 有4*64MB = 256MB(为什么Bank内要分4个块：寻址能力 2^26 =0X0400 0000 = 64MB)
    *    选择BANK1-BORSRAM1 连接 TFT，地址范围为0X6000 0000 ~ 0X63FF FFFF
    *    LCD的DC(寄存器/数据选择)脚接FSMC_A16地址脚，当选择不同的地址线时，以下地址要重新计算  
    *    寄存器基地址 = 0X60000000
    *    RAM基地址 = 0X60000000 + 2^16*2 = 0X60000000 + 0X20000 = 0X60020000
    *     
    *   【更新记录】
    *    2020-12-19  根据原子、野火代码，重写
    *    2020-12-21  增加颜色值、增加调用标准库FSMC时的块定义
    
************************************************************************************************************************************/
#include "stm32f10x.h"




/**************************** 移植修改 ILI9341 8080通讯引脚定义 ***************************/
/******控制信号线******/
//片选，选择NOR/SRAM块
#define      ILI9341_CS_PORT               GPIOD
#define      ILI9341_CS_PIN                GPIO_Pin_7
//PD11为FSMC_A16
#define      ILI9341_DC_PORT               GPIOD        // DC引脚，使用FSMC的地址信号控制，本引脚决定了访问LCD时使用的地址
#define      ILI9341_DC_PIN                GPIO_Pin_11
//写使能
#define      ILI9341_WR_PORT               GPIOD
#define      ILI9341_WR_PIN                GPIO_Pin_5
//读使能
#define      ILI9341_RD_PORT               GPIOD
#define      ILI9341_RD_PIN                GPIO_Pin_4
//复位引脚
//#define      ILI9341_RST_PORT              GPIOE
//#define      ILI9341_RST_PIN               GPIO_Pin_1
//背光引脚
#define      ILI9341_BK_PORT               GPIOA
#define      ILI9341_BK_PIN                GPIO_Pin_15
/******数据信号线*******/
#define      ILI9341_D0_PORT               GPIOD
#define      ILI9341_D0_PIN                GPIO_Pin_14

#define      ILI9341_D1_PORT               GPIOD
#define      ILI9341_D1_PIN                GPIO_Pin_15

#define      ILI9341_D2_PORT               GPIOD
#define      ILI9341_D2_PIN                GPIO_Pin_0

#define      ILI9341_D3_PORT               GPIOD
#define      ILI9341_D3_PIN                GPIO_Pin_1

#define      ILI9341_D4_PORT               GPIOE
#define      ILI9341_D4_PIN                GPIO_Pin_7

#define      ILI9341_D5_PORT               GPIOE
#define      ILI9341_D5_PIN                GPIO_Pin_8

#define      ILI9341_D6_PORT               GPIOE
#define      ILI9341_D6_PIN                GPIO_Pin_9

#define      ILI9341_D7_PORT               GPIOE
#define      ILI9341_D7_PIN                GPIO_Pin_10

#define      ILI9341_D8_PORT               GPIOE
#define      ILI9341_D8_PIN                GPIO_Pin_11

#define      ILI9341_D9_PORT               GPIOE
#define      ILI9341_D9_PIN                GPIO_Pin_12

#define      ILI9341_D10_PORT              GPIOE
#define      ILI9341_D10_PIN               GPIO_Pin_13

#define      ILI9341_D11_PORT              GPIOE
#define      ILI9341_D11_PIN               GPIO_Pin_14

#define      ILI9341_D12_PORT              GPIOE
#define      ILI9341_D12_PIN               GPIO_Pin_15

#define      ILI9341_D13_PORT              GPIOD
#define      ILI9341_D13_PIN               GPIO_Pin_8

#define      ILI9341_D14_PORT              GPIOD
#define      ILI9341_D14_PIN               GPIO_Pin_9

#define      ILI9341_D15_PORT              GPIOD
#define      ILI9341_D15_PIN               GPIO_Pin_10

/****** 屏幕参数 *******/
#define LCD_WIDTH         240            // 屏幕宽度像素，注意：0~239
#define LCD_HIGH          320            // 屏幕高度像素，注意：0~319
#define LCD_DIR           0              // 默认显示方向，0-正竖屏，3-倒竖屏，5-正横屏, 6-倒横屏

//片选引脚决定的NOR/SRAM块
#define      FSMC_Bank1_NORSRAMx           FSMC_Bank1_NORSRAM1
// 结束 移植修改，下方代码不用修改 *********************************************************
    
    
    
/******************************* 定义 LCD 信息结构体 ***************************************/
typedef struct          // 结构体：用于保存信息、状态
{     
    u8  FlagInit;       // 初始化完成标志
    u16 width;          // LCD 宽度
    u16 height;         // LCD 高度
    u8  dir;            // 横屏还是竖屏控制：0，竖屏；1，横屏
    u16 id;
}_LCD;
extern _LCD xLCD;


/******************************* 定义 ILI934 显示屏常用颜色 ********************************/
#define      WHITE                 0xFFFF       // 白色
#define      BLACK               0x0000       // 黑色 
#define      GREY                0xF7DE       // 灰色 
#define      RED                 0xF800       // 红 
#define      MAGENTA             0xF81F       // 洋红色 
#define      GRED                0xFFE0    // 深红色
#define      GREEN               0x07E0       // 绿 
#define      CYAN                0x7FFF       // 青色 
#define      YELLOW              0xFFE0       // 黄色 
#define      LIGHTGREEN          0X841F    // 浅绿色 
#define      BLUE                0x001F       // 蓝 
#define      GBLUE               0x07FF    // 浅蓝 1
#define      LIGHTBLUE           0X7D7C       // 浅蓝 2
#define      BLUE2               0x051F       // 浅蓝 3
#define      GRAYBLUE            0X5458    // 灰蓝 
#define      DARKBLUE               0X01CF    // 深蓝

#define      LGRAY                  0XC618    // 浅灰色,窗体背景色
#define      LGRAYBLUE           0XA651    // 浅灰蓝色(中间层颜色)
#define      LBBLUE              0X2B12    // 浅棕蓝色(选择条目的反色)
                                                                      
        


/*****************************************************************************
 ** 声明全局函数
 ** 魔女开发板     资料Q群文件：262901124
****************************************************************************/
void LCD_Init (void);                                                          // 初始化
void LCD_DisplayOn(void);                                                      // 开显示
void LCD_DisplayOff(void);                                                     // 关显示
void LCD_DisplayDir(u8 dir);                                                   // 显示方向, 0-正竖屏，3-倒竖屏，5-正横屏, 6-倒横屏
void LCD_DrawPoint(u16 x, u16 y, u16 color);                                   // 画点
void LCD_Circle ( uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint16_t color, uint8_t Filled );
void LCD_Line(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);                      // 画线
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);                          // 填充单色
void LCD_String(u16 x, u16 y, char* pFont, u8 size, u32 fColor, u32 bColor);   // 显示中英文字符串
void LCD_Image(u16 x, u16 y, u16 width, u16 height, const u8 *image) ;         // 显示图像
void LCD_Rectangle ( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t filled );  // 画矩形
void LCD_Image(u16 x, u16 y, u16 width, u16 height, const u8 *image) ;         // 显示图像
void LCD_Cross( uint16_t x, uint16_t y, uint16_t len, uint32_t fColor);        // 画十字线
void LCD_GUI(void);                                                            // 绘制简单界面
void LCD_ShiftLeft(void); 
#endif /* __BSP_ILI9341_ILI9341_H */


