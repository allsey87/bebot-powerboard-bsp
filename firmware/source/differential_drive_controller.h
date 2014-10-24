#ifndef DIFFERENTIAL_DRIVE_CONTROLLER_H
#define DIFFERENTIAL_DRIVE_CONTROLLER_H

// http://ebldc.com/?p=86 Article on FD Mode vs. SD Mode
// http://electronics.stackexchange.com/questions/67663/criteria-behind-selecting-pwm-frequency-for-speed-control-of-a-dc-motor
// http://arduino.cc/en/Tutorial/SecretsOfArduinoPWM

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

#define RIGHT_ENC_CHA_PIN 0x80
#define RIGHT_ENC_CHB_PIN 0x40
#define LEFT_ENC_CHA_PIN  0x20
#define LEFT_ENC_CHB_PIN  0x10

#define LEFT_ENC_MASK (LEFT_ENC_CHA_PIN | LEFT_ENC_CHB_PIN)
#define RIGHT_ENC_MASK (RIGHT_ENC_CHA_PIN | RIGHT_ENC_CHB_PIN)

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


//RPM
int16_t GetLeftRPM();

int16_t GetRightRPM();


private:

   

};

#endif
