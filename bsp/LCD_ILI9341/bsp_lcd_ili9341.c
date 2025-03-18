#include "bsp_lcd_ili9341.h"
#include "font.h"    
#include "bsp_w25qxx.h"
#include "bsp_usart.h"



/*****************************************************************************
 ** 变量声明
 *****************************************************************************/
_LCD xLCD;




/*****************************************************************************
 ** 函数声明
 ****************************************************************************/
static void       wrtieCmd  (uint16_t usCmd);
static void       writeData (uint16_t usData);
static uint16_t   readData  (void);
static void       gpioConfig(void);
static void       fsmcConfig(void);
static void       ili9341RegConfig(void);



// ms延时函数，减少移植时对外部文件依赖；
#if 1
static void delay_ms(u32 ms)
{
    ms=ms*6500;                  
    for(u32 i=0; i<ms; i++);      // 72MHz系统时钟下，多少个空循环约耗时1ms
}
#endif



void LCD_Init ( void )
{
    gpioConfig ();          // 引脚初始化
    fsmcConfig ();          // FMSC初始化
    ili9341RegConfig ();    // ili9341参数配置
}

// 初始化ILI9341的IO引脚
static void gpioConfig ( void )
{
    GPIO_InitTypeDef G;

    /* 使能FSMC对应相应管脚时钟*/
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN ;   // 使能PORTA时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN ;   // 使能PORTB时钟, 魔女开发板上没用到，只是为了方便移植而开的
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN ;   // 使能PORTC时钟, 魔女开发板上没用到，只是为了方便移植而开的
    RCC->APB2ENR |= RCC_APB2ENR_IOPDEN ;   // 使能PORTD时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPEEN ;   // 使能PORTE时钟
            
    /* 配置FSMC相对应的数据线,FSMC-D0~D15 */    
    G.GPIO_Speed = GPIO_Speed_50MHz;
    G.GPIO_Mode =  GPIO_Mode_AF_PP;    
    G.GPIO_Pin = ILI9341_D0_PIN;    GPIO_Init ( ILI9341_D0_PORT, & G );
    G.GPIO_Pin = ILI9341_D1_PIN;    GPIO_Init ( ILI9341_D1_PORT, & G );    
    G.GPIO_Pin = ILI9341_D2_PIN;    GPIO_Init ( ILI9341_D2_PORT, & G );    
    G.GPIO_Pin = ILI9341_D3_PIN;    GPIO_Init ( ILI9341_D3_PORT, & G );    
    G.GPIO_Pin = ILI9341_D4_PIN;    GPIO_Init ( ILI9341_D4_PORT, & G );    
    G.GPIO_Pin = ILI9341_D5_PIN;    GPIO_Init ( ILI9341_D5_PORT, & G );    
    G.GPIO_Pin = ILI9341_D6_PIN;    GPIO_Init ( ILI9341_D6_PORT, & G );    
    G.GPIO_Pin = ILI9341_D7_PIN;    GPIO_Init ( ILI9341_D7_PORT, & G );    
    G.GPIO_Pin = ILI9341_D8_PIN;    GPIO_Init ( ILI9341_D8_PORT, & G );    
    G.GPIO_Pin = ILI9341_D9_PIN;    GPIO_Init ( ILI9341_D9_PORT, & G );    
    G.GPIO_Pin = ILI9341_D10_PIN;    GPIO_Init ( ILI9341_D10_PORT, & G );    
    G.GPIO_Pin = ILI9341_D11_PIN;    GPIO_Init ( ILI9341_D11_PORT, & G );
    G.GPIO_Pin = ILI9341_D12_PIN;    GPIO_Init ( ILI9341_D12_PORT, & G );        
    G.GPIO_Pin = ILI9341_D13_PIN;    GPIO_Init ( ILI9341_D13_PORT, & G );    
    G.GPIO_Pin = ILI9341_D14_PIN;    GPIO_Init ( ILI9341_D14_PORT, & G );    
    G.GPIO_Pin = ILI9341_D15_PIN;    GPIO_Init ( ILI9341_D15_PORT, & G );        
    // 配置FSMC相对应的控制线    
    G.GPIO_Pin = ILI9341_RD_PIN;     GPIO_Init (ILI9341_RD_PORT, & G );           // FSMC_NOE   :LCD-RD
    G.GPIO_Pin = ILI9341_WR_PIN;     GPIO_Init (ILI9341_WR_PORT, & G );         // FSMC_NWE   :LCD-WR
    G.GPIO_Pin = ILI9341_CS_PIN;     GPIO_Init ( ILI9341_CS_PORT, & G );       // FSMC_NE1   :LCD-CS
    G.GPIO_Pin = ILI9341_DC_PIN;     GPIO_Init ( ILI9341_DC_PORT, & G );         // FSMC_A16   :LCD-DC
    // 配置LCD复位RST控制管脚*/
    G.GPIO_Mode= GPIO_Mode_Out_PP;    G.GPIO_Speed = GPIO_Speed_50MHz;    
//    G.GPIO_Pin = ILI9341_RST_PIN;     GPIO_Init ( ILI9341_RST_PORT, & G );        
    /* 配置LCD背光控制管脚BK*/    
    G.GPIO_Pin = ILI9341_BK_PIN;     GPIO_Init ( ILI9341_BK_PORT, & G );
}

