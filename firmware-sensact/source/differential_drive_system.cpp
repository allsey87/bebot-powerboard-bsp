
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

int8_t CDifferentialDriveSystem::GetLeftVelocity() {
   int16_t nVelocity;
   uint8_t unSREG = SREG;
   cli();
   nVelocity = nLeftStepsOut;
   SREG = unSREG;
   /* saturate and return the velocity tn the int8_t range */
   return (nVelocity > INT8_MAX ? INT8_MAX : (nVelocity < INT8_MIN ? INT8_MIN : nVelocity));
}

/****************************************/
/****************************************/

int8_t CDifferentialDriveSystem::GetRightVelocity() {
   int16_t nVelocity;
   uint8_t unSREG = SREG;
   cli();
   nVelocity = nRightStepsOut;
   SREG = unSREG;
   /* saturate and return the velocity tn the int8_t range */
   return (nVelocity > INT8_MAX ? INT8_MAX : (nVelocity < INT8_MIN ? INT8_MIN : nVelocity));
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

void CDifferentialDriveSystem::ConfigureLeftMotor(CDifferentialDriveSystem::EBridgeMode e_mode,
                                                  uint8_t un_duty_cycle) {
   switch(e_mode) {
   case EBridgeMode::REVERSE_PWM_FD:
      PORTD |= LEFT_CTRL_PIN;
      PORTD &= ~LEFT_MODE_PIN; //
      break;
   case EBridgeMode::REVERSE_PWM_SD:
      un_duty_cycle = 0xFF - un_duty_cycle;
      PORTD &= ~LEFT_CTRL_PIN;
      PORTD |= LEFT_MODE_PIN; //
      break;
   case EBridgeMode::FORWARD_PWM_FD:
      PORTD &= ~LEFT_CTRL_PIN;
      PORTD &= ~LEFT_MODE_PIN; //
      break;
   case EBridgeMode::FORWARD_PWM_SD:
      un_duty_cycle = 0xFF - un_duty_cycle;
      PORTD |= LEFT_CTRL_PIN;
      PORTD |= LEFT_MODE_PIN; //
      break;
   case EBridgeMode::COAST:
      PORTD &= ~(LEFT_PWM_PIN | LEFT_CTRL_PIN | LEFT_MODE_PIN);
      break;
   case EBridgeMode::REVERSE:
      PORTD |= LEFT_PWM_PIN;
      PORTD &= ~LEFT_MODE_PIN;
      PORTD |= LEFT_CTRL_PIN;
      break;
   case EBridgeMode::FORWARD:
      PORTD |= LEFT_PWM_PIN;
      PORTD &= ~LEFT_MODE_PIN;
      PORTD &= ~LEFT_CTRL_PIN;
      break;
   case EBridgeMode::BRAKE:
      PORTD |= (LEFT_PWM_PIN | LEFT_CTRL_PIN | LEFT_MODE_PIN);
      break;
   }

   if(e_mode == EBridgeMode::COAST   || 
      e_mode == EBridgeMode::REVERSE || 
      e_mode == EBridgeMode::FORWARD || 
      e_mode == EBridgeMode::BRAKE) {
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

void CDifferentialDriveSystem::ConfigureRightMotor(CDifferentialDriveSystem::EBridgeMode e_mode,
                                                   uint8_t un_duty_cycle) {
   switch(e_mode) {
   case EBridgeMode::REVERSE_PWM_FD:
      PORTD &= ~RIGHT_CTRL_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      break;
   case EBridgeMode::REVERSE_PWM_SD:
      un_duty_cycle = 0xFF - un_duty_cycle;
      PORTD |= RIGHT_CTRL_PIN;
      PORTD |= RIGHT_MODE_PIN;
      break;
   case EBridgeMode::FORWARD_PWM_FD:
      PORTD |= RIGHT_CTRL_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      break;
   case EBridgeMode::FORWARD_PWM_SD:
      un_duty_cycle = 0xFF - un_duty_cycle;
      PORTD &= ~RIGHT_CTRL_PIN;
      PORTD |= RIGHT_MODE_PIN;
      break;
   case EBridgeMode::COAST:
      PORTD &= ~(RIGHT_PWM_PIN | RIGHT_CTRL_PIN | RIGHT_MODE_PIN);
      break;
   case EBridgeMode::REVERSE:
      PORTD |= RIGHT_PWM_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      PORTD &= ~RIGHT_CTRL_PIN;
      break;
   case EBridgeMode::FORWARD:
      PORTD |= RIGHT_PWM_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      PORTD |= RIGHT_CTRL_PIN;
      break;
   case EBridgeMode::BRAKE:
      PORTD |= (RIGHT_PWM_PIN | RIGHT_CTRL_PIN | RIGHT_MODE_PIN);
      break;
   }

   if(e_mode == EBridgeMode::COAST   || 
      e_mode == EBridgeMode::REVERSE || 
      e_mode == EBridgeMode::FORWARD || 
      e_mode == EBridgeMode::BRAKE) {
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
   int16_t nLeftError = int16_t(nLeftTarget) - m_pcDifferentialDriveSystem->nLeftSteps;
   nLeftErrorIntegral += nLeftError;
   int16_t nLeftErrorDerivative = (nLeftError - nLeftLastError);
   nLeftLastError = nLeftError;
   /* Calculate output value */
   int16_t nLeftOutput = 
      nKp * nLeftError +
      nKi * nLeftErrorIntegral +
      nKd * nLeftErrorDerivative;
   /* Saturation the output value at +/-100% duty cycle */
   nLeftOutput = nLeftOutput > 0xFF ? 0xFF :
      nLeftOutput < -0xFF ? -0xFF : nLeftOutput;
   /* Calculate right PID intermediates */   
   int16_t nRightError = int16_t(nRightTarget) - m_pcDifferentialDriveSystem->nRightSteps;
   nRightErrorIntegral += nRightError;
   int16_t nRightErrorDerivative = (nRightError - nRightLastError);
   nRightLastError = nRightError;
   /* Calculate output value */
   int16_t nRightOutput = 
      nKp * nRightError +
      nKi * nRightErrorIntegral +
      nKd * nRightErrorDerivative;
   /* Saturation the output value at +/-100% duty cycle */
   nRightOutput = nRightOutput > 0xFF ? 0xFF :
      nRightOutput < -0xFF ? -0xFF : nRightOutput;  
   /* Update motors */
   m_pcDifferentialDriveSystem->ConfigureLeftMotor(
      nLeftOutput < 0 ? CDifferentialDriveSystem::EBridgeMode::REVERSE_PWM_FD :
                        CDifferentialDriveSystem::EBridgeMode::FORWARD_PWM_FD,
      nLeftOutput < 0 ? -nLeftOutput : nLeftOutput);
   m_pcDifferentialDriveSystem->ConfigureRightMotor(
      nRightOutput < 0 ? CDifferentialDriveSystem::EBridgeMode::REVERSE_PWM_FD :
                         CDifferentialDriveSystem::EBridgeMode::FORWARD_PWM_FD,
      nRightOutput < 0 ? -nRightOutput : nRightOutput);
   /* copy the step counters for velocity measurements */
   m_pcDifferentialDriveSystem->nRightStepsOut = m_pcDifferentialDriveSystem->nRightSteps;
   m_pcDifferentialDriveSystem->nLeftStepsOut = m_pcDifferentialDriveSystem->nLeftSteps;
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

CDifferentialDriveSystem::CPIDControlStepInterrupt::CPIDControlStepInterrupt(
   CDifferentialDriveSystem* pc_differential_drive_system, 
   uint8_t un_intr_vect_num) : 
   m_pcDifferentialDriveSystem(pc_differential_drive_system),
   nLeftTarget(0),
   nLeftLastError(0),
   nLeftErrorIntegral(0),
   nRightTarget(0),
   nRightLastError(0),
   nRightErrorIntegral(0),
   nKp(3),
   nKi(1),
   nKd(4) {
   Register(this, un_intr_vect_num);
}

/****************************************/
/****************************************/








