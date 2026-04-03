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
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
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
CLCD_I2C_Name LCD1;
char lcd_buffer[32]; // Tăng lên 32 để in thoải mái không bao giờ tràn
uint32_t last_lcd_update = 0;

// Biến điều khiển
volatile uint8_t is_running = 0; 
volatile uint8_t direction = 0; 
volatile uint8_t pending_reverse = 0;
uint32_t reverse_start_time = 0;

// Biến Encoder & ADC
uint32_t adc_val = 0;
float setpoint_rpm = 0, current_rpm = 0;
uint16_t last_encoder_cnt = 0;
#define PPR 1928.0f // 482 * 4 (Chế độ đếm X4 của TIM3)

// Biến PID & Feed-Forward
// Tăng mạnh Ki để kéo sai số nhanh hơn khi gần đạt đỉnh
// Sửa lại bộ thông số mềm mại hơn
float Kp = 3.5;   // Tỷ lệ 1:1, sai số 100 RPM thì bù 150 PWM
float Ki = 8.0;   // Giảm cực mạnh Ki để cộng dồn từ từ, chống vọt lố
float Kd = 0.0;   // Tắt Kd để chống nhiễu rung giật
float error = 0, last_error = 0, integral = 0, derivative = 0;
float output_pid = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void I2C_Silent_Recover(void);
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
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
 /* USER CODE BEGIN 2 */
  // Khởi tạo LCD chế độ 4-bit với các chân đã chọn
  // 1. Khởi động các bộ phận ngoại vi
  HAL_ADC_Start(&hadc1);
  
  // PWM trên chân PA1 (CH2) và PA2 (CH3) theo image_286483.png
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); 
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  
  // Ngắt Timer 1 chu kỳ 10ms để tính PID
	// Ngắt Timer 1 chu kỳ 10ms để tính PID
  HAL_TIM_Base_Start_IT(&htim1);
  
  // BẬT BỘ ĐẾM ENCODER PHẦN CỨNG (TIMER 3)
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL); 
  
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET); // Ép SD về 0 ngay khi khởi động
	// 2. Khởi tạo LCD
CLCD_I2C_Init(&LCD1, &hi2c1, 0x4e, 16, 2);
  CLCD_I2C_Clear(&LCD1);
  CLCD_I2C_WriteString(&LCD1, "SYSTEM READY...");
  HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    // 1. Logic chờ đảo chiều
    if (pending_reverse && (HAL_GetTick() - reverse_start_time > 800)) {
        direction = !direction;
        integral = 0; last_error = 0; 
        is_running = 1;
        pending_reverse = 0;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); 
    }

    // 2. ĐỌC VÀ LỌC NHIỄU BIẾN TRỞ (OVERSAMPLING + HYSTERESIS)
    uint32_t adc_sum = 0;
    
    // Đọc liên tiếp 16 lần cực nhanh
    for(int i = 0; i < 16; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK) {
            adc_sum += HAL_ADC_GetValue(&hadc1);
        }
    }
    uint32_t adc_avg = adc_sum / 16; // Tính trung bình cộng để triệt tiêu nhiễu gai

    // Tính Setpoint nháp (Ép max 1000 RPM)
    float temp_setpoint = (float)(adc_avg * 1000.0f) / 4095.0f;

    // KHÓA CỨNG SETPOINT (Chống nhảy liti)
    // Chỉ cập nhật tốc độ nếu tay bạn thực sự vặn lệch quá 5 RPM
    float diff_sp = temp_setpoint - setpoint_rpm;
    if (diff_sp > 5.0f || diff_sp < -5.0f) {
        setpoint_rpm = temp_setpoint;
    }
  // 3. CẬP NHẬT LCD (Mỗi 1000ms - BẢN CHỐT CUỐI CÙNG)
    if (HAL_GetTick() - last_lcd_update > 1000) {
        last_lcd_update = HAL_GetTick(); // Khóa mốc thời gian ngay lập tức

        // Kiểm tra xem mạch I2C có đang bị kẹt do nhiễu không (Timeout cực ngắn 10ms)
        if (HAL_I2C_IsDeviceReady(&hi2c1, 0x4E, 3, 10) != HAL_OK) {
            I2C_Silent_Recover();    // Lắc 9 nhịp clock để thông đường truyền
            CLCD_I2C_Clear(&LCD1);   // Xóa sạch rác trên màn hình do nhiễu gây ra
        }

        // DÒNG 1: Hiển thị Tốc độ cài đặt (S) và Tốc độ thực tế (R)
        CLCD_I2C_SetCursor(&LCD1, 0, 0);
        sprintf(lcd_buffer, "S:%-4.0f R:%-4.0f ", setpoint_rpm, current_rpm);
        CLCD_I2C_WriteString(&LCD1, lcd_buffer);
        
        // DÒNG 2: Hiển thị Lực đẩy PWM (P) và Trạng thái (ST)
        CLCD_I2C_SetCursor(&LCD1, 0, 1);
        if (pending_reverse) {
            CLCD_I2C_WriteString(&LCD1, "STATUS: WAIT... ");
        } else {
            sprintf(lcd_buffer, "P:%-3.0f  ST:%s  ", 
                    output_pid, 
                    (is_running ? "RUN " : "STOP"));
            CLCD_I2C_WriteString(&LCD1, lcd_buffer);
        }
    }
}
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
/* USER CODE BEGIN 4 */
// Hàm này sẽ tự động được gọi mỗi 10ms do Timer 1 tạo ra
/* USER CODE BEGIN 4 */
/* --- HÀM HỒI SINH I2C --- */
	void I2C_Silent_Recover(void) {
    if (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY) != RESET || hi2c1.State != HAL_I2C_STATE_READY) {
        HAL_I2C_DeInit(&hi2c1);
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; 
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        for (int i = 0; i < 9; i++) {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
            HAL_Delay(1);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
            HAL_Delay(1);
        }
        MX_I2C1_Init();
    }
}

