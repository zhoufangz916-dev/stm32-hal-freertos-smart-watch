#include "MPU6050.h"
#include "MyI2C.h"
#include <math.h>

/* 写一个寄存器 */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
  MyI2C_Start();
  MyI2C_SendByte(MPU6050_ADDRESS);    /* 地址 + 写方向 */
  MyI2C_ReceiveAck();
  MyI2C_SendByte(RegAddress);         /* 要写哪个寄存器 */
  MyI2C_ReceiveAck();
  MyI2C_SendByte(Data);               /* 写入的数据 */
  MyI2C_ReceiveAck();
  MyI2C_Stop();
}

/* 读一个寄存器 */
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
  uint8_t Data;

  MyI2C_Start();
  MyI2C_SendByte(MPU6050_ADDRESS);    /* 地址 + 写，为了先写寄存器地址 */
  MyI2C_ReceiveAck();
  MyI2C_SendByte(RegAddress);
  MyI2C_ReceiveAck();

  MyI2C_Start();                      /* 重复起始，不释放总线 */
  MyI2C_SendByte(MPU6050_ADDRESS | 0x01);  /* 地址 + 读方向 */
  MyI2C_ReceiveAck();
  Data = MyI2C_ReceiveByte();         /* 读 8 位数据 */
  MyI2C_SendAck(1);                   /* 非应答：告诉从机只读这一个字节 */
  MyI2C_Stop();

  return Data;
}

/* 读芯片 ID */
uint8_t MPU6050_GetID(void)
{
  return MPU6050_ReadReg(MPU6050_WHO_AM_I);   /* 正常返回 0x68 */
}

void MPU6050_Init(void)
{
  MyI2C_Init();

  /* 电源管理1：0x01 = 退出睡眠 + 时钟源选陀螺仪X轴PLL */
  MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
  /* 电源管理2：0x00 = 所有轴都不待机 */
  MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);
  /* 采样率分频：采样率 = 1kHz / (1+4) = 200Hz */
  MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x04);
  /* 低通滤波：0x06 = 最强滤波，滤掉抖动，输出更稳 */
  MPU6050_WriteReg(MPU6050_CONFIG, 0x06);
  /* 陀螺仪量程：0x18 = ±2000°/s */
  MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);
  /* 加速度量程：0x18 = ±16g */
  MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);
}

void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
  uint8_t DataH, DataL;

  DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);
  DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);
  *AccX = (int16_t)((DataH << 8) | DataL);

  DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);
  DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);
  *AccY = (int16_t)((DataH << 8) | DataL);

  DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);
  DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);
  *AccZ = (int16_t)((DataH << 8) | DataL);

  DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);
  DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);
  *GyroX = (int16_t)((DataH << 8) | DataL);

  DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);
  DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);
  *GyroY = (int16_t)((DataH << 8) | DataL);

  DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);
  DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);
  *GyroZ = (int16_t)((DataH << 8) | DataL);
}

void MPU6050_GetAngles(float *Roll, float *Pitch)
{
  int16_t ax, ay, az, gx, gy, gz;

  MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);

  *Roll  = atan2f((float)ay, (float)az) * 57.29578f;
  *Pitch = atan2f((float)-ax, (float)az) * 57.29578f;
}
