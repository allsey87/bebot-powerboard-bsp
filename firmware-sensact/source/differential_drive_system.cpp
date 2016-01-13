
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
   m_nLeftSteps(0),
   m_nRightSteps(0) {

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
}

/****************************************/
/****************************************/

int8_t CDifferentialDriveSystem::GetLeftVelocity() {
   int16_t nVelocity;
   uint8_t unSREG = SREG;
   cli();
   nVelocity = m_nLeftStepsOut;
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
   nVelocity = m_nRightStepsOut;
   SREG = unSREG;
   /* saturate and return the velocity tn the int8_t range */
   return (nVelocity > INT8_MAX ? INT8_MAX : (nVelocity < INT8_MIN ? INT8_MIN : nVelocity));
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::Enable() {
   /* Enable the shaft encoder interrupt */
   m_cShaftEncodersInterrupt.Enable();
   /* Enable the PID controller interrupt */
   m_cPIDControlStepInterrupt.Enable();
   /* Enable the motor driver */
   PORTB |= (DRV8833_EN);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::Disable() {
   /* Disable the motor driver */
   PORTB &= ~(DRV8833_EN);
   /* Disable the PID controller interrupt */
   m_cPIDControlStepInterrupt.Disable();
   /* Disable the shaft encoder interrupt */
   m_cShaftEncodersInterrupt.Disable();
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::ConfigureLeftMotor(CDifferentialDriveSystem::EBridgeMode e_mode,
                                                  uint8_t un_duty_cycle) {
   switch(e_mode) {
   case EBridgeMode::FORWARD_PWM_FD:
      PORTD |= LEFT_CTRL_PIN;
      PORTD &= ~LEFT_MODE_PIN;
      break;
   case EBridgeMode::FORWARD_PWM_SD:
      un_duty_cycle = uint8_t(UINT8_MAX) - un_duty_cycle;
      PORTD &= ~LEFT_CTRL_PIN;
      PORTD |= LEFT_MODE_PIN;
      break;
   case EBridgeMode::REVERSE_PWM_FD:
      PORTD &= ~LEFT_CTRL_PIN;
      PORTD &= ~LEFT_MODE_PIN;
      break;
   case EBridgeMode::REVERSE_PWM_SD:
      un_duty_cycle = uint8_t(UINT8_MAX) - un_duty_cycle;
      PORTD |= LEFT_CTRL_PIN;
      PORTD |= LEFT_MODE_PIN;
      break;
   case EBridgeMode::COAST:
      PORTD &= ~(LEFT_PWM_PIN | LEFT_CTRL_PIN | LEFT_MODE_PIN);
      break;
   case EBridgeMode::FORWARD:
      PORTD |= LEFT_PWM_PIN;
      PORTD &= ~LEFT_MODE_PIN;
      PORTD |= LEFT_CTRL_PIN;
      break;
   case EBridgeMode::REVERSE:
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
   case EBridgeMode::FORWARD_PWM_FD:
      PORTD &= ~RIGHT_CTRL_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      break;
   case EBridgeMode::FORWARD_PWM_SD:
      un_duty_cycle = uint8_t(UINT8_MAX) - un_duty_cycle;
      PORTD |= RIGHT_CTRL_PIN;
      PORTD |= RIGHT_MODE_PIN;
      break;
   case EBridgeMode::REVERSE_PWM_FD:
      PORTD |= RIGHT_CTRL_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      break;
   case EBridgeMode::REVERSE_PWM_SD:
      un_duty_cycle = uint8_t(UINT8_MAX) - un_duty_cycle;
      PORTD &= ~RIGHT_CTRL_PIN;
      PORTD |= RIGHT_MODE_PIN;
      break;
   case EBridgeMode::COAST:
      PORTD &= ~(RIGHT_PWM_PIN | RIGHT_CTRL_PIN | RIGHT_MODE_PIN);
      break;
   case EBridgeMode::FORWARD:
      PORTD |= RIGHT_PWM_PIN;
      PORTD &= ~RIGHT_MODE_PIN;
      PORTD &= ~RIGHT_CTRL_PIN;
      break;
   case EBridgeMode::REVERSE:
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

CDifferentialDriveSystem::CShaftEncodersInterrupt::CShaftEncodersInterrupt(
   CDifferentialDriveSystem* pc_differential_drive_system,
   uint8_t un_intr_vect_num) :
   m_pcDifferentialDriveSystem(pc_differential_drive_system),
   m_unPortLast(0) {
   Register(this, un_intr_vect_num);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::CShaftEncodersInterrupt::Enable() {
   /* clear variables */
   m_unPortLast = 0;
   m_pcDifferentialDriveSystem->m_nLeftSteps = 0;
   m_pcDifferentialDriveSystem->m_nRightSteps = 0;
   /* enable interrupt */
   PCICR |= (1 << PCIE1);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::CShaftEncodersInterrupt::Disable() {
   /* disable interrupt */
   PCICR &= ~(1 << PCIE1);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::CShaftEncodersInterrupt::ServiceRoutine() {
   uint8_t unPortSnapshot = PINC;
   uint8_t unPortDelta = m_unPortLast ^ unPortSnapshot;
   /* This intermediate value determines whether the motors are moving
      backwards or forwards. The expression uses Reed-Muller logic and
      leverages the symmetry of the encoder inputs */
   uint8_t unIntermediate = (~unPortSnapshot) ^ (m_unPortLast >> 1);
   /* check the left encoder */
   if(unPortDelta & (ENC_LEFT_CHA | ENC_LEFT_CHB)) {
      if(unIntermediate & ENC_LEFT_CHA) {
         m_pcDifferentialDriveSystem->m_nLeftSteps--;
      }
      else {
         m_pcDifferentialDriveSystem->m_nLeftSteps++;
      }
   }
   /* check the right encoder */
   if(unPortDelta & (ENC_RIGHT_CHA | ENC_RIGHT_CHB)) {
      if(unIntermediate & ENC_RIGHT_CHA) {
         m_pcDifferentialDriveSystem->m_nRightSteps++;
      }
      else {
         m_pcDifferentialDriveSystem->m_nRightSteps--;
      }
   }
   m_unPortLast = unPortSnapshot;
}

/****************************************/
/****************************************/

CDifferentialDriveSystem::CPIDControlStepInterrupt::CPIDControlStepInterrupt(
   CDifferentialDriveSystem* pc_differential_drive_system,
   uint8_t un_intr_vect_num) :
   m_pcDifferentialDriveSystem(pc_differential_drive_system),
   m_nLeftTarget(0),
   m_nLeftLastError(0),
   m_nLeftErrorIntegral(0),
   m_nRightTarget(0),
   m_nRightLastError(0),
   m_nRightErrorIntegral(0),
   m_unIntegralSat(64),
   m_unKp(1),
   m_unKi(2),
   m_unKd(1),
   m_unScale(1) {
   Register(this, un_intr_vect_num);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::CPIDControlStepInterrupt::Enable() {
   /* clear intermediate variables */
   m_nLeftLastError = 0;
   m_nLeftErrorIntegral = 0;
   m_nRightLastError = 0;
   m_nRightErrorIntegral = 0;
   /* enable interrupt */
   TIMSK0 |= (1 << TOIE0);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::CPIDControlStepInterrupt::Disable() {
   /* disable interrupt */
   TIMSK0 &= ~(1 << TOIE0);
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::CPIDControlStepInterrupt::SetTargetVelocity(int8_t n_left_speed, 
                                                                           int8_t n_right_speed) {
   m_nLeftTarget = n_left_speed;
   m_nRightTarget = n_right_speed;
}

/****************************************/
/****************************************/

void CDifferentialDriveSystem::CPIDControlStepInterrupt::ServiceRoutine() {
   /* Calculate left PID intermediates */
   int16_t nLeftError = int16_t(m_nLeftTarget) - m_pcDifferentialDriveSystem->m_nLeftSteps;
   /* Accumulate and saturate the integral component */
   m_nLeftErrorIntegral += nLeftError;
   m_nLeftErrorIntegral = m_nLeftErrorIntegral > int16_t(m_unIntegralSat) ? int16_t(m_unIntegralSat) :
      (m_nLeftErrorIntegral < -int16_t(m_unIntegralSat) ? -int16_t(m_unIntegralSat) : m_nLeftErrorIntegral);
   int16_t nLeftErrorDerivative = (nLeftError - m_nLeftLastError);
   m_nLeftLastError = nLeftError;
   /* Calculate output value */
   int16_t nLeftOutput =
      m_unKp * nLeftError +
      m_unKi * m_nLeftErrorIntegral +
      m_unKd * nLeftErrorDerivative;
   /* Scale the output value */   
   nLeftOutput /= m_unScale;
   /* Saturate the output magnitude between 0 and UINT8_MAX in the direction of travel */
   uint8_t unLeftOutputMagnitude = 0;
   if((nLeftOutput < 0) && (m_nLeftTarget < 0)) {
      unLeftOutputMagnitude = (nLeftOutput < -int16_t(UINT8_MAX)) ? uint8_t(UINT8_MAX) : uint8_t(-nLeftOutput);
   }
   else if((nLeftOutput >= 0) && (m_nLeftTarget >= 0)) {
      unLeftOutputMagnitude = (nLeftOutput > int16_t(UINT8_MAX)) ? uint8_t(UINT8_MAX) : uint8_t(nLeftOutput);
   }
   else {
      unLeftOutputMagnitude = 0;
   }
   /* Calculate right PID intermediates */
   int16_t nRightError = int16_t(m_nRightTarget) - m_pcDifferentialDriveSystem->m_nRightSteps;
   /* Accumulate and saturate the integral component */
   m_nRightErrorIntegral += nRightError;
   m_nRightErrorIntegral = m_nRightErrorIntegral > int16_t(m_unIntegralSat) ? int16_t(m_unIntegralSat) :
      (m_nRightErrorIntegral < -int16_t(m_unIntegralSat) ? -int16_t(m_unIntegralSat) : m_nRightErrorIntegral);
   int16_t nRightErrorDerivative = (nRightError - m_nRightLastError);
   m_nRightLastError = nRightError;
   /* Calculate output value */
   int16_t nRightOutput =
      m_unKp * nRightError +
      m_unKi * m_nRightErrorIntegral +
      m_unKd * nRightErrorDerivative;
   /* Scale the output value */
   nRightOutput /= m_unScale;
   /* Saturate the output magnitude between 0 and UINT8_MAX in the direction of travel */
   uint8_t unRightOutputMagnitude = 0;
   if((nRightOutput < 0) && (m_nRightTarget < 0)) {
      unRightOutputMagnitude = (nRightOutput < -int16_t(UINT8_MAX)) ? uint8_t(UINT8_MAX) : uint8_t(-nRightOutput);
   }
   else if((nRightOutput >= 0) && (m_nRightTarget >= 0)) {
      unRightOutputMagnitude = (nRightOutput > int16_t(UINT8_MAX)) ? uint8_t(UINT8_MAX) : uint8_t(nRightOutput);
   }
   else {
      unRightOutputMagnitude = 0;
   }
   /* Update right motor */ 
   if(unRightOutputMagnitude == 0) {
      m_pcDifferentialDriveSystem->ConfigureRightMotor(CDifferentialDriveSystem::EBridgeMode::COAST);
   }
   else {
      m_pcDifferentialDriveSystem->ConfigureRightMotor(
         (nRightOutput < 0) ? CDifferentialDriveSystem::EBridgeMode::REVERSE_PWM_FD :
                              CDifferentialDriveSystem::EBridgeMode::FORWARD_PWM_FD,
         unRightOutputMagnitude);
   }
   /* Update left motor */
   if(unLeftOutputMagnitude == 0) {
      m_pcDifferentialDriveSystem->ConfigureLeftMotor(CDifferentialDriveSystem::EBridgeMode::COAST);
   }
   else {
      m_pcDifferentialDriveSystem->ConfigureLeftMotor(
         (nLeftOutput < 0) ? CDifferentialDriveSystem::EBridgeMode::REVERSE_PWM_FD :
                             CDifferentialDriveSystem::EBridgeMode::FORWARD_PWM_FD,
         unLeftOutputMagnitude);
   } 
   /* copy the step counters for velocity measurements */
   m_pcDifferentialDriveSystem->m_nRightStepsOut = m_pcDifferentialDriveSystem->m_nRightSteps;
   m_pcDifferentialDriveSystem->m_nLeftStepsOut = m_pcDifferentialDriveSystem->m_nLeftSteps; 
   /* clear the step counters */
   m_pcDifferentialDriveSystem->m_nRightSteps = 0;
   m_pcDifferentialDriveSystem->m_nLeftSteps = 0;
}

/****************************************/
/****************************************/









