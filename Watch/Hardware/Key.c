#include "Key.h"

/* 键值：0=无，1=Key1(PA6)，2=Key2(PA4)，3=Key3(PA2)，4=Key3长按 */
static volatile uint8_t Key_Num = 0;
static int press_time = 0;

/* 主任务调用：取一次键值，取完清零 */
uint8_t Key_GetNum(void)
{
    uint8_t Temp;
    if (Key_Num)
    {
        Temp = Key_Num;
        Key_Num = 0;
        return Temp;
    }
    return 0;
}

/* 每 1ms 调用：累计 Key3 按住的时间，用于长按判断 */
void Key3_Tick(void)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_RESET)
        press_time++;
    else
        press_time = 0;
}

/* 返回当前按键的瞬时状态（内部函数） */
static uint8_t Key_GetState(void)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET)      return 1;  // Key1
    else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) return 2;  // Key2
    else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_RESET)            // Key3
        return (press_time > 1000) ? 4 : 3;
    return 0;
}

/* 每 1ms 调用：内部每 20ms 采样一次，松手时锁存键值 */
void Key_Tick(void)
{
    static uint8_t Count = 0;
    static uint8_t CurrentState = 0, PreState = 0;

    Count++;
    if (Count >= 20)
    {
        Count = 0;
        PreState = CurrentState;
        CurrentState = Key_GetState();
        if (PreState != 0 && CurrentState == 0)   // 上次按着、这次松开 → 键值有效
        {
            Key_Num = PreState;
        }
    }
}
