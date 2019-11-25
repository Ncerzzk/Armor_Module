/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "icm20600.h"
#include "sk6812.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
int16_t ac[3],gy[3];
int16_t last_ac[3];
int heart_beat_cnt=0;

static uint32_t  colors[4]={0,0,0,0};   // 默认为不发光
void set_color_white(uint32_t * colors,int num);
void set_team_color(uint32_t * colors,int num,uint8_t team);


// 以下为可以通过CAN总线配置的参数
uint8_t TEAM=0;  //队伍  不同队伍显示不一样的颜色    
uint8_t NO_ATTACK_TIME =4;      // n个时间单位的死区时间
int16_t ATTACK_VALUE=8192; //攻击判定阈值
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_CAN_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  MPU9250_Init(&MPU9250);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);

  //set_team_color(colors,4,0x0000FF);
  Can_Config();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */
    int32_t sub=0;
    static int8_t attack_cnt=0;    // 被攻击判定计数，在N*5ms内不再感知



    MPU_Read6500(&MPU9250,ac,gy);

    
    for(int i=0;i<3&&attack_cnt==0;++i){
      sub=abs(ac[i]-last_ac[i]);
      if(sub>ATTACK_VALUE){
        attack_cnt=NO_ATTACK_TIME;
        //发CAN消息告诉被攻击
        //我是个傻逼
        CAN_Send_Message(JUDGE_ID,(uint8_t *)&sub,4);    // 击中判定数据包，长度为4
        set_color_white(colors,4);    // 闪光效果。傻逼操作，亮瞎眼
        break;
      }
    }
    send_wave(ac[0],ac[1],ac[2],attack_cnt*1000);                
    memcpy(last_ac,ac,sizeof(uint16_t)*3);
    
    
    attack_cnt--;
    if(attack_cnt<0){
      attack_cnt=0;
      set_team_color(colors,4,TEAM);
    }
    
    
    /*
    heart_beat_cnt++;
    if(heart_beat_cnt>200){   // 超过1s没收到心跳包
      heart_beat_cnt=0;
      TEAM=0;
      //CAN_Send_Message(JUDGE_ID,(uint8_t *)&sub,1);    // 心跳包，长度为1
    }*/
    HAL_Delay(5);
    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// 本函数通过Can总线调用进行队伍的配置
void equip_module_init(uint8_t * data,int len){   
  if(len!=4){
    return ;
  }
  
  TEAM=data[0];
  NO_ATTACK_TIME=data[1];
  memcpy(&ATTACK_VALUE,data+2,2);
  set_team_color(colors,4,TEAM);
  //heart_beat_cnt=0; // 收到心跳包，清空
}


void set_color_white(uint32_t * colors,int num){
  for(int i=0;i<num;++i){
    colors[i]=0xFFFFFF;
  }
  set_leds_color(colors,num);
}

void set_team_color(uint32_t * colors,int num,uint8_t team){
  uint32_t team_color=0x00;
  switch(team){
  case 1:
    team_color=0x0000FF;
    break;
  case 2:
    team_color=0x00FF00;
    break;
  case 3:
    team_color=0xFF0000;
    break;
  default:
    team_color=0x010101;
    break;
  }
  for(int i=0;i<num;++i){
    colors[i]=team_color;
  }
  set_leds_color(colors,num);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
