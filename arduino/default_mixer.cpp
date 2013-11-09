
#include "raw_analog_input.hpp"
#include "pulse_output.hpp"

namespace {

  struct input_channel_map{
      static uint8_t yaw; 
      static uint8_t pitch;
      static uint8_t roll;
      static uint8_t throttle;
      static uint8_t flap;
      static uint8_t flight_mode;
  };
  uint8_t input_channel_map::roll = 0;
  uint8_t input_channel_map::pitch = 1;
  uint8_t input_channel_map::throttle = 2;
  uint8_t input_channel_map::yaw = 3;
  uint8_t input_channel_map::flight_mode = 4;
  uint8_t input_channel_map::flap = 5;

  struct input_channel_info_t{
      input_channel_info_t(float const & gain_in)
      :gain{gain_in}{ }
      float gain;
  };

  input_channel_info_t input_channel_info[raw_analog_input::max_channels]
   = {
      { 1.0f},
      { 1.0f}, 
      { 1.4f}, 
      { 1.0f}, 
      { 1.0f},
      { 1.2f}, 
      { 1.0f},
      { 1.0f}
  };

   // returns gain * ( value between -0.5 to 0.5)
   float get_normed_input_channel(uint8_t channel)
   {
      auto const & info = input_channel_info[channel];

      return info.gain *
         ( static_cast<int16_t>(raw_analog_input::get_channel_value(channel)) 
            - static_cast<int16_t>(raw_analog_input::max_output_val / 2U) 
         )
            / static_cast<float>(raw_analog_input::max_output_val) ;
   }

   float get_roll()
   {
      return get_normed_input_channel(input_channel_map::roll);
   }

   float get_pitch()
   {
       return get_normed_input_channel(input_channel_map::pitch);
   }

   float get_yaw()
   {
       return get_normed_input_channel(input_channel_map::yaw);
   }

   float get_flap()
   {
       return get_normed_input_channel(input_channel_map::flap);
   }

   float const throttle_offset = 0.2f;
   float get_throttle()
   {
       return get_normed_input_channel(input_channel_map::throttle) - throttle_offset;
   }

   float get_flight_mode()
   {
       return get_normed_input_channel(input_channel_map::flight_mode);
   }

   struct output_channel_map{
      static uint8_t aileron; 
      static uint8_t elevator;
      static uint8_t throttle;
      static uint8_t rudder;
      static uint8_t flap;
      static uint8_t flight_mode;
   };
   uint8_t output_channel_map::aileron = 0;
   uint8_t output_channel_map::elevator = 1;
   uint8_t output_channel_map::throttle = 2;
   uint8_t output_channel_map::rudder = 3;
   uint8_t output_channel_map::flight_mode = 4;
   uint8_t output_channel_map::flap = 5;

} // namespace 


void default_mixer_calculate()
{
   pulse_output::set_channel(output_channel_map::aileron,get_roll());
   pulse_output::set_channel(output_channel_map::elevator,get_pitch());
   pulse_output::set_channel(output_channel_map::throttle,get_throttle());
   pulse_output::set_channel(output_channel_map::rudder,get_yaw());
   pulse_output::set_channel(output_channel_map::flight_mode,get_flight_mode());
   pulse_output::set_channel(output_channel_map::flap,get_flap());
}