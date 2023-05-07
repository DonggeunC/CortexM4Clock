#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"
#include "lcd.h"

#define LED_LIGHT {GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;}

typedef struct{
  int msec;
  int sec;
  int min;
  int hour;
  char apm;
}TIME;

int t_cnt = 0;
volatile int key=0;
volatile int almFlag = 0;
volatile int almsetFlag = 0;
volatile int setFlag = 0;
volatile int alarming = 0;

TIME time;
TIME alarm;
char almtime[20];
char ctime[20];
char pretime[20];
char almtime[20];
char prealm[20];

void clock_display(){
  sprintf(ctime,"%02d:%02d:%02d  %cM",time.hour,time.min,time.sec,time.apm);
  strcpy(pretime,ctime);
  if(!strcmp(ctime,pretime)){
    lcd(0,0,ctime);
  }
}

void clock_calc(){
  if(t_cnt >= 10){
    t_cnt = 0;
    time.sec++;
  }
  else if(time.sec == 60){
    time.sec = 0;
    time.min++;
  }  
  else if(time.min == 60){
    time.min = 0;
    time.hour++;
  }
  else if(time.hour > 12){
    time.hour =1;
  }
  else if(time.apm == 'A'){
    if(time.hour == 12)
      time.apm = 'P';
  }
  else if(time.apm == 'P'){
    if(time.hour == 12)
      time.apm = 'A';
  }
}

void clock_set(){
  sprintf(almtime,"%02d:%02d:%02d  %cM",alarm.hour,alarm.min,alarm.sec,alarm.apm);
  lcd(0,1,almtime);
  if(key != 0 && setFlag == 1)
  {
    if(key == 1) 
    {
      time.hour++;
    }
    else if(key == 2) 
    {
      time.min++;
    }
    else if(key == 3) 
    {
      time.sec++;  
    }
    else if(key == 4){
      almsetFlag = 0; setFlag = 0; almFlag = 1;
    }
    key = 0;
    sprintf(almtime,"%02d:%02d:%02d  %cM",alarm.hour,alarm.min,alarm.sec,alarm.apm);
    lcd(0,1,almtime);
  }
  else if(key != 0 && almsetFlag == 1)
  {
    if(key == 1) 
    {
      alarm.hour++;
      if(alarm.hour == 12){
        if(alarm.apm == 'A')
          alarm.apm = 'P';
        else
          alarm.apm = 'A';
      }
      else if(alarm.hour >12){
        alarm.hour = 1;
      }
    }
    else if(key == 2) 
    {
      alarm.min++;
      if(alarm.min == 60)
        alarm.min = 0;
      
    }
    else if(key == 3) 
    {
      alarm.sec++;    
      if(alarm.sec == 60)
        alarm.sec = 0;
    }
    else if(key == 4){
      almFlag = 0; almsetFlag = 0; setFlag = 1;
    }
    key = 0;
    sprintf(almtime,"%02d:%02d:%02d  %cM",alarm.hour,alarm.min,alarm.sec,alarm.apm);
    lcd(0,1,almtime);
  }
  if(key == 4){

    if (almFlag ==1 && almsetFlag == 0 && setFlag == 0){
      almsetFlag = 1; almFlag = 0;
    }
    else if(almFlag ==0 && almsetFlag == 0 && setFlag == 0){
      almFlag = 1;
    }
    key = 0;
  }
  if(setFlag == 1||almsetFlag == 1){
    lcd(13,1,"*");
  }
  else
    lcd(13,1," ");
  if(!strcmp(ctime,almtime) && almFlag == 1){
      alarming = 1;
    }
  if(alarming == 1){
    if(alarm.min+1 == time.min && alarm.sec == time.sec){  
      alarming = 0;
      GPIO_WriteBit(GPIOB,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15,Bit_RESET);
    }
    else{
      if(time.sec % 2 == 0)
        GPIO_WriteBit(GPIOB,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15,Bit_SET);
      else if(time.sec % 2 == 1)
          GPIO_WriteBit(GPIOB, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, Bit_RESET);
    }
  }
}

void TIM7_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
  { 
    
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    time.msec++;
    if(time.msec == 100){
      t_cnt++ ; //100ms
      time.msec = 0;
      }
    clock_calc();
  }
}

void EXTI0_IRQHandler(void)
{
 
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
    key = 1;
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}
void EXTI1_IRQHandler(void){
  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
    key = 2;
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}
void EXTI2_IRQHandler(void){
  if(EXTI_GetITStatus(EXTI_Line2) != RESET)
  {
    key = 3;
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}
void EXTI3_IRQHandler(void){
  if(EXTI_GetITStatus(EXTI_Line3) != RESET)
  {
    key = 4;
    EXTI_ClearITPendingBit(EXTI_Line3);
  }  
}

int main()
{
  time.apm = 'A';
  time.hour = 0;
  time.min = 1;
  time.sec = 50;
  time.msec = 0;
  alarm.apm = 'A';
  
  GPIO_InitTypeDef   GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef   NVIC_InitStructure; 
  EXTI_InitTypeDef   EXTI_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_Init(GPIOB, &GPIO_InitStructure);              //LED
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;                                 
  GPIO_Init(GPIOA, &GPIO_InitStructure);        //button  
  
  EXTI_InitStructure.EXTI_Line =  EXTI_Line0 |  EXTI_Line1 | EXTI_Line2 | EXTI_Line3;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource1);   
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource2); 
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource3);  
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; 
  NVIC_Init(&NVIC_InitStructure);
 
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; 
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; 
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 84-1;         //(168Mhz/2)/840 = 0.1MHz(10us)
  TIM_TimeBaseStructure.TIM_Period = 1000-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
  
  TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM7, ENABLE);
  
  LcdPort_Init();
  
  while(1)
  {
    clock_display();
    clock_set(); 
  }
}