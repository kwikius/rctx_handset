

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

  static float const throttle_offset = 0.2f;

  input_channel_info_t input_channel_info[raw_analog_input::max_channels]
   = {
      { 1.0f},
      { 1.0f}, 
      { 1.4f}, 
      { 1.0f}, 
      { 1.0f},
      { 1.1f},
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

   float get_throttle()
   {
       return get_normed_input_channel(input_channel_map::throttle) - throttle_offset;
   }

   float get_flight_mode()
   {
       return get_normed_input_channel(input_channel_map::flight_mode);
   }

   struct output_channel_map{
      static uint8_t left_elevon; 
      static uint8_t right_elevon;
      static uint8_t rudder;
      static uint8_t throttle;
      static uint8_t flap;
      static uint8_t flight_mode;
   };
   uint8_t output_channel_map::left_elevon = 0;
   uint8_t output_channel_map::right_elevon = 1;
   uint8_t output_channel_map::throttle = 2;
   uint8_t output_channel_map::rudder = 3;
   uint8_t output_channel_map::flight_mode = 4;
   uint8_t output_channel_map::flap = 5;

   float const k_pitch = 0.5;
   float const k_flap_mix = 0.5;
   float const k_roll = 0.5;
} // namespace 


void Tensor_calculate()
{
   float const roll_part = get_roll() * k_roll;
   float const pitch_part = (get_pitch() - get_flap() * k_flap_mix) * k_pitch ;
   
   float const left_elevon = pitch_part - roll_part;
   pulse_output::set_channel(output_channel_map::left_elevon,left_elevon);
   float const right_elevon =  -pitch_part - roll_part;
   pulse_output::set_channel(output_channel_map::right_elevon,right_elevon);

   pulse_output::set_channel(output_channel_map::throttle,get_throttle());
   pulse_output::set_channel(output_channel_map::rudder,get_yaw());

   pulse_output::set_channel(output_channel_map::flight_mode,get_flight_mode());
   pulse_output::set_channel(output_channel_map::flap,get_flap());

}