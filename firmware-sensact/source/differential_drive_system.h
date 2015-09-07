#ifndef DIFFERENTIAL_DRIVE_SYSTEM_H
#define DIFFERENTIAL_DRIVE_SYSTEM_H

#include <stdint.h>
#include <interrupt.h>

#define MOVING_AVERAGE_LENGTH 1

class CDifferentialDriveSystem {
public:
   CDifferentialDriveSystem();

   void SetTargetVelocity(int8_t n_left_speed, int8_t n_right_speed) {
      m_cPIDControlStepInterrupt.SetTargetVelocity(n_left_speed, n_right_speed);
   }

   int8_t GetLeftVelocity();
   int8_t GetRightVelocity();

   void Enable();
   void Disable();

   /* To be made private */
  
private:
   enum class EBridgeMode {
      COAST,
      REVERSE,
      REVERSE_PWM_FD,
      REVERSE_PWM_SD,
      FORWARD,
      FORWARD_PWM_FD,
      FORWARD_PWM_SD,
      BRAKE
   };

   void ConfigureLeftMotor(EBridgeMode e_mode, uint8_t un_duty_cycle = 0);
   void ConfigureRightMotor(EBridgeMode e_mode, uint8_t un_duty_cycle = 0);

   class CShaftEncodersInterrupt : public CInterrupt {
   public:
      CShaftEncodersInterrupt(CDifferentialDriveSystem* pc_differential_drive_system, 
                              uint8_t un_intr_vect_num);
   private:  
      CDifferentialDriveSystem* m_pcDifferentialDriveSystem;     
      void ServiceRoutine();
      volatile uint8_t unPortLast;
   } m_cShaftEncodersInterrupt;

   class CPIDControlStepInterrupt : public CInterrupt {
   public:
      CPIDControlStepInterrupt(CDifferentialDriveSystem* pc_differential_drive_system, 
                               uint8_t un_intr_vect_num);
      void SetTargetVelocity(int8_t n_left_speed, int8_t n_right_speed) {
         nLeftTarget = n_left_speed;
         nRightTarget = n_right_speed;
      }
   private:     
      CDifferentialDriveSystem* m_pcDifferentialDriveSystem;     
      void ServiceRoutine();
   public:
      int8_t nLeftTarget;
      int16_t nLeftLastError;
      int16_t nLeftErrorIntegral;
      int8_t nRightTarget;
      int16_t nRightLastError;
      int16_t nRightErrorIntegral;

      int16_t nKp;
      int16_t nKi;
      int16_t nKd;
   } m_cPIDControlStepInterrupt;

   friend CShaftEncodersInterrupt;
   friend CPIDControlStepInterrupt;

   /* Actual step count variable */
   volatile int16_t nLeftSteps;
   volatile int16_t nRightSteps;
   /* Cached step count variable */
   volatile int16_t nLeftStepsOut;
   volatile int16_t nRightStepsOut;

};

#endif