//FSMC 模式配置，使用FSMC模拟8080接口
static void fsmcConfig ( void )
{
    #if 0
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  readWriteTiming;     

    //地址建立时间（ADDSET）为1个HCLK 2/72M=28ns
    readWriteTiming.FSMC_AddressSetupTime      = 0x01;     //地址建立时间
    //数据保持时间（DATAST）+ 1个HCLK = 5/72M=70ns    
    readWriteTiming.FSMC_DataSetupTime         = 0x04;     //数据建立时间
    //选择控制的模式
    //模式B,异步NOR FLASH模式，与ILI9341的8080时序匹配
    readWriteTiming.FSMC_AccessMode            = FSMC_AccessMode_B;    
    
    /*以下配置与模式B无关*/
    //地址保持时间（ADDHLD）模式A未用到
    readWriteTiming.FSMC_AddressHoldTime       = 0x00;     //地址保持时间
    //设置总线转换周期，仅用于复用模式的NOR操作
    readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
    //设置时钟分频，仅用于同步类型的存储器
    readWriteTiming.FSMC_CLKDivision           = 0x00;
    //数据保持时间，仅用于同步型的NOR    
    readWriteTiming.FSMC_DataLatency           = 0x00;    
    
    FSMC_NORSRAMInitStructure.FSMC_Bank                  = FSMC_Bank1_NORSRAMx;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType            = FSMC_MemoryType_NOR;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode              = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct     = &readWriteTiming;  
    
    FSMC_NORSRAMInit ( & FSMC_NORSRAMInitStructure );     
    
    /* 使能 FSMC_Bank1_NORSRAM4 */
    FSMC_NORSRAMCmd ( FSMC_Bank1_NORSRAMx, ENABLE );      

    #else
    
    /* 使能FSMC时钟*/
    RCC->AHBENR|=1<<8;              //使能FSMC时钟          
    //寄存器清零
    //bank1有NE1~4,每一个有一个BCR+TCR，所以总共八个寄存器。
    //这里我们使用NE4 ，也就对应BTCR[6],[7]。                    
    FSMC_Bank1->BTCR[0]=0X00000000;
    FSMC_Bank1->BTCR[1]=0X00000000;
    FSMC_Bank1E->BWTR[0]=0X00000000;
    //操作BCR寄存器    使用异步模式
    FSMC_Bank1->BTCR[0]|=1<<12;        //存储器写使能
    FSMC_Bank1->BTCR[0]|=1<<14;        //读写使用不同的时序
    FSMC_Bank1->BTCR[0]|=1<<4;         //存储器数据宽度为16bit         
    //操作BTR寄存器    
    //读时序控制寄存器                                 
    FSMC_Bank1->BTCR[1]|=0<<28;        //模式A                                         
    FSMC_Bank1->BTCR[1]|=1<<0;         //地址建立时间（ADDSET）为2个HCLK 1/36M=27ns(实际>200ns)          
    //因为液晶驱动IC的读数据的时候，速度不能太快，尤其对1289这个IC。
    FSMC_Bank1->BTCR[1]|=0XF<<8;      //数据保存时间为16个HCLK          
    //写时序控制寄存器  
    FSMC_Bank1E->BWTR[0]|=0<<28;     //模式A                                      
    FSMC_Bank1E->BWTR[0]|=0<<0;        //地址建立时间（ADDSET）为1个HCLK 
     //4个HCLK（HCLK=72M）因为液晶驱动IC的写信号脉宽，最少也得50ns。72M/4=24M=55ns       
    FSMC_Bank1E->BWTR[0]|=3<<8;     //数据保存时间为4个HCLK    
    //使能BANK1,区域4
    FSMC_Bank1->BTCR[0]|=1<<0;        //使能BANK1，区域4    
    #endif
}

