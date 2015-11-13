#include "firmware.h"

#include <pca9554_module.h>

/***********************************************************/
/***********************************************************/

#define PORTC_SWITCH_IRQ 0x01
#define PORTC_HUB_IRQ 0x02
#define PORTC_SYSTEM_POWER_IRQ 0x04
#define PORTC_ACTUATOR_POWER_IRQ 0x08

/***********************************************************/
/***********************************************************/

#define SYNC_PERIOD 5000
#define HARD_PWDN_PERIOD 750

/***********************************************************/
/***********************************************************/

/* initialisation of the static singleton */
CFirmware CFirmware::_firmware;

/* main function that runs the firmware */
int main(void)
{
   /* FILE structs for fprintf */
   FILE huart;

   /* Set up FILE structs for fprintf */                           
   fdev_setup_stream(&huart, 
                     [](char c_to_write, FILE* pf_stream) {
                        CFirmware::GetInstance().GetHUARTController().Write(c_to_write);
                        return 1;
                     },
                     [](FILE* pf_stream) {
                        return int(CFirmware::GetInstance().GetHUARTController().Read());
                     },
                     _FDEV_SETUP_RW);

   CFirmware::GetInstance().SetFilePointer(&huart);

   /* Execute the firmware */
   CFirmware::GetInstance().Exec();
   /* Terminate */
   return 0;
}

/***********************************************************/
/***********************************************************/

uint8_t CFirmware::GetId() {
   return ~CPCA9554Module<0x20>::GetInstance().GetRegister(CPCA9554Module<0x20>::ERegister::INPUT);
}

/***********************************************************/
/***********************************************************/

CFirmware::CPowerEventInterrupt::CPowerEventInterrupt(CFirmware* pc_firmware, 
                                                      uint8_t un_intr_vect_num) : 
   m_pcFirmware(pc_firmware) {
   Register(this, un_intr_vect_num);
}

/***********************************************************/
/***********************************************************/

void CFirmware::CPowerEventInterrupt::Enable() {
   /* Enable port change interrupts for external events */ 
   PCMSK1 |= ((1 << PCINT8)  | (1 << PCINT9) | 
              (1 << PCINT10) | (1 << PCINT11));
   /* Assume all interrupt sources except the switch could have fired */
   m_unPortLast = ~PINC | PORTC_SWITCH_IRQ;
   /* Enable the port change interrupt group PCINT[14:8] */
   PCICR |= (1 << PCIE1);
   /* Manually execute the service routine once */
   ServiceRoutine();
}

/***********************************************************/
/***********************************************************/

void CFirmware::CPowerEventInterrupt::Disable() {
   /* Disable port change interrupts for external events */ 
   PCMSK1 &= ~((1 << PCINT8)  | (1 << PCINT9) | 
               (1 << PCINT10) | (1 << PCINT11));
   /* Disable the port change interrupt group PCINT[14:8] */
   PCICR &= ~(1 << PCIE1);
}

/***********************************************************/
/***********************************************************/

void CFirmware::CPowerEventInterrupt::ServiceRoutine() {
   uint8_t unPortSnapshot = PINC;
   uint8_t unPortDelta = m_unPortLast ^ unPortSnapshot;

   /* Check power switch state */
   if(unPortDelta & PORTC_SWITCH_IRQ) {
      m_pcFirmware->m_bSwitchSignal = true;
      m_pcFirmware->m_eSwitchState = (unPortSnapshot & PORTC_SWITCH_IRQ) ?
         CFirmware::ESwitchState::RELEASED :
         CFirmware::ESwitchState::PRESSED;
   }
   /* Check USB hub system state */
   if(unPortDelta & PORTC_HUB_IRQ) {
      /* signal is true if it was a falling edge */
      m_pcFirmware->m_bUSBSignal = 
         m_pcFirmware->m_bUSBSignal ||
         ((unPortSnapshot & PORTC_HUB_IRQ) == 0);
   }
   /* Check system power manager state */
   if(unPortDelta & PORTC_SYSTEM_POWER_IRQ) {
      /* signal is true if it was a falling edge */
      m_pcFirmware->m_bSystemPowerSignal = 
         m_pcFirmware->m_bSystemPowerSignal ||
         ((unPortSnapshot & PORTC_SYSTEM_POWER_IRQ) == 0);
   }
   /* Check actuator power manager state */
   if(unPortDelta & PORTC_ACTUATOR_POWER_IRQ) {
      /* signal is true if it was a falling edge */
      m_pcFirmware->m_bActuatorPowerSignal = 
         m_pcFirmware->m_bActuatorPowerSignal ||
         ((unPortSnapshot & PORTC_ACTUATOR_POWER_IRQ) == 0);
   }
   m_unPortLast = unPortSnapshot;
}

