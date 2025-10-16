/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_MAX_VALUE       4095
#define MIN_PERIOD_MS       250  // Fastest blink
#define MAX_PERIOD_MS		1000 // Slowest blink
#define DUTY_CYCLE_PERCENT  10
#define ADC_CHECK_INTERVAL  3000 // Check ADC every in milliseconds
#define LPTIM_ARR_VALUE 	554 // Define the ARR value
#define LPTIM_TICK_MS       15   // The time step for the software counter
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

LPTIM_HandleTypeDef hlptim1;

/* USER CODE BEGIN PV */

typedef enum {
    MODE_ACTIVE,
    MODE_STANDBY_HALT
} System_State_t;

volatile System_State_t System_Mode = MODE_ACTIVE;

volatile  uint16_t ADC_Value;

volatile uint32_t led_period_ms = MIN_PERIOD_MS;
volatile uint8_t  led_on = 0;

volatile uint8_t Enter_Sleep_Flag = 0;
//volatile uint8_t System_Halt_Flag = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_LPTIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief Calculates a delay time based on the ADC value for variable blinking.
 */
static uint32_t Calculate_Period_ms(uint16_t adc_value)
{
    // Linearly map ADC → 250 .. 1000 ms
    return MIN_PERIOD_MS + (adc_value * (MAX_PERIOD_MS - MIN_PERIOD_MS)) / ADC_MAX_VALUE;
}

/**
 * @brief Performs ADC conversion using Polling and saves the result.
 */
static void Start_ADC_Sequence_Polling(void)
{
    // 1. Power ON ADC Sensor (PA7)
    HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_SET);

    // 2. ADC Polling Loop
    HAL_ADC_Start(&hadc);
    if (HAL_ADC_PollForConversion(&hadc, 100) == HAL_OK)
    {
        ADC_Value = HAL_ADC_GetValue(&hadc);
    }
    HAL_ADC_Stop(&hadc);

    // 3. Power OFF ADC (saves power, though not yet optimized for low power mode)
    HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_RESET);

}

/**
 * @brief Prepares peripherals and enters Standby Mode.
 */
