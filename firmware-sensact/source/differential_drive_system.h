#ifndef DIFFERENTIAL_DRIVE_SYSTEM_H
#define DIFFERENTIAL_DRIVE_SYSTEM_H

#include <stdint.h>
#include <interrupt.h>

class CDifferentialDriveSystem {
public:
   CDifferentialDriveSystem();

   void SetTargetVelocity(int8_t n_left_speed, int8_t n_right_speed);
   
   int8_t GetLeftVelocity();
   int8_t GetRightVelocity();

   void Enable();
   void Disable();

public:
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
                              
      void Enable();
      void Disable();
   private:
      void ServiceRoutine();
   private:
      CDifferentialDriveSystem* m_pcDifferentialDriveSystem;
      volatile uint8_t m_unPortLast;
   } m_cShaftEncodersInterrupt;

   class CPIDControlStepInterrupt : public CInterrupt {
   public:
      CPIDControlStepInterrupt(CDifferentialDriveSystem* pc_differential_drive_system, 
                               uint8_t un_intr_vect_num);
      void Enable();
      void Disable();
      void SetTargetVelocity(int8_t n_left_speed, int8_t n_right_speed);
   private:
      void ServiceRoutine();
   private:   
      CDifferentialDriveSystem* m_pcDifferentialDriveSystem;      

      int8_t m_nLeftTarget;
      int16_t m_nLeftLastError;
      int16_t m_nLeftErrorIntegral;
      int8_t m_nRightTarget;
      int16_t m_nRightLastError;
      int16_t m_nRightErrorIntegral;
      const uint8_t m_unIntegralSat;
      const uint8_t m_unKp;
      const uint8_t m_unKi;
      const uint8_t m_unKd;     
      const uint8_t m_unScale;
   } m_cPIDControlStepInterrupt;

   friend CShaftEncodersInterrupt;
   friend CPIDControlStepInterrupt;

   /* Actual step count variable */
   volatile int16_t m_nLeftSteps;
   volatile int16_t m_nRightSteps;
   /* Cached step count variable */
   volatile int16_t m_nLeftStepsOut;
   volatile int16_t m_nRightStepsOut;

};

#endif
