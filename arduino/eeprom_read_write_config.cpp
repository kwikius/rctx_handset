
#include "config.hpp"
#include "eeprom_map.hpp"
#include "raw_analog_input.hpp"

// ram values have default initialisers for no config
// do we need to write these?
void eeprom_write_default_config()
{
   write<eeprom_map::magic>(get_application_magic());
   write<eeprom_map::num_input_channels>(raw_analog_input::get_num_channels()); 
   write<eeprom_map::reversed_mask>(raw_analog_input::reversed_mask); 
}

void get_initialisers_from_eeprom()
{
  read<eeprom_map::num_input_channels>(raw_analog_input::num_channels); 
  read<eeprom_map::reversed_mask>(raw_analog_input::reversed_mask);
}