/* --- NGẮT TIMER 1: TÍNH PID & FEED-FORWARD --- */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) // Bắt đầu ngắt Timer 1 (Chu kỳ 10ms)
  {
    static uint8_t counter = 0;
    
    // =========================================================
    // KHỐI 1: TÍNH TOÁN RPM VÀ THUẬT TOÁN PID (Mỗi 50ms)
    // =========================================================
    if (++counter >= 5) 
    { 
        counter = 0;
        
        // A. Đọc Encoder phần cứng (Timer 3)
        uint16_t current_cnt = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
        int16_t diff = (int16_t)(current_cnt - last_encoder_cnt);
        last_encoder_cnt = current_cnt;
        
        // B. Tính RPM thực tế
			float current_rpm_raw = (float)(diff * 1200.0f) / PPR;
        current_rpm_raw = (float)(diff * 1200.0f) / PPR;
        if (current_rpm_raw < 0) current_rpm_raw = -current_rpm_raw;
        
        // Lọc thông thấp để số hiển thị và PID chạy êm hơn
        current_rpm = (current_rpm * 0.8f) + (current_rpm_raw * 0.2f);

        // C. TÍNH TOÁN PID (Chỉ tính khi đang RUN)
        if (is_running && !pending_reverse) 
        {
            error = setpoint_rpm - current_rpm;
            
            // Khâu I: Cộng dồn sai số theo thời gian (dt = 50ms = 0.05s)
            integral += error * 0.05f; 
            
            // Chống bão hòa tích phân (Anti-windup) - Nới lỏng cho tốc độ cao
            if (integral > 1000.0f) integral = 1000.0f;
            if (integral < -1000.0f) integral = -1000.0f;
            
            // Khâu D: Tốc độ thay đổi của sai số
            derivative = (error - last_error) / 0.05f;
            
            // Tổng hợp PID
            output_pid = (Kp * error) + (Ki * integral) + (Kd * derivative);
            last_error = error;

            // Giới hạn PWM băm ra chân vi điều khiển (0 - 999)
            if (output_pid > 999.0f) output_pid = 999.0f;
            if (output_pid < 0.0f) output_pid = 0.0f;
        }
    } // Kết thúc khối 50ms

    // =========================================================
    // KHỐI 2: ĐIỀU KHIỂN MOTOR (Chạy mỗi 10ms để đáp ứng siêu nhanh)
    // =========================================================
    if (is_running && !pending_reverse) 
    {
        // Xuất PWM sang Driver L298/Mạch Cầu H
        if (direction == 0) {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, (uint32_t)output_pid);
        } else {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, (uint32_t)output_pid);
        }
    }
    else // Trường hợp STOP hoặc đang đợi đảo chiều
    {
        // Ngắt xung PWM lập tức
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
        
        // RESET THÔNG SỐ PID (Rất quan trọng để lần chạy sau không bị vọt lố)
        integral = 0; 
        last_error = 0; 
        output_pid = 0;
    }
  } // Kết thúc ngắt Timer 1
}
		
	

/* --- NGẮT NÚT NHẤN EXTI --- */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t now = HAL_GetTick();
    
    // --- NÚT PA4: CHẠY / DỪNG ---
    if (GPIO_Pin == GPIO_PIN_4) {
        // BẮT BUỘC: Kiểm tra nút có thực sự đang bị đè xuống mức LOW không
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) {
            static uint32_t last_t4 = 0;
            if (now - last_t4 > 400) { 
                if (pending_reverse) { 
                    pending_reverse = 0;
                    is_running = 0;
                } else {
                    is_running = !is_running; 
                }
                
                if (is_running) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
                else HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
                
                last_t4 = now;
            }
        }
    }
    
    // --- NÚT PA5: ĐẢO CHIỀU ---
    else if (GPIO_Pin == GPIO_PIN_5) {
        // BẮT BUỘC: Kiểm tra nút có thực sự đang bị đè xuống mức LOW không
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) {
            static uint32_t last_t5 = 0;
            if (now - last_t5 > 400) { 
                if (is_running) {
                    is_running = 0;
                    pending_reverse = 1;
                    reverse_start_time = now;
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET); 
                } else {
                    direction = !direction;
                }
                last_t5 = now;
            }
        }
    }
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