// 初始化ILI9341寄存器
static void ili9341RegConfig ( void )
{    
    //尝试9341 ID的读取        
    wrtieCmd(0XD3);            // 指令：读ID               
    readData();             // 第1个参数：dummy        
    readData();               // 第2个参数：IC版本号
    xLCD.id=readData();     // 第3个参数：IC名字(93)    
       xLCD.id<<=8;
    xLCD.id|=readData();    // 第4个参数：IC名字(41)    
     printf("显示屏 检测...        %x\r\n",xLCD.id);  // 打印LCD ID  
    if(xLCD.id !=0X9341)    // 9341初始化失败
        return;                // 注意：如果FSMC配置不正确，有可能可显示，但刷新速度慢，且读取不到正确型号
    
    // Power control B (CFh) 
    wrtieCmd ( 0xCF  );
    writeData ( 0x00  );
    writeData ( 0xC1  ); //-81
    writeData ( 0x30  );    
    // Power on sequence control (EDh)
    wrtieCmd ( 0xED );
    writeData ( 0x64 );
    writeData ( 0x03 );
    writeData ( 0x12 );
    writeData ( 0x81 );    
    // Driver timing control A (E8h)
    wrtieCmd ( 0xE8 ); //
    writeData ( 0x85 );
    writeData ( 0x10 );
    writeData ( 0x7A ); //-78    
    // Power control A (CBh) 
    wrtieCmd ( 0xCB );
    writeData ( 0x39 );
    writeData ( 0x2C );
    writeData ( 0x00 );
    writeData ( 0x34 );
    writeData ( 0x02 );    
    // Pump ratio control (F7h)
    wrtieCmd ( 0xF7 );
    writeData ( 0x20 );        
    // Driver timing control B 
    wrtieCmd ( 0xEA );
    writeData ( 0x00 );
    writeData ( 0x00 );            
    // Frame Rate Control (In Normal Mode/Full Colors) (B1h) */
    wrtieCmd ( 0xB1 );
    writeData ( 0x00 );
    writeData ( 0x1A );  //-1B    
    //  Display Function Control (B6h) */
    wrtieCmd ( 0xB6 );
    writeData ( 0x0A );
    writeData ( 0xA2 );    
    // Power Control 1 (C0h) 
    wrtieCmd ( 0xC0 );
    writeData ( 0x1B ); //-35    
    // Power Control 2 (C1h) 
    wrtieCmd ( 0xC1 );
    writeData ( 0x01 ); //-11    
    // VCOM 控制 1 (C5h) 
    wrtieCmd ( 0xC5 );
    writeData ( 0x30 );  // 45
    writeData ( 0x30 );  // 45    
    // VCOM 控制 2 (C7h) 
    wrtieCmd ( 0xC7 );
    writeData ( 0xB7 );  //-A2    
    // 使能3 伽马控制 (F2h) 
    wrtieCmd ( 0xF2 );
    writeData ( 0x00 );    
    // 伽马设置 (26h)
    wrtieCmd ( 0x26 );
    writeData ( 0x01 );    
    // 正极伽马校准 (E0H)
    wrtieCmd ( 0xE0 ); 
    writeData ( 0x0F );
    writeData ( 0x2A ); // 26
    writeData ( 0x28 ); // 24
    writeData ( 0x08 ); //0B
    writeData ( 0x0E );
    writeData ( 0x08 ); // 09
    writeData ( 0x54 );
    writeData ( 0xA9 ); // A8
    writeData ( 0x43 ); // 46
    writeData ( 0x0A ); // 0c
    writeData ( 0x0F ); // 17
    writeData ( 0x00 ); // 00
    writeData ( 0x00 ); // 00
    writeData ( 0x00 ); // 00
    writeData ( 0x00 );     
    // 负极伽马校准 (E1h) 
    wrtieCmd ( 0XE1 );  
    writeData ( 0x00 );
    writeData ( 0x15 ); // 19
    writeData ( 0x17 ); // 1B
    writeData ( 0x07 ); // 04
    writeData ( 0x11 ); // 10
    writeData ( 0x06 ); // 07
    writeData ( 0x2B ); // 2A
    writeData ( 0x56 ); // 47
    writeData ( 0x3C ); // 39
    writeData ( 0x05 ); // 03
    writeData ( 0x10 ); // 06
    writeData ( 0x0F ); // 06
    writeData ( 0x3F ); // 30
    writeData ( 0x3F ); // 38
    writeData ( 0x0F ); //     
    // 列地址设置
    wrtieCmd ( 0x2A );     
    writeData ( 0x00 );
    writeData ( 0x00 );
    writeData ( 0x00 );
    writeData ( 0xEF );    
    // 页地址设置                 
    wrtieCmd ( 0x2B ); 
    writeData ( 0x00 );
    writeData ( 0x00 );
    writeData ( 0x01 );
    writeData ( 0x3F );    
    // 像素格式设置 (3Ah)
    wrtieCmd ( 0x3a );         // 像素格式设置
    writeData ( 0x55 );        // 16位接口，16位数据    
    // 退出睡眠模式 (11h) 
    wrtieCmd ( 0x11 );         // 等待5ms待电路稳定，再执行其它指令
    delay_ms(10);                
    // 打开显示(29h) 
    wrtieCmd ( 0x29 );         // 不会改变帧存储器数据        
    
    LCD_DisplayDir(LCD_DIR);   // 设置显示方向
    ILI9341_BK_PORT->BSRR |= ILI9341_BK_PIN;
    LCD_Fill(0, 0, xLCD.width, xLCD.height, BLACK);

    xLCD.FlagInit =1;    
}

