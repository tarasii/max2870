#ifndef _MAX2870_H_
#define _MAX2870_H_

#include <Arduino.h>

/**
  @brief The MAX2870 is an ultra-wideband phase-locked loop (PLL) with integrated
  voltage control oscillators (VCOs)capable of operating in both integer-N and
  fractional-N modes. When combined with an external reference oscillator and
  loop filter, the MAX2870 is a high-performance frequency synthesizer capable
  of synthesizing frequencies from 23.5MHz to 6.0GHz while maintaining superior
  phase noise and spurious performance.

  @code

  #include "MAX2870.h"

  void setup() {
    MAX2870 MAX2870(PIN_LE, PIN_CE, PIN_LD);
    MAX2870.powerOn(true);
    MAX2870.setPFD(MAX2870_reference_frequency_mhz, MAX2870_R_divider);
    MAX2870.set_RF_OUT_A(1200); 
  }

@endcode
*/
class MAX2870 {
  public:

    ///@brief MAX2870 Constructor
    MAX2870(const uint8_t MAX2870_pin_LE, const uint8_t MAX2870_pin_CE, const uint8_t MAX2870_pin_LD);

    //MAX2870 Registers
    enum Registers_e
    {
      REG0          = 0x00,
      REG1          = 0x01,
      REG2          = 0x02,
      REG3          = 0x03,
      REG4          = 0x04,
      REG5          = 0x05,
      REG6          = 0x06
    };

    //Register 0 bits
    union REG0_u
    {
      //Access all bits
      uint32_t all;

      //Access individual bits
      struct BitField_s
      {
        uint32_t addr       : 3;
        uint32_t frac       : 12;
        uint32_t n          : 16;
        uint32_t intfrac    : 1;
      } bits;
    };

    //Register 1 bits
    union REG1_u
    {
      //Access all bits
      uint32_t all;

      //Access individual bits
      struct BitField_s
      {
        uint32_t addr         : 3;
        uint32_t m            : 12;
        uint32_t p            : 12;
        uint32_t cpt          : 2;
        uint32_t cpl          : 2;
        uint32_t cpoc         : 1;
      } bits;
    };

    //Register 2 bits
    union REG2_u
    {
      //Access all bits
      uint32_t all;

      //Access individual bits
      struct BitField_s
      {
        uint32_t addr      : 3;
        uint32_t rst       : 1;
        uint32_t tri       : 1;
        uint32_t shdn      : 1;
        uint32_t pdp       : 1;
        uint32_t ldp       : 1;
        uint32_t ldf       : 1;
        uint32_t cp        : 4;
        uint32_t reg4db    : 1;
        uint32_t r         : 10;
        uint32_t rdiv2     : 1;
        uint32_t dbr       : 1;
        uint32_t mux       : 3;
        uint32_t sdn       : 2;
        uint32_t lds       : 1;
      } bits;
    };

    //Register 3 bits
    union REG3_u
    {
      //Access all bits
      uint32_t all;

      //Access individual bits
      struct BitField_s
      {
        uint32_t addr      : 3;
        uint32_t cdiv      : 12;
        uint32_t cdm       : 2;      
        uint32_t reserved0 : 1; //0      //csm
        uint32_t reserved1 : 6; //000000 
        uint32_t retune    : 1;          // vas_temp
        uint32_t vas_shdn  : 1;
        uint32_t vco       : 6;
      } bits;
    };

    //Register 4 bits
    union REG4_u
    {
      //Access all bits
      uint32_t all;

      //Access individual bits
      struct BitField_s
      {
        uint32_t addr       : 3;
        uint32_t apwr       : 2;
        uint32_t rfa_en     : 1;
        uint32_t bpwr       : 2;
        uint32_t rfb_en     : 1;
        uint32_t bdiv       : 1;
        uint32_t reserved0  : 1; //0 //mtld
        uint32_t reserved1  : 1; //0 //sdvco 
        uint32_t bs         : 8;
        uint32_t diva       : 3;
        uint32_t fb         : 1;
        uint32_t bs2        : 2;
        uint32_t reserved2  : 3; //011000
      } bits;
    };

    //Register 5 bits
    union REG5_u
    {
      //Access all bits
      uint32_t all;

      //Access individual bits
      struct BitField_s
      {
        uint32_t addr      : 3;
        uint32_t reserved1 : 15;
        uint32_t mux       : 1; //000000000000000000
        uint32_t reserved2 : 3; //000
        uint32_t ld        : 2;
        uint32_t f01       : 1;
        uint32_t reserved3 : 7; //0000000

      } bits;
    };

    //Register 6 bits
    union REG6_u
    {
      //Access all bits
      uint32_t all;

      //Access individual bits
      struct BitField_s
      {
        uint32_t addr      : 3;
        uint32_t v         : 6;
        //uint32_t vasa      : 1;  //max2871?
        uint32_t reserved1 : 10;  
        //uint32_t adcv      : 1;  //max2871?
        uint32_t adc       : 3;  
        uint32_t por       : 1;  
        uint32_t reserved2 : 9;  
        //uint32_t die       : 4;
      } bits;
    };
    


    ///@brief Updates MAX2870 settings to achieve target output frequency on channel A.\n
    ///@param[in] freq - Frequency in MHz
    void set_RF_OUT_A(double freq);

    ///@brief Provide frequency input to REF_IN pin.\n
    ///@param[in] ref_in - Frequency in MHz
    void setPFD(double ref_in, uint16_t rdiv);
    
    void dump_regs(uint32_t* dump);

    void powerOn(bool pwr);

    void aout(uint8_t pwr);
    void bout(uint8_t pwr, bool bdiv);

    uint8_t get_ld();

    void setConfig(uint8_t first_delay);
    void setConfig();

  private:

    REG0_u reg0;
    REG1_u reg1;
    REG2_u reg2;
    REG3_u reg3;
    REG4_u reg4;
    REG5_u reg5;
    REG6_u reg6;

    double f_pfd;
    double f_rfouta;

    uint8_t pin_LE;
    uint8_t pin_CE;
    uint8_t pin_LD;

    void pulseLE();

    ///@brief Writes raw 32-bit data pattern. The MAX2870 accepts 32-bit words at a time; 29 data bits and 3 address bits.
    ///@param[in] data - 32-bit word to write to the MAX2870. Bits[31:3] contain the register data, and Bits[2:0] contain the register address.
    void writeData(uint32_t data);


};

#endif /* _MAX2870_H_ */


