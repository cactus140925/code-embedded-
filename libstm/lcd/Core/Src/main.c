/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
#include "String.h"
#include "stdio.h"
#include "CLCD_I2C.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE BEGIN 0 */
/* USER CODE BEGIN 0 */
extern TIM_HandleTypeDef htim1;
extern I2C_HandleTypeDef hi2c1;

#define DHT_PORT GPIOB
#define DHT_PIN  GPIO_PIN_0
#define DHT_READ HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)

uint8_t Temp1, Temp2, RH1, RH2, SUM;
uint16_t Temp_16, RH_16; 


void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim1, 0); 
    while (__HAL_TIM_GET_COUNTER(&htim1) < us); 
}

void DHT_SetPinOut(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

void DHT_SetPinIn(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; 
    HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}


uint8_t DHT_Start(void) {
    DHT_SetPinOut();
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
    HAL_Delay(20); 
    
    DHT_SetPinIn(); 
    delay_us(40);  
    
    if (!DHT_READ) {
        delay_us(80); 
        if (DHT_READ) {
            while (DHT_READ); 
            return 1;
        }
    }
    return 0;
}

uint8_t DHT_Read(void) {
    uint8_t Value = 0;
    for(int i = 0; i < 8; i++) {
        while(!DHT_READ); 
        
        delay_us(40); 
        
        if(!DHT_READ) {
            Value &= ~(1 << (7 - i)); // Xuống 0 sớm -> Bit 0
        } else {
            Value |= (1 << (7 - i));  // Vẫn là 1 -> Bit 1
            while(DHT_READ);          
        }
    }
    return Value;
}

uint8_t DHT_ReadTempHum(void) {
    if (DHT_Start()) {
        RH1 = DHT_Read();
        RH2 = DHT_Read();
        Temp1 = DHT_Read();
        Temp2 = DHT_Read();
        SUM = DHT_Read();
        
        if ((uint8_t)(RH1 + RH2 + Temp1 + Temp2) == SUM && SUM != 0) {
        
            RH_16 = (RH1 << 8) | RH2;
            Temp_16 = (Temp1 << 8) | Temp2;
            return 1;
        }
    }
    return 0;
}

CLCD_I2C_Name LCD1;
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
  MX_TIM1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
HAL_TIM_Base_Start(&htim1);
  
  // Khởi tạo LCD I2C (Địa chỉ thường là 0x4E hoặc 0x27, ở đây dùng 16 cột 2 hàng)
  CLCD_I2C_Init(&LCD1, &hi2c1, 0x4E, 16, 2);

  CLCD_I2C_SetCursor(&LCD1, 0, 0);
  CLCD_I2C_WriteString(&LCD1, "KHOI DONG...");
  HAL_Delay(2000); // Chờ hệ thống và cảm biến ổn định
  CLCD_I2C_Clear(&LCD1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
while (1)
  {
      if (DHT_ReadTempHum() == 1) {
          char str_temp[16];
          char str_humi[16];
        
          sprintf(str_temp, "Nhiet: %d.%d C  ", Temp_16 / 10, Temp_16 % 10);
          sprintf(str_humi, "Do am: %d.%d %%  ", RH_16 / 10, RH_16 % 10);
          
          CLCD_I2C_SetCursor(&LCD1, 0, 0);
          CLCD_I2C_WriteString(&LCD1, str_temp);
          CLCD_I2C_SetCursor(&LCD1, 0, 1);
          CLCD_I2C_WriteString(&LCD1, str_humi);
      } else {
          CLCD_I2C_SetCursor(&LCD1, 0, 0);
          CLCD_I2C_WriteString(&LCD1, "Loi doc DHT!    ");
          CLCD_I2C_SetCursor(&LCD1, 0, 1);
          CLCD_I2C_WriteString(&LCD1, "Kiem tra day... ");
      }
      
      HAL_Delay(2000); 
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
