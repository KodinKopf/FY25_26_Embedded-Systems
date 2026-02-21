/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* Uses htim1/htim2 declared in tim.c via tim.h */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* ---- Pin mapping (per your wiring) ----
   PA9  = PWM wave      (TIM1_CH2)
   PA0  = PWM wave 2    (TIM2_CH1)
   PA10 = PWM wave 3    (TIM1_CH3)
   PA8  = PWM wave 4    (TIM1_CH1)

   PC10 = D1 polarity
   PC11 = D2 (unused)
   PC12 = RFG Run enable
   PA4  = D3 (unused)
   PB0  = RFE
*/
#define POLARITY_OUT_GPIO_Port   GPIOC
#define POLARITY_OUT_Pin         GPIO_PIN_10   /* PC10 = D1 (polarity) */

#define RFE_CTL_GPIO_Port        GPIOB
#define RFE_CTL_Pin              GPIO_PIN_0    /* PB0 = RFE */

#define RUN_EN_OUT_GPIO_Port     GPIOC
#define RUN_EN_OUT_Pin           GPIO_PIN_12   /* PC12 = RFG Run (run enable) */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static volatile uint8_t g_polarity_out = 1;       /* 1=retain, 0=invert */
static volatile uint8_t g_desiredDutyPercent = 0; /* 0..100 (intended motor effect) */

/* Demo sequencer */
typedef struct {
  uint8_t duty;      /* 0..100 intended */
  uint8_t polarity;  /* 1=pass, 0=invert */
  uint8_t rfe;       /* 0=normal, 1=coast/raised */
  uint16_t hold_ms;
} PropulsionDemoStep;

static const PropulsionDemoStep kDemo[] = {
  /* duty, pol, rfe, hold */
  {70, 1, 0, 2000},  /* accelerate */
  {20, 0, 0, 2000},  /* decelerate (invert) */
  {50, 1, 0, 2000},  /* neutral */
  {80, 0, 0, 2000},  /* strong brake (invert) */
  { 0, 1, 1, 2000},  /* coast: RFE raised, no drive */
};
static uint32_t g_demo_idx = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Minimal helper set */
static inline void SetPolarityOut(uint8_t v);
static inline void SetRFE(uint8_t level);
static inline void KeepRunEnableOn(void);
static inline void PropulsionSetDesiredDuty(uint8_t duty_percent);

static void PWM_SetPercent(TIM_HandleTypeDef* htim, uint32_t channel, uint8_t dutyPct);

/* Duplicate same PWM wave to ALL 4 PWM outputs */
static void ApplyPropulsionPWM_All4(uint8_t duty, uint8_t polarity);

