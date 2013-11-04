#ifndef RCTX_HANDSET_RAW_ANALOG_INPUT_HPP_INCLUDED
#define RCTX_HANDSET_RAW_ANALOG_INPUT_HPP_INCLUDED

#include <stdint.h>

void set_channel_values();
void get_initialisers_from_eeprom();

extern "C" void setup();

struct raw_analog_input{
  
  // raw ato d values read by the sensors
  // 0 to 1023 but actually should be limited to approx 61 to 961
  // since nominal max difference is 900 around centre

   static constexpr uint8_t max_channels = 8;
   static constexpr uint16_t nominal_min_val = 61;
   static constexpr uint16_t nominal_max_val = 961;
   static constexpr uint16_t max_output_val = nominal_max_val - nominal_min_val;
   static uint8_t get_num_channels(){ return num_channels;}
   //#### call with interrupts disabled ##################
   // returns a value in range 0 to max_output_val
   static uint16_t get_channel_value(uint8_t channel_idx);
   static bool is_reversed(uint8_t channel);
   //#####################################################
   private:  
   
   friend void set_channel_values(uint8_t chan);
   friend void get_initialisers_from_eeprom();
   friend void eeprom_write_default_config();
   friend void setup();

   static void init();
   static uint8_t num_channels;
   static volatile uint16_t channel_value[max_channels];
   static uint8_t reversed_mask;
};

#endif // RCTX_HANDSET_RAW_ANALOG_INPUT_HPP_INCLUDED
