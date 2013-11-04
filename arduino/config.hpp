#ifndef RCTX_HANDSET_CONFIG_HPP_INCLUDED
#define RCTX_HANDSET_CONFIG_HPP_INCLUDED

// Timer1 compare interrupt is used for PWM output
// timer2 

#include <stdint.h>
// magic number hour and date
inline constexpr uint32_t get_application_magic()
{
   return 1024092013;
}

void eeprom_write_default_config();




#endif // RCTX_HANDSET_CONFIG_HPP_INCLUDED