static void Enter_Standby_Mode(void)
{
    // 1. Stop Peripherals
    HAL_LPTIM_Counter_Stop_IT(&hlptim1);
    HAL_ADC_DeInit(&hadc);

    // 2. Disable EXTI for WKUP1 pin (PA0)
    HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);

    // 3. Clear Wakeup Flags (Mandatory)
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // Clear WUP flag
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); // Clear Standby flag

    // 4. Enable Wakeup pin 1 (PA0 → rising edge)
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1); // Enable WUP1 (e.g., connected to PA0)

    // 5. Enter Standby Mode (MCU performs a full reset upon wake)
    HAL_PWR_EnterSTANDBYMode();

    // The MCU will not return from this call; it resets when it wakes up.
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	__enable_irq();

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
  MX_ADC_Init();
  MX_LPTIM1_Init();
  /* USER CODE BEGIN 2 */

  if (HAL_LPTIM_Counter_Start_IT(&hlptim1, LPTIM_ARR_VALUE) != HAL_OK) // ADD
  {
      Error_Handler();
  }

  // *** Standby Wakeup Check (CRITICAL) ***
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
      // MCU woke up from Standby (full reset occurred)
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

      // Clear any pending EXTI lines that might have caused the reset
      // (Note: The specific WUP line must be handled depending on connection)
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

      // VITAL: Debounce/Hold time after wake-up
      HAL_Delay(100); // Wait 100ms for button to settle

      // Check if the pin is still HIGH (indicating a sustained press or a faulty release)
      if (HAL_GPIO_ReadPin(WKUP1_GPIO_Port, WKUP1_Pin) == GPIO_PIN_SET)
      {
          // Button is still held down, or bouncing.
          // Option A (Safest): Stay in an idle loop or re-enter Standby after a short time
          // Option B (Active mode): Ignore the button press for a grace period

          // For simple fix: Wait for the button to be released, or a maximum grace period
          uint32_t timeout = HAL_GetTick() + 2000; // 2 second grace period
          while (HAL_GPIO_ReadPin(WKUP1_GPIO_Port, WKUP1_Pin) == GPIO_PIN_SET && HAL_GetTick() < timeout)
          {
              // Do nothing, just wait for release
          }
      }

      // Check if the wakeup was intentional (e.g., from a button press)
      // Here, we assume any wakeup from Standby means we enter ACTIVE mode.
      System_Mode = MODE_ACTIVE;


  }

  // Ensure initial state based on current mode
  if (System_Mode == MODE_ACTIVE)
  {
      // Start peripherals required for ACTIVE mode
      Start_ADC_Sequence_Polling(); // Initial ADC read
      led_period_ms = Calculate_Period_ms(ADC_Value);
      HAL_LPTIM_Counter_Start_IT(&hlptim1, LPTIM_ARR_VALUE);
  } else {
      // If not active (should not happen after reset, but safety measure)
      HAL_LPTIM_Counter_Stop_IT(&hlptim1);
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  }



  // Ensure control pins start low
  HAL_GPIO_WritePin(Discon_GPIO_Port, Discon_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_EN_GPIO_Port, ADC_EN_Pin, GPIO_PIN_RESET);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	if (System_Mode == MODE_STANDBY_HALT)
	{
		// **BUTTON-INITIATED STANDBY (Deepest Sleep)**
		Enter_Standby_Mode();
		// NOTE: The code will never reach here if Standby succeeds; it resets instead.
	}
	else if (Enter_Sleep_Flag)
	{
		// **LED-TIMING RELATED SLEEP (Short duration in Stop Mode)**
		// LPTIM is running, allowing the LPTIM to wake the MCU.
		HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
		SystemClock_Config(); // Restore fast clock upon LPTIM wakeup
		Enter_Sleep_Flag = 0; // reset flag
	}
    /* USER CODE END WHILE */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
  PeriphClkInit.LptimClockSelection = RCC_LPTIM1CLKSOURCE_LSI;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.OversamplingMode = DISABLE;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerFrequencyMode = ENABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief LPTIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPTIM1_Init(void)
{

  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM1_Init 2 */
  __HAL_LPTIM_ENABLE_IT(&hlptim1, LPTIM_IT_ARRM);
  HAL_NVIC_SetPriority(LPTIM1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
  /* USER CODE END LPTIM1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_Pin|Discon_Pin|ADC_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_Pin ADC_EN_Pin */
  GPIO_InitStruct.Pin = LED_Pin|ADC_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : Discon_Pin */
  GPIO_InitStruct.Pin = Discon_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Discon_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : WKUP1_Pin */
  GPIO_InitStruct.Pin = WKUP1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(WKUP1_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim TIM handle
  * @retval None
  */
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim1)
{
    if (hlptim1->Instance != LPTIM1) return;

    static uint32_t led_timer_ms = 0;
    led_timer_ms += LPTIM_TICK_MS;

    // Re-read these constants inside the function or define them locally if needed
    // Assuming led_period_ms is set in main loop
    const uint32_t on_time_ms = 15; // Fixed 15ms pulse
    uint32_t off_time = led_period_ms - on_time_ms;

    if (led_on) // Currently ON
    {
        // Check if ON_TIME has elapsed (5ms)
        // Since we are checking every 15ms, we check if the counter is past the 5ms mark.
        // The LED will be ON for the first 15ms interval, which is > 15ms,
        // ensuring the pulse is at least 5ms wide.
        if (led_timer_ms >= on_time_ms)
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); // LED OFF
            led_on = 0;
            led_timer_ms = 0;
            Enter_Sleep_Flag = 1;
        }
    }
    else // Currently OFF (waiting for the rest of the period)
    {
        // Check if OFF_TIME has elapsed
        if (led_timer_ms >= off_time)
        {
            Enter_Sleep_Flag = 0;
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); // LED ON
            led_on = 1;
            led_timer_ms = 0;
        }
    }

    static uint32_t adc_elapsed_ms = 0;
    adc_elapsed_ms += LPTIM_TICK_MS;
    if (adc_elapsed_ms >= ADC_CHECK_INTERVAL)
       {
           adc_elapsed_ms = 0;

           Start_ADC_Sequence_Polling();  // measure ADC
           led_period_ms = Calculate_Period_ms(ADC_Value);  // adjust blink speed
       }
}


/**
  * @brief Handler for the external switch (WKUP1_Pin) press event (PA10/EXTI).
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == WKUP1_Pin)
    {
        static uint32_t last_press_tick = 0;
        uint32_t now = HAL_GetTick();

        // Debounce: ignore triggers within 150 ms of the last one
        if (now - last_press_tick < 500)
            return; // Ignore bounce

        last_press_tick = now;

        if (System_Mode == MODE_ACTIVE)
        {
            System_Mode = MODE_STANDBY_HALT;
            Enter_Sleep_Flag = 1;
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
