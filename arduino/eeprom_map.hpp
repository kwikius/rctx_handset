#ifndef EEPROM_MAP_HPP_INCLUDED
#define EEPROM_MAP_HPP_INCLUDED

#include "eeprom_data.hpp"

   struct eeprom_map{

      typedef eeprom::variable<void,uint32_t> magic;  
      typedef eeprom::variable<magic,uint8_t> num_input_channels;  
      typedef eeprom::variable<num_input_channels,uint8_t> reversed_mask; 
   };

#endif // EEPROM_MAP_HPP_INCLUDED