// 向ILI9341写入命令 (表寄存器地址)
// FSMC_Bank1_NORSRAM用于LCD命令操作的地址
static void wrtieCmd (uint16_t cmdTemp)
{
    * (__IO uint16_t *)0x60000000 = cmdTemp;    
}

// 向ILI9341写入数据 (要写入的数据)
// FSMC_Bank1_NORSRAM用于LCD数据操作的地址 
static void writeData (uint16_t dataTemp)
{
    * (__IO uint16_t *)0x60020000 = dataTemp;    
}

// 从ILI9341读取数据
static uint16_t readData (void)
{
    return (*(__IO uint16_t *)0x60020000);    
}

/*****************************************************************
 * 函  数：setCursor
 * 功  能：设置显示区域，在此区域写点数据自动换行
 * 参  数：x      区域坐标起点, 
 *         y      区域坐标起点,
 *         width  区域宽度，     
 *         height 纵坐标线束点,
 * 返回值：无
 
******************************************************************/
void setCursor(u16 x, u16 y, u16 width, u16 height)
{             
    wrtieCmd (0X2A);                 // 发送指令：设置x坐标
    writeData(x>>8);
    writeData(x&0xFF);
    writeData((x+width-1)>>8);
    writeData((x+width-1)&0xFF);     
    
    wrtieCmd (0X2B); 
    writeData(y>>8);
    writeData(y&0xFF);
    writeData((y+height-1)>>8);
    writeData((y+height-1)&0xFF);     

    // 发送写GRAM指令
    wrtieCmd(0X2C);     
}     

/*****************************************************************
 * 函  数：LCD_DrawPoint
 * 功  能：画一个点
 * 参  数：x坐标，y坐标, 16位颜色值
 * 返回值：无
******************************************************************/
void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
    setCursor(x,y, 1, 1);        //设置光标位置 
    writeData(color); 
}    

/*****************************************************************
 * 函  数：LCD_GetPointPixel
 * 功  能：获取某一个坐标点像素的颜色值数据
 * 参  数：x      x坐标 
 *         y      y坐标
 * 返回值：16位像素颜色值
 * 
******************************************************************/
uint16_t LCD_GetPoint ( uint16_t x, uint16_t y )
{ 
    uint16_t r=0, g=0, b=0 ;
    
    setCursor ( x, y, 1, 1 );    
    wrtieCmd ( 0x2E );  // 读GRAN
    
    r = readData ();     // 第1个返回的是无效值 
    r = readData ();  
    b = readData ();  
    g = readData ();         
    
    return ( ( ( r >> 11 ) << 11 ) | ( ( g >> 10 ) << 5 ) | ( b >> 11 ) );
}

/*****************************************************************
 * 函  数：LCD_DisplayDir
 * 功  能：设置LCD显示方向
 * 参  数：scanDir 显示方向(扫描方向)，0-正竖屏，3-倒竖屏，5-正横屏, 6-倒横屏
 *    
 * 返回值：
 * 
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
 * 分  享： 不断更新于Q群文件夹：262901124  最后修改_2020年09月01日
******************************************************************/
void LCD_DisplayDir(u8 scanDir)
{
    u16 regval=0;
    
    if(scanDir==0||scanDir==3)       // 竖屏
    {
        xLCD.dir=0;          
        xLCD.width  = LCD_WIDTH;
        xLCD.height = LCD_HIGH ;
    }
    if(scanDir==5 || scanDir==6)   // 横屏
    {                      
        xLCD.dir=1;          
        xLCD.width  = LCD_HIGH;
        xLCD.height = LCD_WIDTH;
    }         
    
    if(scanDir==0) regval|=(0<<7)|(0<<6)|(0<<5); // 从左到右,从上到下
    if(scanDir==3) regval|=(1<<7)|(1<<6)|(0<<5); // 从右到左,从下到上
    if(scanDir==5) regval|=(0<<7)|(1<<6)|(1<<5); // 从上到下,从右到左
    if(scanDir==6) regval|=(1<<7)|(0<<6)|(1<<5); // 从下到上,从左到右           
    wrtieCmd (0X36);             // 读写方向，颜色模式
    writeData(regval|0x08);          // 
}     

