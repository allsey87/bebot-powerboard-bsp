
#include "accelerometer_system.h"

#include <firmware.h>

#define MPU6050_DEV_ADDR 0x68

/****************************************/
/****************************************/

bool CAccelerometerSystem::Init() {
   /* Probe */
   Firmware::GetInstance().GetTWController().BeginTransmission(MPU6050_DEV_ADDR);
   Firmware::GetInstance().GetTWController().Write(static_cast<uint8_t>(ERegister::WHOAMI));
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(MPU6050_DEV_ADDR, 1, true);
         
   if(Firmware::GetInstance().GetTWController().Read() != MPU6050_DEV_ADDR) 
      return false;

   /* select internal clock, disable sleep/cycle mode, enable temperature sensor*/
   Firmware::GetInstance().GetTWController().BeginTransmission(MPU6050_DEV_ADDR);
   Firmware::GetInstance().GetTWController().Write(static_cast<uint8_t>(ERegister::PWR_MGMT_1));
   Firmware::GetInstance().GetTWController().Write(0x00);
   Firmware::GetInstance().GetTWController().EndTransmission(true);

   return true;
}

/****************************************/
/****************************************/


CAccelerometerSystem::SReading CAccelerometerSystem::GetReading() {
   /* Buffer for holding accelerometer result */
   uint8_t punRes[8];

   Firmware::GetInstance().GetTWController().BeginTransmission(MPU6050_DEV_ADDR);
   Firmware::GetInstance().GetTWController().Write(static_cast<uint8_t>(ERegister::ACCEL_XOUT_H));
   Firmware::GetInstance().GetTWController().EndTransmission(false);
   Firmware::GetInstance().GetTWController().Read(MPU6050_DEV_ADDR, 8, true);
   /* Read the requested 8 bytes */
   for(uint8_t i = 0; i < 8; i++) {
      punRes[i] = Firmware::GetInstance().GetTWController().Read();
   }

   return SReading { 
      int16_t((punRes[0] << 8) | punRes[1]),
      int16_t((punRes[2] << 8) | punRes[3]),
      int16_t((punRes[4] << 8) | punRes[5]),
      (int16_t((punRes[6] << 8) | punRes[7]) + 12412) / 340};
}

/****************************************/
/****************************************/



