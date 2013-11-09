//#include <Arduino.h>
#include <avr/interrupt.h>
#include "config.hpp"
#include "eeprom_map.hpp"
#include "raw_analog_input.hpp"
#include "pulse_output.hpp"

namespace{
      //static const int HeartbeatLed = 13; // On all boards?
}

extern "C" void setup()
{
#if 0
 // leave eeprom for now
   uint32_t magic;
   read<eeprom_map::magic>(magic);

   if ( magic != get_application_magic()){
      eeprom_write_default_config();
   }
   get_initialisers_from_eeprom();
#endif
   cli();

   raw_analog_input::init();
   pulse_output::setup();

    DDRB |= (1<<5);
    PORTB &= ~(1<<5);
   sei();
}

void Tensor_calculate();
void default_mixer_calculate();

void mix_calc()
{
  // Tensor_calculate();
   default_mixer_calculate();
}

extern "C" void loop()
{
   mix_calc();
#if 0
   static int count = 0;
   static bool led_state = false;
   if (++count == 1000){
     count = 0;
      if ( led_state == false){
//         digitalWrite(HeartbeatLed,HIGH);
           PORTB |= (1 <<5);
         led_state = true;
      }else{
          PORTB &= ~(1 << 5);
        // digitalWrite(HeartbeatLed,LOW);
         led_state = false;
      }
   }
#endif
}

int main()
{
   setup();
   for(;;){
      loop();
   }
}









