
#include "differential_drive_controller.h"

CDifferentialDriveController::CDifferentialDriveController() {
   /* set the drive system initially to coast mode, with the drive disabled */
   PORTL &= ~(LEFT_MODE_PIN  |
              LEFT_DIR_PIN   |
              RIGHT_MODE_PIN |
              RIGHT_DIR_PIN  |
              DRV8833_EN);
   /* set the direction of the output pins to output */
   DDRL |= (LEFT_MODE_PIN  |
            LEFT_DIR_PIN   |
            RIGHT_MODE_PIN |
            RIGHT_DIR_PIN  |
            DRV8833_EN);
   /* set up the timer to generate the PWM signal */
}
   
void CDifferentialDriveController::SetLeftMotor(EMode e_mode, uint8_t un_duty_cycle) {
   switch(e_mode) {
   case EMode::COAST:
      break;
   case EMode::REVERSE:
      break;
   case EMode::REVERSE_PWM_FD:
      break;
   case EMode::REVERSE_PWM_SD:
      break;
   case EMode::FORWARD:
      break;
   case EMode::FORWARD_PWM_FD:
      break;
   case EMode::FORWARD_PWM_SD:
      break;
   case EMode::BREAK:
      break;
   }
}

void CDifferentialDriveController::SetRightMotor(EMode e_mode, uint8_t un_duty_cycle) {

}
