
#include "differential_drive_controller.h"

#include <avr/interrupt.h>

#define MOVING_AVERAGE_LENGTH 5
/* Note: TIMER_NUM_OVERFLOWS 60 => 244.8ms for 8MHz with 64 prescaler
   in 8-bit phase correct PWM mode */
#define TIMER_OVERFLOWS_PERSAMPLE 60
#define SAMPLES_PER_MINUTE 245
/* TODO: Find out the actual value of STEPS_PER_REV. Current value was
   determined empirically! */
#define STEPS_PER_REV 7424

volatile uint8_t unPortSnapshot = 0x00;
volatile uint8_t unPortLast = PINB;

volatile int16_t nLeftSteps[MOVING_AVERAGE_LENGTH] = {};
volatile int16_t nRightSteps[MOVING_AVERAGE_LENGTH] = {};

volatile uint8_t unWindowIdx = 0;
volatile uint8_t unTimerOverflowCnt = 0;

ISR(PCINT0_vect) {
   unPortSnapshot = PINB;
   uint8_t unPortDelta = unPortLast ^ unPortSnapshot;
   /* This intermediate value determines whether the motors are moving
      backwards or forwards. The expression uses Reed-Muller logic and 
      leverages the symmetry of the encoder inputs */
   uint8_t unIntermediate = (~unPortSnapshot) ^ (unPortLast << 1);
   /* check the left encoder */
   if(unPortDelta & LEFT_ENC_MASK) {
      if(unIntermediate & LEFT_ENC_CHA_PIN) {
         nLeftSteps[unWindowIdx]++;
      }
      else {
         nLeftSteps[unWindowIdx]--;
      }
   }
   /* check the right encoder */
   if(unPortDelta & RIGHT_ENC_MASK) {
      if(unIntermediate & RIGHT_ENC_CHA_PIN) {
         nRightSteps[unWindowIdx]--;
      }
      else {
         nRightSteps[unWindowIdx]++;
      }
   }
   unPortLast = unPortSnapshot;
}


ISR(TIMER5_OVF_vect) {
   if(unTimerOverflowCnt < TIMER_OVERFLOWS_PERSAMPLE) {
      unTimerOverflowCnt++;
   }
   else {
      unTimerOverflowCnt = 0;
      unWindowIdx = (unWindowIdx + 1) % MOVING_AVERAGE_LENGTH;
      nLeftSteps[unWindowIdx] = 0;
      nRightSteps[unWindowIdx] = 0;
   }
}

int16_t CDifferentialDriveController::GetLeftRPM() {
   uint8_t unSREG = SREG;
   int32_t nTotalSteps = 0;
   cli();
   for(uint8_t unIdx = 0; unIdx < MOVING_AVERAGE_LENGTH; unIdx++) {
      nTotalSteps += (unIdx != unWindowIdx) ? nLeftSteps[unIdx] : 0;
   }
   SREG = unSREG;
   return (nTotalSteps * SAMPLES_PER_MINUTE) / ((MOVING_AVERAGE_LENGTH - 1) * STEPS_PER_REV);
}

int16_t CDifferentialDriveController::GetRightRPM() {
   uint8_t unSREG = SREG;
   int32_t nTotalSteps = 0;
   cli();
   for(uint8_t unIdx = 0; unIdx < MOVING_AVERAGE_LENGTH; unIdx++) {
      nTotalSteps += (unIdx != unWindowIdx) ? nRightSteps[unIdx] : 0;
   }
   SREG = unSREG;
   return (nTotalSteps * SAMPLES_PER_MINUTE) / ((MOVING_AVERAGE_LENGTH - 1) * STEPS_PER_REV);
}


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

   /* Encoder init logic */
   /* Enable port change interrupts on PCINT4, PCINT5 (left) and PCINT6, PCINT7 (right) */
   PCMSK0 |= ((1 << PCINT4) | (1 << PCINT5) | (1 << PCINT6) | (1 << PCINT7));
   /* Enable the port change interrupt group PCINT0-7 */
   PCICR |= (1 << PCIE0);


   /* Enable the overflow interrupt on timer 5 (set at BOTTOM) */
   TIMSK5 |= (1 << TOIE5);

   /* Debugging only*/
   //DDRD |= STATUS_LED;
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
