
#include "differential_drive_controller.h"

CDifferentialDriveController::CDifferentialDriveController() {
   /* set the drive system initially to coast mode, with the h-bridge disabled */
   PORTL &= ~(LEFT_MODE_PIN  |
              LEFT_CTRL_PIN  |
              LEFT_PWM_PIN   |
              RIGHT_MODE_PIN |
              RIGHT_CTRL_PIN |
              RIGHT_PWM_PIN  |
              DRV8833_EN);
   /* set the direction of the output pins to output */
   DDRL |= (LEFT_MODE_PIN  |
            LEFT_CTRL_PIN  |
            LEFT_PWM_PIN   |
            RIGHT_MODE_PIN |
            RIGHT_CTRL_PIN |
            RIGHT_PWM_PIN  |
            DRV8833_EN);
   /* set up the timer to generate the PWM signal */
   /* set timer 5 prescale factor to 64 */
   TCCR5B |= ((1 << CS51) | (1 << CS50));
   /* Put timer 5 in 8-bit phase correct PWM mode */
   TCCR5A |= (1 << WGM50);
   /* Left and right motor duty cycle to zero respectively*/
   OCR5C = 0;
   OCR5B = 0;
}

void CDifferentialDriveController::Enable() {
   /* Enable the H-Bridge */
   PORTL |= DRV8833_EN;
}

void CDifferentialDriveController::Disable() {
   /* Disable the H-Bridge */
   PORTL &= ~DRV8833_EN;
}
   
void CDifferentialDriveController::SetLeftMotor(EMode e_mode, uint8_t un_duty_cycle) {
   switch(e_mode) {
   case EMode::REVERSE_PWM_FD:
   case EMode::REVERSE_PWM_SD:
   case EMode::FORWARD_PWM_FD:
   case EMode::FORWARD_PWM_SD:
      OCR5C = un_duty_cycle;
      TCCR5A |= (1 << COM5C1);
      break;
   case EMode::COAST:
   case EMode::REVERSE:
   case EMode::FORWARD:
   case EMode::BRAKE:
      OCR5C = 0;
      TCCR5A &= ~(1 << COM5C1);
      break;
   }
}

void CDifferentialDriveController::SetRightMotor(EMode e_mode, uint8_t un_duty_cycle) {
   /* Connect the right PWM signal to the motor */
   //sbi(TCCR5A, COM5B1);

}