/*****************************************************************
 * 函  数：LCD_DisplayOn
 * 功  能：开启显示，亮屏
 * 参  数：
 *    
 * 返回值：
 * 
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
 * 分  享： 不断更新于Q群文件夹：262901124  最后修改_2020年09月01日
******************************************************************/
void LCD_DisplayOn(void)
{                       
    wrtieCmd(0X29);    //开启显示
    ILI9341_BK_PORT->BSRR |= ILI9341_BK_PIN;
}     

/*****************************************************************
 * 函  数：LCD_DisplayOff
 * 功  能：关闭显示, 熄屏
 * 参  数：
 *    
 * 返回值：
 * 
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
 * 分  享： 不断更新于Q群文件夹：262901124  最后修改_2020年09月01日
******************************************************************/
void LCD_DisplayOff(void)
{       
    wrtieCmd(0X28);    //关闭显示
    ILI9341_BK_PORT->BSRR |= (u32)ILI9341_BK_PIN << 16;
}   

/*****************************************************************
 * 函  数：LCD_Fill
 * 功  能：在区域内填充某一颜色像素点
 * 参  数：x      区域坐标起点, 
 *         y      区域坐标起点,
 *         width  区域宽度，     
 *         height 纵坐标线束点,
 *         color  颜色值
 * 返回值：无
 * 
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
 * 分  享： 不断更新于Q群文件夹：262901124  最后修改_2020年09月01日
******************************************************************/
void LCD_Fill(u16 x, u16 y, u16 width, u16 height, u16 color)
{          
    u32 num = width * height;
    setCursor(x, y, width, height);    // 设置点读写区域 
    while(num--)
        writeData(color);
}  

/*****************************************************************
 * 函  数：LCD_Line
 * 功  能：画线段 (参考野火大神)  
 * 参  数：xStart      起始x坐标 
 *         yStart      起始y坐标
 *         xEnd        结束x坐标
 *         yEnd        结束y坐标
 *         color       颜色
 * 返回值：
 * 
 * 备  注： 魔女开发板团队        淘宝 https://demoboard.taobao.com
 * 分  享： 不断更新于Q群文件夹：262901124  最后修改_2020年09月01日
******************************************************************/
void LCD_Line ( uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd , uint16_t color)
{
    uint16_t us; 
    uint16_t usX_Current, usY_Current;
    
    int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
    int32_t lIncrease_X, lIncrease_Y;         
    
    lDelta_X = xEnd - xStart; //计算坐标增量 
    lDelta_Y = yEnd - yStart; 
    
    usX_Current = xStart; 
    usY_Current = yStart;     
    
    if ( lDelta_X > 0 ) 
        lIncrease_X = 1; //设置单步方向     
    else if ( lDelta_X == 0 ) 
        lIncrease_X = 0;//垂直线     
    else 
    { 
        lIncrease_X = -1;
        lDelta_X = - lDelta_X;
    } 
    
    if ( lDelta_Y > 0 )
        lIncrease_Y = 1;     
    else if ( lDelta_Y == 0 )
        lIncrease_Y = 0;//水平线     
    else 
    {
        lIncrease_Y = -1;
        lDelta_Y = - lDelta_Y;
    } 
    
    if (  lDelta_X > lDelta_Y )
        lDistance = lDelta_X; //选取基本增量坐标轴     
    else 
        lDistance = lDelta_Y; 
    
    for ( us = 0; us <= lDistance + 1; us ++ )//画线输出 
    {  
        LCD_DrawPoint ( usX_Current, usY_Current , color);//画点 
        
        lError_X += lDelta_X ; 
        lError_Y += lDelta_Y ; 
        
        if ( lError_X > lDistance ) 
        { 
            lError_X -= lDistance; 
            usX_Current += lIncrease_X; 
        }  
        
        if ( lError_Y > lDistance ) 
        { 
            lError_Y -= lDistance; 
            usY_Current += lIncrease_Y; 
        }         
    }      
}   

/*****************************************************************
 * 函  数：LCD_Rectangle
 * 功  能：画一个矩形 (参考野火大神)
 * 参  数：x       起始x坐标 
 *         y       起始y坐标
 *         width   矩形的像素宽度
 *         height  矩形的像素高度
 *         color   颜色
 *         filled  是否实体填充
 * 返回值：
 * 
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
 * 分  享： 不断更新于Q群文件夹：262901124  最后修改_2020年09月01日
******************************************************************/
void LCD_Rectangle ( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t filled )
{
    if ( filled )
    {
        LCD_Fill (x, y, width, height, color);
    }
    else
    {
        LCD_Line ( x, y, x + width - 1, y , color);
        LCD_Line ( x, y + height - 1, x + width - 1, y + height - 1 , color);
        LCD_Line ( x, y, x, y + height - 1, color );
        LCD_Line ( x + width - 1, y, x + width - 1, y + height - 1 , color);        
    }
}

