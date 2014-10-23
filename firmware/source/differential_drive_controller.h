#ifndef DIFFERENTIAL_DRIVE_CONTROLLER_H
#define DIFFERENTIAL_DRIVE_CONTROLLER_H

// http://ebldc.com/?p=86 Article on FD Mode vs. SD Mode

#include <avr/io.h>
#include <stdint.h>

#define LEFT_CTRL_PIN  0x80
#define RIGHT_CTRL_PIN 0x40
#define LEFT_PWM_PIN   0x20
#define RIGHT_PWM_PIN  0x10
#define LEFT_MODE_PIN  0x08
#define RIGHT_MODE_PIN 0x04
#define DRV8833_EN     0x02
#define DRV8833_FAULT  0x01

class CDifferentialDriveController {
public:

   enum class EMode {
      COAST,
      REVERSE,
      REVERSE_PWM_FD,
      REVERSE_PWM_SD,
      FORWARD,
      FORWARD_PWM_FD,
      FORWARD_PWM_SD,
      BRAKE
   };

CDifferentialDriveController();

void Enable();

void Disable();

   void SetLeftMotor(EMode e_mode, uint8_t un_duty_cycle = 0);

   void SetRightMotor(EMode e_mode, uint8_t un_duty_cycle = 0);


private:

   

};

#endif
