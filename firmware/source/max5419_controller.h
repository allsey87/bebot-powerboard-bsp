#ifndef MAX5419_CONTROLLER_H
#define MAX5419_CONTROLLER_H

#include <stdint.h>

class CMAX5419Controller {
public:

   /**
    * Constructors, stores the address of the target device
    * @param un_addr the i2c address of the target device
    */   
   CMAX5419Controller(uint8_t un_addr) :
      m_unAddr(un_addr) {}

   /**
    * Sets the current resistance of the device
    * @param un_val the value to put in the volatile register
    * of the max5419, this value immediately takes effect
    */   
   void SetActualValue(uint8_t un_val);

   /**
    * Sets the stored resistance of the device
    * @param un_val the value to put in the non-volatile register
    * of the max5419, this value has no effect until the device
    * is reset
    */   
   void SetStoredValue(uint8_t un_val);

   /**
    * Sets the resistance to the stored resistance in the device
    */   
   void Reset();

private:

   uint8_t m_unAddr;

};

#endif
