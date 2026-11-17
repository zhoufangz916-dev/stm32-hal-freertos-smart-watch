/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "OLED.h"
#include "Key.h"
#include "rtc.h"
#include "MyI2C.h"
#include "MPU6050.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "Dino.h"
#include "MenuIcon.h"

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
/* USER CODE BEGIN Variables */
osThreadId_t menuTaskHandle;
const osThreadAttr_t menuTask_attributes = {
  .name = "menuTask",
  .stack_size = 128 * 4,          // 512 字节
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t stopWatchTaskHandle;
const osThreadAttr_t stopWatchTask_attributes = {
  .name = "stopWatchTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t mpuTaskHandle;
const osThreadAttr_t mpuTask_attributes = {
  .name = "mpuTask",
  .stack_size = 256 * 4,          // 1024 字节：浮点运算 + OLED 调用更深
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t gradienterTaskHandle;
const osThreadAttr_t gradienterTask_attributes = {
  .name = "gradienterTask",
  .stack_size = 256 * 4,          // 1024 字节
  .priority = (osPriority_t) osPriorityNormal,
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t setTimeTaskHandle;
const osThreadAttr_t setTimeTask_attributes = {
  .name = "setTimeTask",
  .stack_size = 128 * 4,          // 512 字节
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t emojiTaskHandle;
const osThreadAttr_t emojiTask_attributes = {
  .name = "emojiTask",
  .stack_size = 256 * 4,          // 1024 字节：画弧线会用到 atan2 数学库
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t gameTaskHandle;
const osThreadAttr_t gameTask_attributes = {
  .name = "gameTask",
  .stack_size = 256 * 4,          // 1024 字节
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartMenuTask(void *argument);
void StartStopWatchTask(void *argument);
void StartMpuTask(void *argument);
void StartGradienterTask(void *argument);
void StartSetTimeTask(void *argument);
void StartEmojiTask(void *argument);
void StartGameTask(void *argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  menuTaskHandle = osThreadNew(StartMenuTask, NULL, &menuTask_attributes);
  stopWatchTaskHandle = osThreadNew(StartStopWatchTask, NULL, &stopWatchTask_attributes);
  mpuTaskHandle = osThreadNew(StartMpuTask, NULL, &mpuTask_attributes);
  gradienterTaskHandle = osThreadNew(StartGradienterTask, NULL, &gradienterTask_attributes);
  setTimeTaskHandle = osThreadNew(StartSetTimeTask, NULL, &setTimeTask_attributes);
  emojiTaskHandle = osThreadNew(StartEmojiTask, NULL, &emojiTask_attributes);
  gameTaskHandle = osThreadNew(StartGameTask, NULL, &gameTask_attributes);

  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  uint8_t last_sec = 0xFF;

  OLED_Init();
  MPU6050_Init();          // 初始化只做一次，放循环外
  OLED_Clear();
  OLED_Update();

  for(;;)
  {
    uint8_t key = Key_GetNum();

    if (key == 3)                    // Key3：进入菜单
    {
      osThreadResume(menuTaskHandle);
      osThreadSuspend(osThreadGetId());

      OLED_Clear();
      last_sec = 0xFF;
    }

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    if (sTime.Seconds != last_sec)
    {
      last_sec = sTime.Seconds;

      /* 日期行：20YY-MM-DD（6×8 字体） */
      OLED_ShowString(34, 0, "20", OLED_6X8);
      OLED_ShowNum(46, 0, sDate.Year, 2, OLED_6X8);
      OLED_ShowString(58, 0, "-", OLED_6X8);
      OLED_ShowNum(64, 0, sDate.Month, 2, OLED_6X8);
      OLED_ShowString(76, 0, "-", OLED_6X8);
      OLED_ShowNum(82, 0, sDate.Date, 2, OLED_6X8);

      /* 时间行：HH:MM:SS（12×24 大字体） */
      OLED_ShowNum(16, 16, sTime.Hours, 2, OLED_12X24);
      OLED_ShowString(40, 16, ":", OLED_12X24);
      OLED_ShowNum(52, 16, sTime.Minutes, 2, OLED_12X24);
      OLED_ShowString(76, 16, ":", OLED_12X24);
      OLED_ShowNum(88, 16, sTime.Seconds, 2, OLED_12X24);

      OLED_ShowString(40, 52, "K3: menu", OLED_6X8);
      OLED_Update();
    }

    osDelay(100);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
#define MENU_COUNT  8

static uint8_t menuIndex = 0;     // 当前选中项 0~7

/* 循环取图标下标：支持 -1、8 这样的越界值 */
static uint8_t Menu_Wrap(int8_t index)
{
  while (index < 0) index += MENU_COUNT;
  while (index >= MENU_COUNT) index -= MENU_COUNT;
  return (uint8_t)index;
}

/* 按滑动偏移画一帧：x_pre=48 为静止位置 */
static void Menu_Draw(uint8_t x_pre, uint8_t index)
{
  OLED_Clear();

  /* 选中框固定不动 */
  OLED_ShowImage(42, 10, 44, 48, Menu_Frame);

  /* 左、中、右、右二，四个图标跟着 x_pre 移动 */
  OLED_ShowImage((int16_t)x_pre - 48, 16, 32, 32, Menu_Graph[Menu_Wrap(index - 1)]);
  OLED_ShowImage((int16_t)x_pre,      16, 32, 32, Menu_Graph[Menu_Wrap(index)]);
  OLED_ShowImage((int16_t)x_pre + 48, 16, 32, 32, Menu_Graph[Menu_Wrap(index + 1)]);
  OLED_ShowImage((int16_t)x_pre + 96, 16, 32, 32, Menu_Graph[Menu_Wrap(index + 2)]);

  OLED_Update();
}

/* 切换动画：dir=+1 下一项（左移），dir=-1 上一项（右移） */
static void Menu_Animate(int8_t dir)
{
  uint8_t target = Menu_Wrap(menuIndex + dir);
  uint8_t x = 48;

  while (1)
  {
    if (dir > 0)
    {
      if (x >= 8) x -= 8;
      else x = 0;
    }
    else
    {
      x += 8;
      if (x > 96) x = 96;
    }

    Menu_Draw(x, menuIndex);
    osDelay(8);

    if ((dir > 0 && x == 0) || (dir < 0 && x == 96)) break;
  }

  menuIndex = target;
  Menu_Draw(48, menuIndex);
}

void StartMenuTask(void *argument)
{
  uint8_t key;

  /* 创建后先挂起，等时钟任务用 Key3 唤醒 */
  osThreadSuspend(osThreadGetId());

  for(;;)
  {
    Menu_Draw(48, menuIndex);

    /* 等待按键 */
    do
    {
      key = Key_GetNum();
      osDelay(20);
    } while (key == 0);

    if (key == 1)                       // Key1：上一项
    {
      Menu_Animate(-1);
    }
    else if (key == 2)                  // Key2：下一项
    {
      Menu_Animate(1);
    }
    else if (key == 3 || key == 4)      // Key3：确认；长按：回主页
    {
      if (key == 4 || menuIndex == 0)   // 长按 或 选中 Clock
      {
        osThreadResume(defaultTaskHandle);
        osThreadSuspend(osThreadGetId());
      }
      else if (menuIndex == 1)          // Stopwatch
      {
        osThreadResume(stopWatchTaskHandle);
        osThreadSuspend(osThreadGetId());
      }
      else if (menuIndex == 2)          // Flashlight（无独立任务，内层循环）
      {
        uint8_t led_on;

        led_on = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_RESET);

        for (;;)
        {
          OLED_Clear();
          OLED_ShowString(34, 0, "Flashlight", OLED_6X8);
          if (led_on) OLED_ShowString(56, 24, "ON", OLED_8X16);
          else        OLED_ShowString(52, 24, "OFF", OLED_8X16);
          OLED_ShowString(34, 44, "K1: toggle", OLED_6X8);
          OLED_ShowString(40, 54, "K3: back", OLED_6X8);
          OLED_Update();

          do
          {
            key = Key_GetNum();
            osDelay(20);
          } while (key == 0);

          if (key == 1)
          {
            led_on = !led_on;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15,
                              led_on ? GPIO_PIN_RESET : GPIO_PIN_SET);
          }
          else if (key == 3)
          {
            break;
          }
          else if (key == 4)
          {
            osThreadResume(defaultTaskHandle);
            osThreadSuspend(osThreadGetId());
            break;
          }
        }
      }
      else if (menuIndex == 3)          // MPU6050
      {
        osThreadResume(mpuTaskHandle);
        osThreadSuspend(osThreadGetId());
      }
      else if (menuIndex == 4)          // Game
      {
        osThreadResume(gameTaskHandle);
        osThreadSuspend(osThreadGetId());
      }
      else if (menuIndex == 5)          // Emoji
      {
        osThreadResume(emojiTaskHandle);
        osThreadSuspend(osThreadGetId());
      }
      else if (menuIndex == 6)          // Gradienter
      {
        osThreadResume(gradienterTaskHandle);
        osThreadSuspend(osThreadGetId());
      }
      else if (menuIndex == 7)          // Set Time
      {
        osThreadResume(setTimeTaskHandle);
        osThreadSuspend(osThreadGetId());
      }
    }
  }
}

static void StopWatch_Draw(uint32_t total_ms)
{
  /* 体育秒表格式：MM:SS.CC（分:秒.百分秒） */
  OLED_ShowNum(32, 16, (total_ms / 60000) % 100, 2, OLED_8X16);   // 分
  OLED_ShowString(48, 16, ":", OLED_8X16);
  OLED_ShowNum(56, 16, (total_ms / 1000) % 60, 2, OLED_8X16);     // 秒
  OLED_ShowString(72, 16, ".", OLED_8X16);
  OLED_ShowNum(80, 16, (total_ms / 10) % 100, 2, OLED_8X16);      // 百分秒
}

void StartStopWatchTask(void *argument)
{
  uint32_t elapsed_ms = 0;      // 已累计的毫秒
  uint32_t start_tick = 0;      // 本次启动的时刻
  uint32_t total;
  uint32_t shown_centi = 0xFFFFFFFF;
  uint8_t running = 0;
  uint8_t need_redraw = 1;

  osThreadSuspend(osThreadGetId());   // 创建后先挂起

  for(;;)
  {
    uint8_t key = Key_GetNum();

    if (key == 1)                       // Key1：开始 / 暂停
    {
      if (running)
      {
        elapsed_ms += osKernelGetTickCount() - start_tick;
        running = 0;
      }
      else
      {
        start_tick = osKernelGetTickCount();
        running = 1;
      }
      need_redraw = 1;
    }
    else if (key == 2)                  // Key2：复位
    {
      elapsed_ms = 0;
      running = 0;
      need_redraw = 1;
    }
    else if (key == 3)                  // Key3：返回菜单（后台继续计时）
    {
      osThreadResume(menuTaskHandle);
      osThreadSuspend(osThreadGetId());
      need_redraw = 1;
    }
    else if (key == 4)                  // 长按 Key3：回主页（后台继续计时）
    {
      osThreadResume(defaultTaskHandle);
      osThreadSuspend(osThreadGetId());
      need_redraw = 1;
    }

    total = elapsed_ms + (running ? (osKernelGetTickCount() - start_tick) : 0);

    if (need_redraw)
    {
      OLED_Clear();
      OLED_ShowString(37, 0, "Stopwatch", OLED_6X8);
      OLED_ShowString(0, 38, "K1 Start/Stop", OLED_6X8);
      OLED_ShowString(0, 46, "K2 Reset", OLED_6X8);
      OLED_ShowString(0, 54, "K3 Back", OLED_6X8);
      StopWatch_Draw(total);
      OLED_Update();
      need_redraw = 0;
      shown_centi = total / 10;
    }
    else if (running && (total / 10) != shown_centi)
    {
      StopWatch_Draw(total);
      OLED_UpdateArea(32, 16, 64, 16);   // 只刷新时间行，节省 I2C
      shown_centi = total / 10;
    }

    osDelay(20);
  }
}

void StartMpuTask(void *argument)
{
  float roll, pitch;
  int32_t roll_i, pitch_i;
  int32_t last_roll = 9999, last_pitch = 9999;
  uint8_t need_redraw = 1;

  osThreadSuspend(osThreadGetId());   // 创建后先挂起

  for(;;)
  {
    uint8_t key = Key_GetNum();

    if (key == 3)                     // 返回菜单
    {
      osThreadResume(menuTaskHandle);
      osThreadSuspend(osThreadGetId());
      need_redraw = 1;
    }
    else if (key == 4)                // 长按：回主页
    {
      osThreadResume(defaultTaskHandle);
      osThreadSuspend(osThreadGetId());
      need_redraw = 1;
    }

    MPU6050_GetAngles(&roll, &pitch);
    roll_i  = (int32_t)roll;
    pitch_i = (int32_t)pitch;

    /* 进入页面或角度变化时才重画，避免无意义的 I2C 传输 */
    if (need_redraw || roll_i != last_roll || pitch_i != last_pitch)
    {
      if (need_redraw)
      {
        OLED_Clear();
        OLED_ShowString(0, 0, "Roll:", OLED_8X16);
        OLED_ShowString(0, 20, "Pitch:", OLED_8X16);
        OLED_ShowString(0, 44, "K3: back", OLED_6X8);
        OLED_ShowString(0, 54, "Hold K3: home", OLED_6X8);
        need_redraw = 0;
      }

      OLED_ShowSignedNum(48, 0, roll_i, 2, OLED_8X16);    // 符号 + 2 位数字
      OLED_ShowSignedNum(56, 20, pitch_i, 2, OLED_8X16);
      OLED_Update();

      last_roll = roll_i;
      last_pitch = pitch_i;
    }

    osDelay(50);                      // 20Hz 刷新
  }
}

void StartGradienterTask(void *argument)
{
  float roll, pitch;
  float dx, dy, dist;
  int16_t bx, by;

  osThreadSuspend(osThreadGetId());   // 创建后先挂起

  for(;;)
  {
    uint8_t key = Key_GetNum();

    if (key == 3)                     // 返回菜单
    {
      osThreadResume(menuTaskHandle);
      osThreadSuspend(osThreadGetId());
    }
    else if (key == 4)                // 长按：回主页
    {
      osThreadResume(defaultTaskHandle);
      osThreadSuspend(osThreadGetId());
    }

    MPU6050_GetAngles(&roll, &pitch);

    /* 角度 → 像素偏移：右倾球向右，前倾球向下 */
    dx = roll;
    dy = pitch;

    /* 限制在圆内：偏移向量太长时等比例缩小 */
    dist = sqrtf(dx * dx + dy * dy);
    if (dist > 26.0f)
    {
      dx = dx * 26.0f / dist;
      dy = dy * 26.0f / dist;
    }

    bx = (int16_t)(64 + dx);
    by = (int16_t)(32 + dy);

    /* 每帧重画：清屏 → 外圆 → 填充小球 → 刷屏 */
    OLED_Clear();
    OLED_DrawCircle(64, 32, 30, OLED_UNFILLED);
    OLED_DrawCircle(bx, by, 4, OLED_FILLED);
    OLED_Update();

    osDelay(50);                      // 20Hz
  }
}

static uint8_t SetTime_MaxDay(uint8_t year, uint8_t month)
{
  static const uint8_t days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
  uint8_t d = days[month - 1];

  if (month == 2 && (year % 4) == 0)   // 2000~2099 内能被4整除就是闰年
  {
    d = 29;
  }
  return d;
}

static void SetTime_Draw(uint8_t field,
                         uint8_t year, uint8_t month, uint8_t day,
                         uint8_t hour, uint8_t minute, uint8_t second)
{
  OLED_Clear();

  /* 日期行：20YY-MM-DD */
  OLED_ShowString(24, 0, "20", OLED_8X16);
  OLED_ShowNum(40, 0, year, 2, OLED_8X16);
  OLED_ShowString(56, 0, "-", OLED_8X16);
  OLED_ShowNum(64, 0, month, 2, OLED_8X16);
  OLED_ShowString(80, 0, "-", OLED_8X16);
  OLED_ShowNum(88, 0, day, 2, OLED_8X16);

  /* 时间行：HH:MM:SS */
  OLED_ShowNum(32, 20, hour, 2, OLED_8X16);
  OLED_ShowString(48, 20, ":", OLED_8X16);
  OLED_ShowNum(56, 20, minute, 2, OLED_8X16);
  OLED_ShowString(72, 20, ":", OLED_8X16);
  OLED_ShowNum(80, 20, second, 2, OLED_8X16);

  /* 当前字段反色 */
  if (field == 0)      OLED_ReverseArea(40, 0, 16, 16);
  else if (field == 1) OLED_ReverseArea(64, 0, 16, 16);
  else if (field == 2) OLED_ReverseArea(88, 0, 16, 16);
  else if (field == 3) OLED_ReverseArea(32, 20, 16, 16);
  else if (field == 4) OLED_ReverseArea(56, 20, 16, 16);
  else                 OLED_ReverseArea(80, 20, 16, 16);

  OLED_ShowString(0, 44, "K1:field  K2:+1", OLED_6X8);
  OLED_ShowString(0, 54, "K3: save", OLED_6X8);
  OLED_Update();
}

static void SetTime_Save(uint8_t year, uint8_t month, uint8_t day,
                         uint8_t hour, uint8_t minute, uint8_t second)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_DateTypeDef nowDate;

  HAL_RTC_GetDate(&hrtc, &nowDate, RTC_FORMAT_BIN);  // 取出当前 WeekDay

  sTime.Hours   = hour;
  sTime.Minutes = minute;
  sTime.Seconds = second;
  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

  sDate.Year    = year;
  sDate.Month   = month;
  sDate.Date    = day;
  sDate.WeekDay = nowDate.WeekDay;                   // 星期几不显示，沿用原来的
  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

void StartSetTimeTask(void *argument)
{
  uint8_t field;
  uint8_t year, month, day, hour, minute, second;
  uint8_t key;
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  osThreadSuspend(osThreadGetId());   // 创建后先挂起

  for(;;)
  {
    /* 每次进入页面，先读 RTC 的当前值作为编辑初值 */
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    year   = sDate.Year;
    month  = sDate.Month;
    day    = sDate.Date;
    hour   = sTime.Hours;
    minute = sTime.Minutes;
    second = sTime.Seconds;
    field  = 0;

    for(;;)
    {
      SetTime_Draw(field, year, month, day, hour, minute, second);

      do
      {
        key = Key_GetNum();
        osDelay(20);
      } while (key == 0);

      if (key == 1)                       // 切换字段
      {
        field++;
        if (field >= 6) field = 0;
      }
      else if (key == 2)                  // 当前字段 +1
      {
        if (field == 0)
        {
          year++;
          if (year >= 100) year = 0;
        }
        else if (field == 1)
        {
          month++;
          if (month > 12) month = 1;
        }
        else if (field == 2)
        {
          day++;
          if (day > SetTime_MaxDay(year, month)) day = 1;
        }
        else if (field == 3)
        {
          hour++;
          if (hour > 23) hour = 0;
        }
        else if (field == 4)
        {
          minute++;
          if (minute > 59) minute = 0;
        }
        else
        {
          second++;
          if (second > 59) second = 0;
        }

        /* 改了年份或月份后，原来的“日”可能超过当月最大天数 */
        if (day > SetTime_MaxDay(year, month))
        {
          day = SetTime_MaxDay(year, month);
        }
      }
      else if (key == 3)                  // 保存并回菜单
      {
        SetTime_Save(year, month, day, hour, minute, second);
        osThreadResume(menuTaskHandle);
        osThreadSuspend(osThreadGetId());
        break;
      }
      else if (key == 4)                  // 保存并回主页
      {
        SetTime_Save(year, month, day, hour, minute, second);
        osThreadResume(defaultTaskHandle);
        osThreadSuspend(osThreadGetId());
        break;
      }
    }
  }
}

void StartEmojiTask(void *argument)
{
  uint8_t count = 0;
  uint8_t blink;

  osThreadSuspend(osThreadGetId());   // 创建后先挂起

  for(;;)
  {
    uint8_t key = Key_GetNum();

    if (key == 3)                     // 返回菜单
    {
      osThreadResume(menuTaskHandle);
      osThreadSuspend(osThreadGetId());
      count = 0;
    }
    else if (key == 4)                // 长按：回主页
    {
      osThreadResume(defaultTaskHandle);
      osThreadSuspend(osThreadGetId());
      count = 0;
    }

    /* 2 秒一个眨眼周期，最后 0.2 秒闭眼 */
    count++;
    if (count >= 20) count = 0;
    blink = (count >= 18);

    OLED_Clear();

    /* 脸 */
    OLED_DrawCircle(64, 32, 30, OLED_UNFILLED);

    /* 眼睛：睁眼画填充圆，闭眼画横线 */
    if (blink)
    {
      OLED_DrawLine(42, 26, 54, 26);
      OLED_DrawLine(74, 26, 86, 26);
    }
    else
    {
      OLED_DrawCircle(48, 26, 5, OLED_FILLED);
      OLED_DrawCircle(80, 26, 5, OLED_FILLED);
    }

    /* 嘴巴：圆心(64,40)，半径14，20°~160° 的下半圆弧 = 微笑 */
    OLED_DrawArc(64, 40, 14, 20, 160, OLED_UNFILLED);

    OLED_Update();
    osDelay(100);                     // 10fps
  }
}

void StartGameTask(void *argument)
{
  int result;

  osThreadSuspend(osThreadGetId());

  for (;;)
  {
    DinoGame_Pos_Init();
    result = DinoGame_Animation();

    if (result == 2)                  // 长按 Key3：回主页
    {
      osThreadResume(defaultTaskHandle);
      osThreadSuspend(osThreadGetId());
    }
    else                              // 碰撞结束或短按 Key3：回菜单
    {
      osThreadResume(menuTaskHandle);
      osThreadSuspend(osThreadGetId());
    }
  }
}

/* USER CODE END Application */

