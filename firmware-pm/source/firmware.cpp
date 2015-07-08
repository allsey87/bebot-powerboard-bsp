#include "firmware.h"

/***********************************************************/
/***********************************************************/

#define PORTC_SWITCH_IRQ 0x01
#define PORTC_HUB_IRQ 0x02
#define PORTC_SYSTEM_POWER_IRQ 0x04
#define PORTC_ACTUATOR_POWER_IRQ 0x08

#define UIS_EN_PIN  0x01
#define UIS_NRST_PIN 0x02

/***********************************************************/
/***********************************************************/

#define SYNC_PERIOD 10000

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
   /* Enable the port change interrupt group PCINT[14:8] */
   PCICR |= (1 << PCIE1);
   /* Assume all interrupt sources could have fired */
   m_unPortLast = ~PINC;
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
         CFirmware::ESwitchState::PRESSED :
         CFirmware::ESwitchState::RELEASED;
   }
   /* Check USB hub system state */
   if(unPortDelta & PORTC_HUB_IRQ) {
      /* signal is true if it was a falling edge */
      m_pcFirmware->m_bUSBSignal = 
         ((unPortSnapshot & PORTC_HUB_IRQ) == 0);
   }
   /* Check system power manager state */
   if(unPortDelta & PORTC_SYSTEM_POWER_IRQ) {
      /* signal is true if it was a falling edge */
      m_pcFirmware->m_bSystemPowerSignal = 
         ((unPortSnapshot & PORTC_SYSTEM_POWER_IRQ) == 0);
   }
   /* Check actuator power manager state */
   if(unPortDelta & PORTC_ACTUATOR_POWER_IRQ) {
      /* signal is true if it was a falling edge */
      m_pcFirmware->m_bActuatorPowerSignal = 
         ((unPortSnapshot & PORTC_ACTUATOR_POWER_IRQ) == 0);
   }
   m_unPortLast = unPortSnapshot;
}

/***********************************************************/
/***********************************************************/

void CFirmware::Exec() 
{
   uint32_t unLastSyncTime = 0;
   uint8_t unInput = 0;
   bool bPrintPrompt = true;

   PORTB &= ~(UIS_NRST_PIN | UIS_EN_PIN); 
   DDRB |= UIS_NRST_PIN | UIS_EN_PIN;

   m_cPowerManagementSystem.Init();

   for(;;) {
      if(m_bSwitchSignal || m_bUSBSignal || m_bSystemPowerSignal || m_bActuatorPowerSignal) {
         fprintf(m_psHUART, "IRQs: ");
         if(m_bSwitchSignal) 
            fprintf(m_psHUART, "SW-");
         if(m_bUSBSignal) 
            fprintf(m_psHUART, "USB-");
         if(m_bSystemPowerSignal) 
            fprintf(m_psHUART, "PSYS-");
         if(m_bActuatorPowerSignal) 
            fprintf(m_psHUART, "PACT-");
         fprintf(m_psHUART, "\r\n");
      }

      /* Check if an update is required */
      if((GetTimer().GetMilliseconds() - unLastSyncTime > SYNC_PERIOD)) {
         /* Update last sync time variable */
         unLastSyncTime = GetTimer().GetMilliseconds();
         /* Run the update loop for the power mangement system */
         m_cPowerManagementSystem.Update();
      }

      if(bPrintPrompt) {
         fprintf(m_psHUART, "Ready> ");
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
         switch(unInput) {
         case 'X':
            m_cPowerEventInterrupt.Enable();
            break;
         case 'x':
            m_cPowerEventInterrupt.Disable();
            break;
         case 'd':
            m_cPowerManagementSystem.PrintStatus();
            break;
         case 'e':
            m_cUSBInterfaceSystem.Disable();
            PORTB &= ~(UIS_NRST_PIN | UIS_EN_PIN);
            break;
         case 'E':
            PORTB |= (UIS_EN_PIN);
            GetTimer().Delay(100);
            PORTB |= (UIS_NRST_PIN);
            GetTimer().Delay(100);
            m_cUSBInterfaceSystem.Enable();
            GetTimer().Delay(100);
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