/* Spec sequencing: RFE first, then (after delay) RunEnable */
static void Sequence_RFE_then_RunEnable(uint8_t rfe_level, uint32_t delay_ms);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static inline void SetPolarityOut(uint8_t v)
{
  g_polarity_out = v ? 1U : 0U;
  HAL_GPIO_WritePin(POLARITY_OUT_GPIO_Port, POLARITY_OUT_Pin,
                    g_polarity_out ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static inline void SetRFE(uint8_t level) /* 1=coast/raised, 0=normal */
{
  HAL_GPIO_WritePin(RFE_CTL_GPIO_Port, RFE_CTL_Pin,
                    level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static inline void KeepRunEnableOn(void)
{
  HAL_GPIO_WritePin(RUN_EN_OUT_GPIO_Port, RUN_EN_OUT_Pin, GPIO_PIN_SET);
}

static inline void PropulsionSetDesiredDuty(uint8_t duty_percent)
{
  if (duty_percent > 100U) duty_percent = 100U;
  g_desiredDutyPercent = duty_percent;
}

/* Duty% -> CCR; uses live ARR so duty math stays correct if ARR changes */
static void PWM_SetPercent(TIM_HandleTypeDef* htim, uint32_t channel, uint8_t dutyPct)
{
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(htim);
  uint32_t ccr = ((uint32_t)dutyPct * (arr + 1U)) / 100U;
  __HAL_TIM_SET_COMPARE(htim, channel, ccr);
}

/* Spec: polarity=1 => eff=duty; polarity=0 => eff=100-duty
   This version duplicates the SAME effective duty across all 4 outputs. */
static void ApplyPropulsionPWM_All4(uint8_t duty, uint8_t polarity)
{
  uint8_t eff = polarity ? duty : (uint8_t)(100U - duty);

  /* TIM1: PA8/PA9/PA10 */
  PWM_SetPercent(&htim1, TIM_CHANNEL_1, eff); /* PA8  = TIM1_CH1 (PWM wave 4) */
  PWM_SetPercent(&htim1, TIM_CHANNEL_2, eff); /* PA9  = TIM1_CH2 (PWM wave)   */
  PWM_SetPercent(&htim1, TIM_CHANNEL_3, eff); /* PA10 = TIM1_CH3 (PWM wave 3) */

  /* TIM2: PA0 */
  PWM_SetPercent(&htim2, TIM_CHANNEL_1, eff); /* PA0  = TIM2_CH1 (PWM wave 2) */
}

/* Enforce spec: set RFE first, then (after delay) assert RunEnable. */
static void Sequence_RFE_then_RunEnable(uint8_t rfe_level, uint32_t delay_ms)
{
  SetRFE(rfe_level);              /* Step 1: RFE */
  HAL_Delay(delay_ms);            /* Required interlock */
  KeepRunEnableOn();              /* Step 2: RunEnable */
}

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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* Force both timers to ~100 kHz: 84MHz / (PSC+1)/(ARR+1) = 84e6/840 = 100kHz */
  __HAL_TIM_SET_PRESCALER(&htim1, 0);
  __HAL_TIM_SET_AUTORELOAD(&htim1, 839);
  HAL_TIM_GenerateEvent(&htim1, TIM_EVENTSOURCE_UPDATE);
  __HAL_TIM_SET_COUNTER(&htim1, 0);

  __HAL_TIM_SET_PRESCALER(&htim2, 0);
  __HAL_TIM_SET_AUTORELOAD(&htim2, 839);
  HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE);
  __HAL_TIM_SET_COUNTER(&htim2, 0);

  /* Start PWM on all 4 outputs */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  /* Bring-up sequence per spec: RFE first, then (after 0.5s) RunEnable */
  Sequence_RFE_then_RunEnable(0 /* normal */, 500 /* ms */);

  /* Initial state */
  PropulsionSetDesiredDuty(70);
  SetPolarityOut(1);
  ApplyPropulsionPWM_All4(g_desiredDutyPercent, g_polarity_out);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    PropulsionSetDesiredDuty(70);   /* accelerate: 70% effective */
    SetPolarityOut(1);              /* pass duty */
    ApplyPropulsionPWM_All4(g_desiredDutyPercent, g_polarity_out);
    HAL_Delay(2000);

    PropulsionSetDesiredDuty(20);   /* decelerate: 20% effective */
    SetPolarityOut(0);              /* invert => output 80% (power stage sees 20% effect) */
    ApplyPropulsionPWM_All4(g_desiredDutyPercent, g_polarity_out);
    HAL_Delay(2000);

    PropulsionSetDesiredDuty(50);   /* neutral/partial accel: 50% effective */
    SetPolarityOut(1);
    ApplyPropulsionPWM_All4(g_desiredDutyPercent, g_polarity_out);
    HAL_Delay(2000);

    PropulsionSetDesiredDuty(80);   /* strong brake: 80% intended -> output 20% with invert */
    SetPolarityOut(0);
    ApplyPropulsionPWM_All4(g_desiredDutyPercent, g_polarity_out);
    HAL_Delay(2000);

    /* Coasting: RFE=1 (connectors raised), duty=0%. RunEnable stays logically ON. */
    SetRFE(1);
    PropulsionSetDesiredDuty(0);
    ApplyPropulsionPWM_All4(g_desiredDutyPercent, g_polarity_out);
    HAL_Delay(2000);

    /* Re-engage per spec (0.5s gap) */
    Sequence_RFE_then_RunEnable(0 /* normal */, 500 /* ms */);
    /* ---- end demo sequence ---- */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  __disable_irq();
  while (1) {}
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
  (void)file; (void)line;
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
