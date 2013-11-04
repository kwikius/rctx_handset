#ifndef RCTX_HANDSET_PULSE_OUTPUT_HPP_INCLUDED
#define RCTX_HANDSET_PULSE_OUTPUT_HPP_INCLUDED

#include <stdint.h>

extern "C" void setup();

struct pulse_output{
   
   static constexpr uint8_t max_channels = 8;
   static constexpr uint16_t min_val = 750; // usec
   static constexpr uint16_t max_val = 2250;// usec
   static void set_channel(uint8_t channel,float const & val);
   static bool want_inverted_pulse(){ return inverted_pulse;}
   static uint8_t get_num_channels(){ return num_channels;}
   static uint16_t get_frame_period(){ return frame_period;} // usec

   static bool is_reversed(uint8_t channel);
  private:

   friend uint16_t get_pulse_output_channel(uint8_t val);
   friend void ::setup();

   static void setup();

      // values in usec
   static volatile uint16_t channel_value[max_channels];
   static bool inverted_pulse;
   static uint8_t reversed_mask;
   static uint8_t num_channels;
   static uint16_t frame_period; //usec
   static uint16_t min_sync_len; //usec
};

#endif // RCTX_HANDSET_PULSE_OUTPUT_HPP_INCLUDED
