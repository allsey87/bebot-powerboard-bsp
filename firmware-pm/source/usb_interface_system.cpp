
#include "usb_interface_system.h"

#include <firmware.h>

/* Two-wire addresses for the modules */
#define GPIO_PWRCFG_ADDR 0x21
#define HUB_CFGMODE_ADDR 0x2D
#define HUB_ACTMODE_ADDR

/* Configuration and power management masks for the USB hub */
#define HS_IND      0x01 
#define CFG_STRAP1  0x02
#define SUSP_IND    0x04
#define CFG_STRAP2  0x08
#define TW_SDA_PU   0x10
#define TW_SCL_PU   0x20
#define TW_INT_EN   0x40
#define RST         0x80

//#define IRQ_PIN     0x01


CUSBInterfaceSystem::CUSBInterfaceSystem() :
   cMCP23008Module(GPIO_PWRCFG_ADDR) {
}

void CUSBInterfaceSystem::Enable() {
   /* Set up the power and configuration GPIO port */
   /* drive the two-wire pull resistors high, other outputs are low */
   uint8_t unPort = 0; // TW_SDA_PU | TW_SCL_PU;
   cMCP23008Module.SetRegister(CMCP23008Module::ERegister::PORT, unPort);
   /* set the direction bits for the outputs to override the pull up resistors*/
   uint8_t unOutputs = CFG_STRAP1 | CFG_STRAP2 | TW_SDA_PU | TW_SCL_PU | TW_INT_EN | RST;
   cMCP23008Module.SetRegister(CMCP23008Module::ERegister::DIRECTION, ~unOutputs);

   /* enable the two-wire interface */
   //unPort |= TW_INT_EN;
   //cMCP23008Module.SetRegister(CMCP23008Module::ERegister::PORT, unPort);
   /* release the hub reset signal */
   unPort |= RST;
   cMCP23008Module.SetRegister(CMCP23008Module::ERegister::PORT, unPort);
   /* Configure the USB2532 */
   //cUSB2532Module.Init();
}

void CUSBInterfaceSystem::Disable() {
   /* leave the TW lines pulled up, but disconnect from the bus and assert reset */
   uint8_t unPort = TW_SDA_PU | TW_SCL_PU;
   cMCP23008Module.SetRegister(CMCP23008Module::ERegister::PORT, unPort);
}

