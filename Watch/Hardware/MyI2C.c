#include "MyI2C.h"

/* SCL = PB10，SDA = PB11（CubeMX 已配置为开漏输出） */

static void MyI2C_W_SCL(uint8_t BitValue)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void MyI2C_W_SDA(uint8_t BitValue)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t MyI2C_R_SDA(void)
{
  return (uint8_t)HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
}

void MyI2C_Init(void)
{
  MyI2C_W_SDA(1);      /* 释放总线，两根线都拉高 */
  MyI2C_W_SCL(1);
}

void MyI2C_Start(void)
{
  MyI2C_W_SDA(1);
  MyI2C_W_SCL(1);
  MyI2C_W_SDA(0);      /* SCL 高电平期间拉低 SDA = 起始 */
  MyI2C_W_SCL(0);
}

void MyI2C_Stop(void)
{
  MyI2C_W_SDA(0);
  MyI2C_W_SCL(1);
  MyI2C_W_SDA(1);      /* SCL 高电平期间释放 SDA = 停止 */
}

void MyI2C_SendByte(uint8_t Byte)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    MyI2C_W_SDA(!!(Byte & (0x80 >> i)));   /* 高位先发 */
    MyI2C_W_SCL(1);                        /* 从机在高电平期间读 SDA */
    MyI2C_W_SCL(0);
  }
}

uint8_t MyI2C_ReceiveByte(void)
{
  uint8_t i, Byte = 0x00;

  MyI2C_W_SDA(1);                          /* 释放 SDA，交给从机 */
  for (i = 0; i < 8; i++)
  {
    MyI2C_W_SCL(1);
    if (MyI2C_R_SDA()) { Byte |= (0x80 >> i); }
    MyI2C_W_SCL(0);
  }
  return Byte;
}

void MyI2C_SendAck(uint8_t AckBit)
{
  MyI2C_W_SDA(AckBit);     /* 0 = 应答，1 = 非应答 */
  MyI2C_W_SCL(1);
  MyI2C_W_SCL(0);
}

uint8_t MyI2C_ReceiveAck(void)
{
  uint8_t AckBit;

  MyI2C_W_SDA(1);          /* 主机释放 SDA，从机拉低表示应答 */
  MyI2C_W_SCL(1);
  AckBit = MyI2C_R_SDA();
  MyI2C_W_SCL(0);
  return AckBit;           /* 0 = 有应答，1 = 无应答 */
}
