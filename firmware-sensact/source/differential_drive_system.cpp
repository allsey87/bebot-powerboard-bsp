
#include "differential_drive_system.h"

#include <firmware.h>

/* Port B Pins - Power and faults */
#define DRV8833_EN     0x01
#define DRV8833_FAULT  0x02

/* Port C Pins - Encoder input */
#define ENC_RIGHT_CHA  0x01
#define ENC_RIGHT_CHB  0x02
#define ENC_LEFT_CHA   0x04
#define ENC_LEFT_CHB   0x08

/* Port D Pins - Motor output */
#define LEFT_CTRL_PIN  0x04
#define RIGHT_CTRL_PIN 0x08
#define RIGHT_MODE_PIN 0x10
#define RIGHT_PWM_PIN  0x20
#define LEFT_PWM_PIN   0x40
#define LEFT_MODE_PIN  0x80

/****************************************/
/****************************************/


CDifferentialDriveSystem::CDifferentialDriveSystem() :
   m_cShaftEncodersInterrupt(this, PCINT1_vect_num),
   m_cPIDControlStepInterrupt(this, TIMER0_OVF_vect_num),
   nLeftSteps(0),
   nRightSteps(0) {

   /* Initialise pins in a disabled, coasting state */
   PORTB &= ~(DRV8833_EN);
   PORTD &= ~(LEFT_MODE_PIN  |
              LEFT_CTRL_PIN  |
              LEFT_PWM_PIN   |
              RIGHT_MODE_PIN |
              RIGHT_CTRL_PIN |
              RIGHT_PWM_PIN);
   /* set the direction of the output pins to output */
   DDRB |= (DRV8833_EN);
   DDRD |= (LEFT_MODE_PIN  |
            LEFT_CTRL_PIN  |
            LEFT_PWM_PIN   |
            RIGHT_MODE_PIN |
            RIGHT_CTRL_PIN |
            RIGHT_PWM_PIN);

   /* Setup up timer 0 for PWM */
   /* Select phase correct, non-inverting PWM mode on channel A & B */
   TCCR0A |= (1 << WGM00);
   /* Select precaler to 64 */
   TCCR0B |= ((1 << CS01) | (1 << CS00));

   /* Initialize left and right motor duty cycle to zero */
   OCR0A = 0;
   OCR0B = 0;

   /* Enable port change interrupts for right encoder A/B 
      and left encoder A/B respectively */
   PCMSK1 |= (1 << PCINT8)  | (1 << PCINT9) | 
             (1 << PCINT10) | (1 << PCINT11);
   /* Enable the port change interrupt group PCINT[14:8] */
   PCICR |= (1 << PCIE1);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::GetVelocity() {
   uint16_t input, target, output;
   uint8_t unSREG = SREG;
   cli();
   input = tempSteps;
   output = tempOut;
   SREG = unSREG;
   target = m_cPIDControlStepInterrupt.nRightTarget;
   fprintf(Firmware::GetInstance().m_psHUART,
           "T:%4d I:%4d E:%4d O:%4d\r\n",
           target,
           input,
           target - input,
           output);

}

/****************************************/
/****************************************/


void CDifferentialDriveSystem::Enable() {
   /* Enable the motor driver */
   PORTB |= (DRV8833_EN);
   /* Enable the timer overflow interrupt for the controller */
   TIMSK0 |= (1 << TOIE0);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::Disable() {
   /* Disable the motor driver */
   PORTB &= ~(DRV8833_EN);
   /* Disable the timer overflow interrupt for the controller */
   TIMSK0 &= ~(1 << TOIE0);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::ConfigureLeftMotor(CDifferentialDriveSystem::EMode e_mode,
                                                  uint8_t un_duty_cycle) {
   switch(e_mode) {
   case EMode::REVERSE_PWM_FD:
      PORTD |= LEFT_CTRL_PIN;
      PORTD &= ~LEFT_MODE_PIN; //
      break;
   case EMode::REVERSE_PWM_SD:
      un_duty_cycle = 0xFF - un_duty_cycle;
      PORTD &= ~LEFT_CTRL_PIN;
      PORTD |= LEFT_MODE_PIN; //
      break;
   case EMode::FORWARD_PWM_FD:
      PORTD &= ~LEFT_CTRL_PIN;
      PORTD &= ~LEFT_MODE_PIN; //
      break;
   case EMode::FORWARD_PWM_SD:
      un_duty_cycle = 0xFF - un_duty_cycle;
      PORTD |= LEFT_CTRL_PIN;
      PORTD |= LEFT_MODE_PIN; //
      break;
   case EMode::COAST:
      PORTD &= ~(LEFT_PWM_PIN | LEFT_CTRL_PIN | LEFT_MODE_PIN);
      break;
   case EMode::REVERSE:
      PORTD |= LEFT_PWM_PIN;
      PORTD &= ~LEFT_MODE_PIN;
      PORTD |= LEFT_CTRL_PIN;
      break;
   case EMode::FORWARD:
      PORTD |= LEFT_PWM_PIN;
      PORTD &= ~LEFT_MODE_PIN;
      PORTD &= ~LEFT_CTRL_PIN;
      break;
   case EMode::BRAKE:
      PORTD |= (LEFT_PWM_PIN | LEFT_CTRL_PIN | LEFT_MODE_PIN);
      break;
   }

   if(e_mode == EMode::COAST   || 
      e_mode == EMode::REVERSE || 
      e_mode == EMode::FORWARD || 
      e_mode == EMode::BRAKE) {
      /* disconnect PWM from the output pin */
      OCR0A = 0;
      TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));
   }
   else {
      /* connect PWM to the output pin */
      OCR0A = un_duty_cycle;
      TCCR0A |= (1 << COM0A1);
   }
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::ConfigureRightMotor(CDifferentialDriveSystem::EMode e_mode,
                                                   uint8_t un_duty_cycle) {
   switch(e_mode) {
   case EMode::REVERSE_PWM_FD:
      PORTD &= ~RIGHT_CTRL_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      break;
   case EMode::REVERSE_PWM_SD:
      un_duty_cycle = 0xFF - un_duty_cycle;
      PORTD |= RIGHT_CTRL_PIN;
      PORTD |= RIGHT_MODE_PIN;
      break;
   case EMode::FORWARD_PWM_FD:
      PORTD |= RIGHT_CTRL_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      break;
   case EMode::FORWARD_PWM_SD:
      un_duty_cycle = 0xFF - un_duty_cycle;
      PORTD &= ~RIGHT_CTRL_PIN;
      PORTD |= RIGHT_MODE_PIN;
      break;
   case EMode::COAST:
      PORTD &= ~(RIGHT_PWM_PIN | RIGHT_CTRL_PIN | RIGHT_MODE_PIN);
      break;
   case EMode::REVERSE:
      PORTD |= RIGHT_PWM_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      PORTD &= ~RIGHT_CTRL_PIN;
      break;
   case EMode::FORWARD:
      PORTD |= RIGHT_PWM_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      PORTD |= RIGHT_CTRL_PIN;
      break;
   case EMode::BRAKE:
      PORTD |= (RIGHT_PWM_PIN | RIGHT_CTRL_PIN | RIGHT_MODE_PIN);
      break;
   }

   if(e_mode == EMode::COAST   || 
      e_mode == EMode::REVERSE || 
      e_mode == EMode::FORWARD || 
      e_mode == EMode::BRAKE) {
      /* disconnect PWM from the output pin */
      OCR0B = 0;
      TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0));
   }
   else {
      /* connect PWM to the output pin */
      OCR0B = un_duty_cycle;
      TCCR0A |= (1 << COM0B1);
   }
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::CShaftEncodersInterrupt::ServiceRoutine() {
   uint8_t unPortSnapshot = PINC;
   uint8_t unPortDelta = unPortLast ^ unPortSnapshot;
   /* This intermediate value determines whether the motors are moving
      backwards or forwards. The expression uses Reed-Muller logic and 
      leverages the symmetry of the encoder inputs */
   uint8_t unIntermediate = (~unPortSnapshot) ^ (unPortLast >> 1);
   /* check the left encoder */
   if(unPortDelta & (ENC_LEFT_CHA | ENC_LEFT_CHB)) {
      if(unIntermediate & ENC_LEFT_CHA) {
         m_pcDifferentialDriveSystem->nLeftSteps++;
      }
      else {
         m_pcDifferentialDriveSystem->nLeftSteps--;
      }
   }
   /* check the right encoder */
   if(unPortDelta & (ENC_RIGHT_CHA | ENC_RIGHT_CHB)) {
      if(unIntermediate & ENC_RIGHT_CHA) {
         m_pcDifferentialDriveSystem->nRightSteps--;
      }
      else {
         m_pcDifferentialDriveSystem->nRightSteps++;
      }
   }
   unPortLast = unPortSnapshot;
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::CPIDControlStepInterrupt::ServiceRoutine() {
   /* Calculate left PID intermediates */
   int16_t nLeftError = nLeftTarget - m_pcDifferentialDriveSystem->nLeftSteps;
   //nLeftIntegral += nLeftError * unDt; // overflow?
   //int16_t nLeftDerivative = (nLeftError - nLeftLastError) / unDt; // rounding?
   //nLeftLastError = nLeftError;
   /* Calculate output value */
   int16_t nLeftOutput = 
      nKp * nLeftError; /* +
      nKi * nLeftIntegral +
      nKd * nLeftDerivative;*/



   /* Calculate right PID intermediates */   
   int16_t nRightError = nRightTarget - m_pcDifferentialDriveSystem->nRightSteps;

   // DEBUG
   m_pcDifferentialDriveSystem->tempSteps = m_pcDifferentialDriveSystem->nRightSteps;

   //nRightIntegral += nRightError * unDt; // overflow?
   //int16_t nRightDerivative = (nRightError - nRightLastError) / unDt; // rounding?
   //nRightLastError = nRightError;
   /* Calculate output value */
   int16_t nRightOutput = 
      nKp * nRightError; /* +
      nKi * nRightIntegral +
      nKd * nRightDerivative;   */

   //nRightOutput = nRightOutput > 0xFF ? 0xFF : nRightOutput;

   // DEBUG
   m_pcDifferentialDriveSystem->tempOut = nRightOutput;

   
   /* Update motors 
   m_pcDifferentialDriveSystem->ConfigureLeftMotor(nLeftOutput < 0 ?
                                                   CDifferentialDriveSystem::EMode::REVERSE_PWM_FD :
                                                   CDifferentialDriveSystem::EMode::FORWARD_PWM_FD,
                                                   nLeftOutput < 0 ? -nLeftOutput : nLeftOutput); */
   m_pcDifferentialDriveSystem->ConfigureRightMotor(nRightOutput < 0 ?
                                                   CDifferentialDriveSystem::EMode::REVERSE_PWM_FD :
                                                   CDifferentialDriveSystem::EMode::FORWARD_PWM_FD,
                                                   nRightOutput < 0 ? -nRightOutput : nRightOutput);
   
   /* clear the step counters */
   m_pcDifferentialDriveSystem->nRightSteps = 0;
   m_pcDifferentialDriveSystem->nLeftSteps = 0;
}

/****************************************/
/****************************************/

CDifferentialDriveSystem::CShaftEncodersInterrupt::CShaftEncodersInterrupt(
   CDifferentialDriveSystem* pc_differential_drive_system, 
   uint8_t un_intr_vect_num) : 
   m_pcDifferentialDriveSystem(pc_differential_drive_system),
   unPortLast(0) {
   Register(this, un_intr_vect_num);
}

/****************************************/
/****************************************/

/* 

   8MHz, x64 prescaler, 510 steps
   unDt 4.08ms
   

   
   output must be mapped to -0xFF to +0xFF

 */

CDifferentialDriveSystem::CPIDControlStepInterrupt::CPIDControlStepInterrupt(
   CDifferentialDriveSystem* pc_differential_drive_system, 
   uint8_t un_intr_vect_num) : 
   m_pcDifferentialDriveSystem(pc_differential_drive_system),
   nLeftTarget(0),
   nLeftLastError(0),
   nLeftIntegral(0),
   nRightTarget(0),
   nRightLastError(0),
   nRightIntegral(0),

   nKp(2),
   nKi(15),
   nKd(1),
   unDt(4) {
   Register(this, un_intr_vect_num);
}

/****************************************/
/****************************************/








