#ifndef RCTX_HANSET_EEPROM_DATA_HPP_INCLUDED
#define RCTX_HANSET_EEPROM_DATA_HPP_INCLUDED

//#include <Arduino.h>
#include <avr/eeprom.h>
#include <quan/is_model_of.hpp>
#include <quan/meta/and.hpp>
#include <quan/meta/not.hpp>

#include <quan/std/tr1/is_same.hpp>
#include <quan/std/tr1/is_array.hpp>
#include <quan/std/tr1/extent.hpp>
#include <quan/std/tr1/remove_extent.hpp>
#include <quan/std/tr1/rank.hpp>

namespace eeprom{

   typedef uint16_t address_type; 

 // use to build the EEPROM data in sequence in eeprom memory
   // Prev is previous item in eeprom, type is runtime type
   template <typename Prev, typename type_in>
   struct variable{
       static constexpr address_type address_value = Prev::address_value + Prev::size_value;
       typedef type_in type;
       static constexpr uint16_t size_value = sizeof(type);
   };

    // specialise first item
   template <typename type_in>
   struct variable<void,type_in>{
       static constexpr address_type address_value = 0;
       typedef type_in type;
       static constexpr uint16_t size_value = sizeof(type);
   };

   // helper for reading and writing
   // want to differentiate version for arrays and non arrays
   template <typename T, typename where = void>
   struct accessor;

   //accessor for non arrays
   template <typename T>
   struct accessor<T, 
      typename quan::where_<
         quan::meta::not_<
            std::is_array<T> 
         > 
      >::type
   > {

      static void read( address_type ee_addr_in, T & result_out)
      {
        union{
            T val;
            uint8_t arr[sizeof(T)];
        } result_union;

        address_type cur_ee_addr = ee_addr_in;
        for ( uint16_t i = 0; i < sizeof(T); ++i){
            result_union.arr[i] = eeprom_read_byte(reinterpret_cast<uint8_t*>(cur_ee_addr));
            ++cur_ee_addr;
        }
        result_out = result_union.val;
      }

      static void write(address_type ee_addr_in, T const & val_in)
      {
         union{
            T val;
            uint8_t arr[sizeof(T)];
         } arg_union;

         arg_union.val = val_in;

         address_type cur_ee_addr = ee_addr_in;

         for ( uint16_t i = 0; i < sizeof(T); ++i){
         // only write if necessary
//            if (  eeprom_read_byte(reinterpret_cast<uint8_t*>(cur_ee_addr)) != arg_union.arr[i]){
//               eeprom_write_byte(reinterpret_cast<uint8_t*>(cur_ee_addr),arg_union.arr[i]);
//            }
            eeprom_update_byte(reinterpret_cast<uint8_t*>(cur_ee_addr),arg_union.arr[i]);
            ++cur_ee_addr;
         }
      }
   };

   //accessor for arrays
   template <typename T>
   struct accessor<T, typename quan::where_<std::is_array<T> >::type> {

      static void read( address_type ee_addr, T & result)
      {
         static const uint16_t size = std::extent<T>::value;
         typedef typename std::remove_extent<T>::type element_type;
         address_type next_ee_addr = ee_addr;
         for ( uint16_t i = 0; i < size;++i){
            accessor<element_type>::read(next_ee_addr,result[i]);
            next_ee_addr += sizeof(element_type);
         } 
      }

      static void write(address_type ee_addr, T const & val)
      {
         static const uint16_t size = std::extent<T>::value;
         typedef typename std::remove_extent<T>::type element_type;
         address_type next_ee_addr = ee_addr;
         for ( uint16_t i = 0; i < size;++i){
            accessor<element_type>::write(next_ee_addr,val[i]);
            next_ee_addr += sizeof(element_type);
         } 
      }

   };

}//eeprom

namespace quan{ 
   struct EEpromID;
   namespace impl{
      template <typename T, typename T1> struct is_model_of_impl<quan::EEpromID,eeprom::variable<T,T1> > : quan::meta::true_{};
   } 
}

template<typename EEID, typename T >
inline
typename quan::where_<
   quan::meta::and_<
      quan::is_model_of<quan::EEpromID,EEID>,
      std::is_same<T,typename EEID::type>
   >
>::type
 read( T & result)
{
   eeprom::accessor<typename EEID::type>::read(EEID::address_value,result);
}

template<typename EEID, typename T >
inline
typename quan::where_<
   quan::meta::and_<
      quan::is_model_of<quan::EEpromID,EEID>,
      std::is_same<T,typename EEID::type>
   >
>::type
write( T const & value)
{
   eeprom::accessor<typename EEID::type>::write(EEID::address_value,value);
}

#endif // RCTX_HANSET_EEPROM_DATA_HPP_INCLUDED
