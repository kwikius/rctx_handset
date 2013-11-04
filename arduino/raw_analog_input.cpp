#include "raw_analog_input.hpp"
#include <Arduino.h>
#include <quan/time.hpp>


uint8_t raw_analog_input::num_channels = 8U;
volatile uint16_t raw_analog_input::channel_value[raw_analog_input::max_channels] = {0};
uint8_t raw_analog_input::reversed_mask = bit(2) | bit(3);

// if an input channel is reversed
bool raw_analog_input::is_reversed(uint8_t channel_idx)
{
   return (reversed_mask & bit(channel_idx) ) != static_cast<uint8_t>(0);
}

/*
return a value of 0 to raw_analog_input::max_output_val
*/
uint16_t raw_analog_input::get_channel_value(uint8_t channel_idx)
{
   // disable ADC irq while reading

 //  ADCSRA &= ~bit(ADIE);
   cli();
   int16_t result = static_cast<int16_t>(channel_value[channel_idx]);
   sei();
 //  ADCSRA |= bit(ADIE);

   if ( result < static_cast<int16_t>(nominal_min_val)){
      result = static_cast<int16_t>(nominal_min_val);
   }else{
      if ( result > static_cast<int16_t>(nominal_max_val)){
          result = static_cast<int16_t>(nominal_max_val);
      }
   }
   // do reversed option
   if( ! is_reversed(channel_idx) ){
      return static_cast<uint16_t>(result - static_cast<int16_t>(nominal_min_val));
   }else{
      return static_cast<uint16_t>(static_cast<int16_t>(nominal_max_val)- result);
   }
}
// need this fn to access private data from irq only
void set_channel_values(uint8_t addr)
{
  // get the A2D value. Low byte must be read first
  uint16_t latest_conv = ADCL;
  latest_conv |= ( static_cast<uint16_t>(ADCH) << 8U ) ;

  // the a2d values for each channel are accumulated
  raw_analog_input::channel_value[addr] = latest_conv;
}

namespace {
  
   constexpr uint8_t raw_analog_input_pin = 3;
// digital address pins are pins 0,1,2
   constexpr uint8_t analog_address_mask = 0b111;

   //The time between conversions
   // gives 8 convs per millisec
   constexpr quan::time_<int16_t>::us time_between_conversions{250};

   // timer clk is 2 usec
   constexpr uint8_t get_a2d_timing()
   {
      static_assert((time_between_conversions.numeric_value() / 2 ) < 255, "acquisition too long");
      return static_cast<uint8_t>(time_between_conversions.numeric_value()/2);
   }

   // timer 2 is used to allow some acqusition time for a2d 
   // between changing input addresses.
   // The timing is based on the slew rate of the opamp

   void a2d_timer_setup()
   {

      TCCR2A = 0;
      TCCR2B = 0;
      TCNT2 = 0;
      TIFR2 = 0;
       // set the compare match value
      OCR2A = get_a2d_timing();

      TCCR2A |= bit(WGM21);

      // clk / 32 gives 0.5 usec res
      TCCR2B |= (bit(CS21) | bit(CS20) );

      // enable the compare interrupt
      TIMSK2 |= bit(OCIE2A);
   }

   // setup the A2D and addressing to read the stick inputs
   void a2d_setup()
   {
      // configure analog_pin to analog input, address pins to digital output
      DDRC = (DDRC | analog_address_mask) & ~bit(raw_analog_input_pin);

      // clearing analog inputs here reduces current apparently
      DIDR0 = (DIDR0 | bit(raw_analog_input_pin) ) & ~analog_address_mask;

      // disable pullup on analog pin and set address outputs low ( to start a2d conv at address 0)
      PORTC &= ~( bit(raw_analog_input_pin) | analog_address_mask );

      // Enable the A2D
      // setup prescaler for < 200 kHz ad clock on 16MHz sysclk means prescaler of 128
      // all other bits in ADCSRA can be cleared at mo
      ADCSRA = bit(ADEN) | bit(ADPS2) | bit(ADPS1) | bit(ADPS0) ;

      // connect vref to AVCC for arduino 
      // and want result in low end of ADCH etc
      ADMUX = (ADMUX | bit(REFS0)) & ~ ( bit(REFS1) | bit(ADLAR) );

      // configure mux to read analog input pin
      ADMUX = (ADMUX & 0xF0) | raw_analog_input_pin;

      // clear interrupt flag
      ADCSRA &= ~bit(ADIF);

      // enable atod conv interrupt
      ADCSRA |= bit(ADIE);

     // ADCSRB = dont care unless using special modes

      // set up the timer
      a2d_timer_setup();
   }
} // namespace

// ADC complete irq .. read next input value
ISR(ADC_vect)
{
   // read the current sensor address
   static uint8_t addr = 0;
   uint8_t const conv_addr = addr;
   set_channel_values(conv_addr);
   addr = ( addr + 1U ) % raw_analog_input::get_num_channels();
  
   // set up next sensor address 
   PORTC = (PORTC & ~analog_address_mask) | addr ;
   // write the current result to the raw analog input channel
   
}

// compare match interrupt..
// settling time for new atod channel is finished
// N.B. could start conv automatically with timer0 but currently usd by arduino..
// or use an output pin connected...?
ISR(TIMER2_COMPA_vect)
{
   ADCSRA |= bit(ADSC);  
}

void raw_analog_input::init()
{
   constexpr uint16_t init_val = (nominal_min_val + nominal_max_val) / 2U;

   for (auto & v : channel_value){ v = init_val;}

   a2d_setup();
}







