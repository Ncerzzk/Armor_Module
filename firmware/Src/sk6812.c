#include "sk6812.h"
#include "stm32f1xx_hal.h"
#include "stdlib.h"
#include "tim.h"
#include "string.h"

#define Peroid          90
#define One_Duty        (uint8_t)(0.48f*Peroid)
#define Zero_Duty       (uint8_t)(0.24f*Peroid)

// 24bit 颜色 高8位：绿色 中8位 红色  第八位蓝色

uint16_t * RGB2PWM_duty_array(uint32_t color){
  static uint16_t duty_array[24]={0};
  int j=0;
  for(int i=23;i>=0;--i){
    if(color&(1<<i)){
      duty_array[j]=One_Duty;
    }else{
      duty_array[j]=Zero_Duty;
    }
    j++;
  }
  return duty_array;
}

uint16_t *color_all;
void set_leds_color(uint32_t * colors,uint8_t led_cnt){
  uint16_t *duty_array;
  color_all=(uint16_t *)malloc((led_cnt*24+1)*sizeof(uint16_t));
  memset(color_all,0,(led_cnt*24+1)*sizeof(uint16_t));
  for(int i=0;i<led_cnt;++i){
    duty_array=RGB2PWM_duty_array(colors[i]);
    memcpy(color_all+i*24,duty_array,24*sizeof(uint16_t));
  }
  HAL_TIM_OC_Start_DMA(&htim2,TIM_CHANNEL_1,(uint32_t *)color_all,led_cnt*24+1);
  //HAL_TIM_OC_Start_DMA(&htim2,TIM_CHANNEL_4,(uint32_t *)color_all,led_cnt*24+1); 
  
}


void leds_color_left_shift(uint32_t * colors,uint8_t led_cnt){
  uint32_t temp=colors[0];
  for(int i=0;i<led_cnt-1;++i){
    colors[i]=colors[i+1];
  }
  colors[led_cnt-1]=temp;
}


void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
  free(color_all);
  HAL_TIM_OC_Stop_DMA(&htim2,TIM_CHANNEL_1);
  //HAL_TIM_OC_Stop_DMA(&htim2,TIM_CHANNEL_4);
  /*
  uint16_t * color;
  if(now_led_i<led_cnt_max){
    color=set_color(color_array[now_led_i]);
    now_led_i++;
    HAL_TIM_OC_Start_DMA(&htim2,TIM_CHANNEL_3,(uint32_t *)color,25);
    HAL_TIM_OC_Start_DMA(&htim2,TIM_CHANNEL_4,(uint32_t *)color,25);
    
  }else if(now_led_i==led_cnt_max){
    color=set_color(color_array[now_led_i]);
    memset(color,0,400);
    now_led_i++;
    HAL_TIM_OC_Start_DMA(&htim2,TIM_CHANNEL_3,(uint32_t *)color,84);
    HAL_TIM_OC_Start_DMA(&htim2,TIM_CHANNEL_4,(uint32_t *)color,84);
    
  }else{
    ;
  }*/
  
  
}