/*****************************************************************
 * 函  数：LCD_Circle
 * 功  能：画一个圆形  (参考野火大神)
 * 参  数：x       起始x坐标 
 *         y       起始y坐标
 *         radius  半径
 *         color   颜色
 *         filled  是否实体填充
 * 返回值：
 * 
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
 * 分  享： 不断更新于Q群文件夹：262901124  最后修改_2020年09月01日
******************************************************************/
void LCD_Circle ( uint16_t x, uint16_t y, uint16_t radius, uint16_t color, uint8_t filled )
{
    int16_t sCurrentX, sCurrentY;
    int16_t sError;    
    
    sCurrentX = 0; sCurrentY = radius;        
    sError = 3 - ( radius << 1 );     //判断下个点位置的标志    
    
    while ( sCurrentX <= sCurrentY )
    {
        int16_t sCountY;        
        
        if ( filled )             
            for ( sCountY = sCurrentX; sCountY <= sCurrentY; sCountY ++ ) 
            {                      
                LCD_DrawPoint ( x + sCurrentX, y + sCountY, color );           //1，研究对象 
                LCD_DrawPoint ( x - sCurrentX, y + sCountY , color);           //2       
                LCD_DrawPoint ( x - sCountY,   y + sCurrentX, color );           //3
                LCD_DrawPoint ( x - sCountY,   y - sCurrentX , color);           //4
                LCD_DrawPoint ( x - sCurrentX, y - sCountY , color);           //5    
                LCD_DrawPoint ( x + sCurrentX, y - sCountY , color);           //6
                LCD_DrawPoint ( x + sCountY,   y - sCurrentX , color);           //7     
                LCD_DrawPoint ( x + sCountY,   y + sCurrentX , color);           //0                
            }        
        else
        {          
            LCD_DrawPoint ( x + sCurrentX, y + sCurrentY , color);             //1，研究对象
            LCD_DrawPoint ( x - sCurrentX, y + sCurrentY , color);             //2      
            LCD_DrawPoint ( x - sCurrentY, y + sCurrentX , color);             //3
            LCD_DrawPoint ( x - sCurrentY, y - sCurrentX , color);             //4
            LCD_DrawPoint ( x - sCurrentX, y - sCurrentY , color);             //5       
            LCD_DrawPoint ( x + sCurrentX, y - sCurrentY , color);             //6
            LCD_DrawPoint ( x + sCurrentY, y - sCurrentX , color);             //7 
            LCD_DrawPoint ( x + sCurrentY, y + sCurrentX , color);             //0
    }    
        
        sCurrentX ++;
        
        if ( sError < 0 ) 
            sError += 4 * sCurrentX + 6;            
        else
        {
            sError += 10 + 4 * ( sCurrentX - sCurrentY );   
            sCurrentY --;
        }             
    }    
}


/******************************************************************
 * 函数名： drawAscii
 * 功  能： 在指定位置显示一个字符    
 * 参  数： u16 x,y     起始坐标
 *          u8  num     要显示的字符:" "--->"~"
 *          u8  size    字体大小 12/16/24/32
 *          u32 fColor  字体颜色
 *          u32 bColor  背景颜色
 *****************************************************************/
void drawAscii(u16 x,u16 y,u8 num,u8 size,u32 fColor, u32 bColor)
{              
    //spiInit();                                      // 防止SPI参数被其它设备修改了
     
    u8 temp;
    u16 y0=y;
    
    u8 csize=(size/8+((size%8)?1:0))*(size/2);           // 得到字体一个字符对应点阵集所占的字节数    
     num=num-' ';                                       // 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）
    for(u8 t=0;t<csize;t++)
    {   
        if(size==12)         temp=asc2_1206[num][t];   // 调用1206字体
        else if(size==16)    temp=asc2_1608[num][t];   // 调用1608字体
        else if(size==24)    temp=asc2_2412[num][t];   // 调用2412字体
        else if(size==32)    temp=asc2_3216[num][t];   // 调用3216字体
        else return;                                   // 没有的字库
        
        for(u8 t1=0; t1<8; t1++)
        {                
            if(temp&0x80)   LCD_DrawPoint (x, y, fColor);  // 字体 画点 
            else            LCD_DrawPoint (x, y, bColor);  // 背景 画点
            temp<<=1;
            y++;
            if(y>=xLCD.height)    return;               // 超出屏幕高度(底)
            if((y-y0)==size)
            {
                y=y0;
                x++;
                if(x>=xLCD.width) return;              // 超出屏幕宽度(宽)
                break;
            }
        }       
    }                     
} 

