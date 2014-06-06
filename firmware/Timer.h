#ifndef TIMER_H
#define TIMER_H

class Timer {
   public:
      Timer();
      
      unsigned long millis();
      unsigned long micros();
      void delay(unsigned long ms)
      void delayMicroseconds(unsigned int us);
      
   private:   

};

#endif