/***********************************************************/
/***********************************************************/

void CFirmware::Exec() 
{
   uint32_t unLastSyncTime = 0;
   uint32_t unSwitchPressedTime = 0;
   uint8_t unInput = 0;
   bool bPrintPrompt = true;
   bool bSyncRequiredSignal = false;
  
   m_cPowerManagementSystem.Init();
   m_cPowerEventInterrupt.Enable();

   for(;;) {
      /* Respond to interrupt signals */
      if(m_bSwitchSignal || m_bUSBSignal || m_bSystemPowerSignal || m_bActuatorPowerSignal) {
         fprintf(m_psHUART, "IRQs (0x%02x): ", PINC);
         if(m_bSwitchSignal) {
            m_bSwitchSignal = false;
            if(m_eSwitchState == ESwitchState::PRESSED) {
               unSwitchPressedTime = GetTimer().GetMilliseconds();
               fprintf(m_psHUART, "SW(PRESS)-");
            }
            else {
               fprintf(m_psHUART, "SW(RELEASE)-");
            }
         }
         if(m_bUSBSignal) {
            fprintf(m_psHUART, "USB-");
            m_bUSBSignal = false;
         }
         if(m_bSystemPowerSignal || m_bActuatorPowerSignal) { 
            fprintf(m_psHUART, "%s%s", 
                    m_bSystemPowerSignal ? "PSYS-" : "",
                    m_bActuatorPowerSignal ? "PACT-" : "");
            m_bSystemPowerSignal = false;
            m_bActuatorPowerSignal = false;
            /* assert the sync required signal */
            bSyncRequiredSignal = true;
         }
         fprintf(m_psHUART, "\r\n");
      }

      /* Check if an update is required */
      if((GetTimer().GetMilliseconds() - unLastSyncTime > SYNC_PERIOD) || bSyncRequiredSignal) {
         /* Update last sync time variable */
         unLastSyncTime = GetTimer().GetMilliseconds();
         /* Deassert the sync required signal */
         bSyncRequiredSignal = false;
         /* Run the update loop for the power mangement system */
         m_cPowerManagementSystem.Update();
      }

      if(m_eSwitchState == ESwitchState::PRESSED) {
         if(m_cPowerManagementSystem.IsSystemPowerOn()) {
            if(GetTimer().GetMilliseconds() - unSwitchPressedTime > HARD_PWDN_PERIOD) {
               /* hard power down */
               m_cPowerManagementSystem.SetActuatorPowerOn(false);
               m_cPowerManagementSystem.SetSystemPowerOn(false);
               /* Set m_eSwitchState to ESwitchState::RELEASED indicate that the switch
                  pressed event has been handled */
               m_eSwitchState = ESwitchState::RELEASED;
               /* Assert the sync required signal */
               bSyncRequiredSignal = true;
            }
         }
         else { /* !m_cPowerManagementSystem.IsSystemPowerOn() */
            m_cPowerManagementSystem.SetSystemPowerOn(true);
            /* Set m_eSwitchState to ESwitchState::RELEASED indicate that the switch
               pressed event has been handled */
            m_eSwitchState = ESwitchState::RELEASED;
            /* Assert the sync required signal */
            bSyncRequiredSignal = true;
         }
      }

      if(bPrintPrompt) {
         fprintf(m_psHUART, "\r\nReady> ");
         bPrintPrompt = false;
      }
      
      /* check for input command */
      if(CFirmware::GetInstance().GetHUARTController().Available()) {
         unInput = CFirmware::GetInstance().GetHUARTController().Read();
         /* flush */
         while(CFirmware::GetInstance().GetHUARTController().Available()) {
            CFirmware::GetInstance().GetHUARTController().Read();
         }
      }
      else {
         unInput = 0;
      }

      /* process input command */
      if(unInput != 0) {
         fprintf(m_psHUART, "\r\n");
         switch(unInput) {
         case '0':
            m_cPowerManagementSystem.PrintStatus();
            break;
         case 'E':
            m_cUSBInterfaceSystem.Enable();
            break;
         case 'e':
            m_cUSBInterfaceSystem.Disable();
            break;
         case 'u':
            fprintf(m_psHUART, "Uptime = %lums\r\n", m_cTimer.GetMilliseconds());
            break;
         default:
            fprintf(m_psHUART, "Unknown command: %c\r\n", static_cast<char>(unInput));
            break;
         }
         bPrintPrompt = true;
      }
   }
}

/***********************************************************/
/***********************************************************/

