

#include "pulse_output.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

#define bit(x) ( 1 << (x))

volatile uint16_t pulse_output::channel_value[max_channels];
bool     pulse_output::inverted_pulse = true;
uint8_t  pulse_output::reversed_mask = 0U;
uint8_t  pulse_output::num_channels = max_channels;
uint16_t pulse_output::min_sync_len = 4000; // usec
uint16_t pulse_output::frame_period= 0; // = num_channels * max_val + min_sync_len;

namespace{
// with prescaler of 8 each compare timer tick is 0.5 usec
// so multiply time in usec by 2 for compare reg
   uint16_t rawValueFromPulseTime(uint16_t val)
   {
      return val * 2U;
   }
}

// true if user wants the output channel reversed
bool pulse_output::is_reversed(uint8_t channel_idx)
{
   return (reversed_mask & bit(channel_idx) ) != static_cast<uint8_t>(0);
}

// input val  is nominally +- 0.5
// but with gain can be bigger
void pulse_output::set_channel(uint8_t channel, float const & val_in)
{
   float const & val = ( is_reversed(channel) )?-val_in:val_in;
   int16_t chval =  static_cast<int16_t>(val * 1000) + 1500;
  
   if ( chval < static_cast<int16_t>(min_val)){
         chval = static_cast<int16_t>(min_val);
   }else{
      if( chval > static_cast<int16_t>(max_val)){
         chval = static_cast<int16_t>(max_val);
       }
   }
  // disable the comp irq to update
  // TIMSK1 &= ~(1 << OCIE1A);
   cli();
   channel_value[channel] = static_cast<uint16_t>(chval);
   sei();
 // TIMSK1 |= (1 << OCIE1A);
}

// N.B. only call from in irq
uint16_t get_pulse_output_channel(uint8_t channel)
{
    return pulse_output::channel_value[channel];
}

ISR(TIMER1_COMPA_vect)
{
   constexpr uint16_t pulsegap = 400; // us

   static uint8_t  channelIndex = 0; // current channel for ISR;
   static bool inPulse = true; // true if we were currently doing a pulsegap
   static uint16_t period_left = 22000; //for init val  want enough for all channels at max and syncpulse

   if (inPulse == true){ // end of pulse irq
      if( channelIndex == pulse_output::get_num_channels()){
         OCR1A = rawValueFromPulseTime(period_left - pulsegap);
         period_left = pulse_output::get_frame_period(); 
      }else{
         uint16_t chan_pulse_len = get_pulse_output_channel(channelIndex) - pulsegap;
         OCR1A = rawValueFromPulseTime(chan_pulse_len);
         period_left -= chan_pulse_len;
      }
      if (pulse_output::want_inverted_pulse()){
         TCCR1A |= (1 << COM1A1) | (1 << COM1A0);
      }else{
         TCCR1A = ( TCCR1A |(1 << COM1A1 ) ) & ~(1 << COM1A0);
      }
      inPulse = false;
   }else{ // end of channel irq
      OCR1A = rawValueFromPulseTime(pulsegap);
      period_left -= pulsegap;
      // set opt to change for end of pulse
      if (!pulse_output::want_inverted_pulse()){
         TCCR1A |= (1 << COM1A1) | (1 << COM1A0);
      }else{
         TCCR1A = ( TCCR1A |(1 << COM1A1 ) ) & ~(1 << COM1A0);
      }
      channelIndex = (channelIndex + 1U) % (pulse_output::get_num_channels() + 1U );  
      inPulse = true;
   }
}

void pulse_output::setup()
{
  frame_period = 20000;//num_channels * max_val + min_sync_len;
  // init values
  for(auto & v : channel_value){ v = 1500;}

  DDRB |= (1<<1);
  PORTB &= ~(1<<1);

  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B

  TCNT1 = 0;
  //... arbitrary first interrupt in 1 ms
  OCR1A =  rawValueFromPulseTime(1000);
  // turn on CTC mode conjunction with OCR1A
  TCCR1B |= (1 << WGM12);
  // Set  /8 prescaler so clock is 0.5 usec
  TCCR1B |=  (1 << CS11);  
  // clear compare interrupt flag
  TIFR1 &= ~(1 << OCF1A);
  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
}