/******************************************************************
 * 函数名： drawGBK
 * 功  能： 在指定位置显示一个字符    
 * 参  数： u16 x,y     起始坐标
 *          u8  num     要显示的字符:" "--->"~"
 *          u8  size    字体大小 12/16/24/32
 *          u32 fColor  字体颜色
 *          u32 bColor  背景颜色
 *****************************************************************/
void drawGBK( u16 x, u16 y, u8* font, u8 size, u32 fColor, u32 bColor)
{    
    u8 temp;
    u16 y0=y;
    u8 GBK[128];   
    u8 csize=(size/8+((size%8)?1:0))*(size);    // 得到字体一个字符对应点阵集所占的字节数     
    W25qxx_ReadGBK(font, size, GBK);                    // 得到相应大小的点阵数据
    
    //spiInit();                                  // 防止SPI参数被其它设备修改了    
    for(u8 t=0; t<csize; t++)
    {                                                      
        temp = GBK[t];                            // 得到GBK点阵数据                          
        for(u8 t1=0; t1<8; t1++)
        {
            if(temp&0x80)   LCD_DrawPoint (x, y, fColor);    
            else            LCD_DrawPoint (x, y, bColor);            
            temp<<=1;
            y++;
            if((y-y0)==size)
            {
                y=y0;
                x++;
                break;
            }
        }       
    }      
}

/******************************************************************************
 * 函  数： LCD_String
 * 功  能： 在LCD上显示字符串(支持英文、汉字)
 * 描  述： 英文：字模数据保存在font.h，编译后和代码一起保存在芯片内部Flash
 *          汉字：字模保存在外部Flash中，本函数字库在W25Q128中
 *                魔女开发板中W25Q128已烧录宋体4种字号大小字模数据
 * 参  数： u16   x      字体左上角X坐标
 *          u16   y      字体左上角y坐标
 *          char* pFont  要显示的字符串数据
 *          u8    size   字号大小：12 16 24 32
 *          u32   fColor 字体颜色
 *          u32   bColor 背景颜色
 ******************************************************************************/  
void LCD_String(u16 x, u16 y, char* pFont, u8 size, u32 fColor, u32 bColor)
{
    if(xLCD .FlagInit ==0 ) return;
    
    u16 xStart = x;    

    if( size!=12 && size!=16 && size!=24 && size!=32 )       // 字体大小控制
        size=24;    
    
    while(*pFont != 0)                // 连续读取字符串数据，直到'\0'时停止
    {    
        if(x>(xLCD.width-size))       // 行位置判断，如果到了行末，就把光标换行
        {
            x=xStart;
            y=y+size;
        }
        if(y>(xLCD.height - size))    // 列位置判断，如果到了列末，就返回，不再输出
            return;        
        
        if(*pFont < 127)              // ASCII字符
        {
            drawAscii (x, y, *pFont, size, fColor, bColor);            
            pFont++;
            x+=size/2;            
        }
        else                          // 汉字显示
        {     
            // 重要: 如果用的不是魔女开发板的字库, 就要修改或注释下面这一行, 这样就不影响ASCII英文字符的输出            
            drawGBK(x, y, (u8*)pFont, size, fColor, bColor);     
            pFont = pFont+2;          // 下一个要显示的数据在内存中的位置              
            x=x+size;                 // 下一个要显示的数据在屏幕上的X位置    
        }
    }      
}



/******************************************************************
 * 函数名： LCD_Cross
 * 功  能： 在指定点上绘制十字线，用于校准触摸屏
 * 参  数： uint16_t x  　   十字线的中心点坐标x
 *          uint16_t y  　   十字线的中心点坐标x
 *          uint16_t len     十字线的像素长度
 *          uint32_t fColor  颜色
 * 返　回： 无
 * 备  注： 
 *****************************************************************/
void LCD_Cross( uint16_t x, uint16_t y, uint16_t len, uint32_t fColor)
{
    uint16_t temp=len/2;                
                
    LCD_Line(x-temp, y, x+temp, y, fColor);   
    LCD_Line(x, y-temp, x, y+temp, fColor);       
}

/*****************************************************************
 * 函  数：LCD_ShiftLeft
 * 功  能：将屏幕内容整体左移1像素，右侧填充黑色
 * 参  数：无
 * 返回值：无
 ?******************************************************************/
