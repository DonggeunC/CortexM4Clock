#include "stm32f4xx_gpio.h"
#include "lcd.h"

// delay 함수
static void Delay(const uint32_t Count)
{
  __IO uint32_t index = 0; 
  for(index = (16800 * Count); index != 0; index--);
}

static void Delay_us(const uint32_t Count)
{
  __IO uint32_t index = 0; 
  for(index = (16 * Count); index != 0; index--);
}


void LcdPort_Init(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                             GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
  GPIO_Init(GPIOC, &GPIO_InitStructure);  //Char LCD, Char RS, R/W, E
  
  Init_LCD();
}

// LCD 에 모든 명령을 전송한후 반드시 Enable Pulse를 주어야 한다.
void E_Pulse(void)
{
  GPIO_SetBits(LCDDATAPORT, TLCD_E);
  Delay(5);
  GPIO_ResetBits(LCDDATAPORT, TLCD_E);
}
  
// LCD에 데이터를 전송하는 함수이다   
void TLCD_LCDDATAPORT(unsigned char data)
{
  GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) | data);
  TLCD_EN;    // 이 부분에서는  E_Pulse() 함수 대신 사용한다.
} 

void Func_Set(void)
{
  GPIO_ResetBits(LCDDATAPORT, TLCD_RW);        // RW = 0, RS = 0 일 때 IR쓰기 명령
  GPIO_ResetBits(LCDDATAPORT, TLCD_RS);
  GPIO_Write( LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) |0x38);               // 초기화를 위한 Function Set
  E_Pulse();                            // Enable Pulse
}

// LCD 초기화 함수
void Init_LCD(void)
{
  GPIO_ResetBits(LCDDATAPORT, TLCD_E);
  Delay(15);                    // 약 15ms 시간 지연
  Func_Set();
  Delay(10);                    // 약 10ms 시간 지연
  Func_Set();
  Delay_us(150);                // 약 150us 시간 지연
  Func_Set();
  GPIO_Write(LCDDATAPORT,(GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) | 0x0f);       // 디스플레이 ON, 커서 ON 
  E_Pulse();
  GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) | 0x06);       // 글자를 오른쪽으로 시프트 시키며 출력 
  E_Pulse();
  GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) | 0x01);       // 화면 클리어
  E_Pulse();
}
  
  /******************************************
     lcd_char() : 1 문자를 디스플레이 하는 함수
  *******************************************/
void lcd_char(char s)
{
  GPIO_SetBits(LCDDATAPORT, TLCD_RS);
  GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) | s);
  E_Pulse();
}

void lcd_disp(char x, char y)
{
  GPIO_ResetBits(LCDDATAPORT, TLCD_RS);
  GPIO_ResetBits(LCDDATAPORT, TLCD_RW);
  if(y==0) GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) | x+0x80);
  else if(y==1) GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) |x+0xc0);
  E_Pulse();
 }
 
void move_disp(char p )
{
  GPIO_ResetBits(LCDDATAPORT, TLCD_RS);
  GPIO_ResetBits(LCDDATAPORT, TLCD_RW);

  if(p==LEFT ) GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) |0x18);
  else if(p==RIGHT) GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) | 0x1c);

  E_Pulse();
}

void disp_ON_OFF(char d, char c, char b)
{
  char disp = 0x08;

  GPIO_ResetBits(LCDDATAPORT, TLCD_RS);
  GPIO_ResetBits(LCDDATAPORT, TLCD_RW);

  if (d==ON) d = 0x04;
  else         d = 0x00;

  if(c == ON) c = 0x02;
  else          c = 0x00;

  if(b == ON) b = 0x01;
  else          b = 0x00;

  GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) | disp | d | c| b);
  E_Pulse();
            
}   
 
void clrscr(void)
{
  GPIO_ResetBits(LCDDATAPORT, TLCD_RS);
  GPIO_ResetBits(LCDDATAPORT, TLCD_RW);
  GPIO_Write(LCDDATAPORT, (GPIO_ReadInputData(LCDDATAPORT) & 0xFF00) | 0x01);
  E_Pulse();

  Delay(10);
} 


 /***************************************************************
    LCD 에 위치를 지정하여 문자열을 출력할 수 있도록 만든 함수이다 
 ****************************************************************/
void lcd(char x, char y, char *str)
{
  lcd_disp(x, y);
  while(*str) lcd_char(*str++);
}