void LCD_ShiftLeft(void)
{
    uint16_t color;
    
    // 遍历所有行 (0~239)
    for (uint16_t y = 0; y < xLCD.height; y++) 
    {
        // 将每行数据左移1像素 (x从1~319 → 0~318)
        for (uint16_t x = 0; x < xLCD.width - 1; x++) 
        {
            color = LCD_GetPoint(x + 1, y);  // 读取右侧像素
            LCD_DrawPoint(x, y, color);     // 写入当前位置
        }
        
        // 最右侧列填充黑色
        LCD_DrawPoint(xLCD.width - 1, y, BLACK);
    }
}

/******************************************************************
 * 函数名： LCD_GUI
 * 功  能： 测试板载设备情况的LCD显示方案
 * 参  数：           
 * 返　回： 
 * 备  注： 
 *****************************************************************/
void LCD_GUI(void)
{
    char strTemp[30];
    
    LCD_Fill(0, 0,  xLCD.width,  xLCD.height, BLACK);
    //LCD_Image (0,0, 60, 60, imageLoge);  // 图片显示函数
    LCD_Line(  0, 49,  0,329,GREY);        // 左竖
    LCD_Line(119, 70,119,329,GREY);        // 中竖
    LCD_Line(239, 49,239,329,GREY);        // 右竖
    LCD_Fill(  0,287,239, 33,WHITE);       // 底白
    LCD_Line(  0,303,239,303,BLACK);
    LCD_Line(119,287,119,329,BLACK);
    LCD_Fill(  0, 49,239, 21,WHITE);    
    LCD_Line(119, 49,119, 70,BLACK);       // 中竖
    LCD_Fill(119,168,120,21,WHITE);    
    
    LCD_String( 60,  0,"魔女开发板", 24, WHITE, BLACK);    
    LCD_String(  8, 28,"STM32F103VET6 - 设备检测监控", 16, GREY, BLACK);        
    LCD_String(  6, 52,"板载设备", 16,  BLACK , WHITE);
    LCD_String(125, 52,"NRF无线通信", 16, BLACK , WHITE);
    LCD_String(125,171,"CAN通信" , 16, BLACK , WHITE);    
    LCD_String(  6,290,"内部温度", 12, BLACK , WHITE);     // 内部温度
    LCD_String(  6,306,"启动次数", 12, BLACK , WHITE);     // 启动次数
    LCD_String(125,290,"触摸坐标", 12, BLACK , WHITE);     // 触摸坐标
    LCD_String(125,306,"运行时长", 12, BLACK , WHITE);     // 运行时长
    sprintf(strTemp, "第%d次", xW25Qxx .StartupTimes);
    LCD_String( 68,306,strTemp,12,BLUE,WHITE);
    
    u16 y=74;
    // UASRT1
    LCD_String( 6, y, "UART1配置",  12, YELLOW ,BLACK);  
    if(xUSART.USART1InitFlag ==1)  { LCD_String(90,y,"完成", 12, GREEN ,BLACK); }   else    { LCD_String(90,y,"失败", 12, RED ,BLACK); }  y=y+15;
    // SystemClock
    LCD_String( 6, y, "系统时钟",   12, YELLOW ,BLACK);   
    sprintf(strTemp,"%d",SystemCoreClock/1000000);
    LCD_String(84, y,strTemp,       12, GREEN ,BLACK);  
    LCD_String(96, y, "MHz", 12, GREEN ,BLACK);   y=y+15;
    // LED指示灯
    LCD_String( 6, y, "LED指示灯",  12, YELLOW ,BLACK);   
    LCD_String(90,y,"完成", 12, GREEN ,BLACK);    y=y+15;
    // 按键中断
    LCD_String( 6, y, "按键中断",   12, YELLOW ,BLACK);   
    LCD_String(90,y,"完成", 12, GREEN ,BLACK);    y=y+15;
    // FLASH存储器
    LCD_String( 6, y, "FLASH存储",  12, YELLOW ,BLACK);   
    if(xW25Qxx.FlagInit  ==1)  { LCD_String(71,y,xW25Qxx.type, 12, GREEN ,BLACK); }   else    { LCD_String(90,y,"失败", 12, RED ,BLACK); }  y=y+15;
    // 汉字字库
    LCD_String( 6, y, "汉字字库",   12, YELLOW ,BLACK);  
    if(xW25Qxx .FlagGBKStorage ==1)  { LCD_String(90,y,"正常", 12, GREEN ,BLACK); }   else    { LCD_String(90,y,"失败", 12, RED ,BLACK); }  y=y+15;
    // 显示屏
    LCD_String( 6, y, "显示屏芯片", 12, YELLOW ,BLACK);  
    sprintf(strTemp,"%X",xLCD.id);        
    if(xLCD.FlagInit  ==1)  { LCD_String(90,y,strTemp, 12, GREEN ,BLACK); }   else    { LCD_String(90,y,"失败", 12, RED ,BLACK); }  y=y+15;
           
}


