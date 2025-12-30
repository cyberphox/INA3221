/*

    Arduino library for INA3221 current and voltage sensor.

    MIT License

    Copyright (c) 2020 Beast Devices, Andrejs Bondarevs

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.

*/

#ifndef _INA3221_H_
#define _INA3221_H_

#include "Arduino.h"
#include "Wire.h"

/*
 * Divide positive or negative dividend by positive or negative divisor
 * and round to closest integer. Result is undefined for negative
 * divisors if the dividend variable type is unsigned and for negative
 * dividends if the divisor variable type is unsigned.
 * //https://elixir.bootlin.com/linux/v6.18.2/source/include/linux/math.h
 */
#define DIV_ROUND_CLOSEST(x, divisor)(      \
{              \
  typeof(x) __x = x;        \
  typeof(divisor) __d = divisor;      \
  (((typeof(x))-1) > 0 ||        \
   ((typeof(divisor))-1) > 0 ||      \
   (((__x) > 0) == ((__d) > 0))) ?    \
    (((__x) + ((__d) / 2)) / (__d)) :  \
    (((__x) - ((__d) / 2)) / (__d));  \
}

//https://www.ti.com/lit/ds/symlink/ina3221.pdf
//https://www.ti.com/lit/an/sboa511a/sboa511a.pdf?ts=1766589721079&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FINA3221
typedef enum {
    INA3221_ADDR40_GND = 0b1000000,  // A0 pin -> GND
    INA3221_ADDR41_VCC = 0b1000001,  // A0 pin -> VCC
    INA3221_ADDR42_SDA = 0b1000010,  // A0 pin -> SDA
    INA3221_ADDR43_SCL = 0b1000011   // A0 pin -> SCL
} ina3221_addr_t;

// Channels
typedef enum {
    INA3221_CH1 = 0,
    INA3221_CH2,
    INA3221_CH3,
    INA3221_CH_NUM
} ina3221_ch_t;

// Registers
typedef enum {
    //All-register reset, shunt and bus voltage ADC conversion times and averaging, operating mode.
    INA3221_REG_CONF = 0,
    //Averaged shunt voltage value.
    INA3221_REG_CH1_SHUNTV,
    //Averaged bus voltage value.
    INA3221_REG_CH1_BUSV,
    //Averaged shunt voltage value.
    INA3221_REG_CH2_SHUNTV,
    //Averaged bus voltage value.
    INA3221_REG_CH2_BUSV,
    //Averaged shunt voltage value.
    INA3221_REG_CH3_SHUNTV,
    //Averaged bus voltage value.
    INA3221_REG_CH3_BUSV,
    //Contains limit value to compare each conversion value to determine
    //if the corresponding limit has been exceeded
    INA3221_REG_CH1_CRIT_ALERT_LIM,
    INA3221_REG_CH1_WARNING_ALERT_LIM,
    INA3221_REG_CH2_CRIT_ALERT_LIM,
    INA3221_REG_CH2_WARNING_ALERT_LIM,
    INA3221_REG_CH3_CRIT_ALERT_LIM,
    INA3221_REG_CH3_WARNING_ALERT_LIM,
    INA3221_REG_SHUNTV_SUM,
    INA3221_REG_SHUNTV_SUM_LIM,
    INA3221_REG_MASK_ENABLE,
    INA3221_REG_PWR_VALID_HI_LIM,
    INA3221_REG_PWR_VALID_LO_LIM,
    INA3221_REG_MANUF_ID = 0xFE,
    INA3221_REG_DIE_ID   = 0xFF
} ina3221_reg_t;

// Conversion times
typedef enum {
    INA3221_REG_CONF_CT_140US = 0,
    INA3221_REG_CONF_CT_204US,
    INA3221_REG_CONF_CT_332US,
    INA3221_REG_CONF_CT_588US,
    INA3221_REG_CONF_CT_1100US,
    INA3221_REG_CONF_CT_2116US,
    INA3221_REG_CONF_CT_4156US,
    INA3221_REG_CONF_CT_8244US
} ina3221_conv_time_t;

// Averaging modes
typedef enum {
    INA3221_REG_CONF_AVG_1 = 0,
    INA3221_REG_CONF_AVG_4,
    INA3221_REG_CONF_AVG_16,
    INA3221_REG_CONF_AVG_64,
    INA3221_REG_CONF_AVG_128,
    INA3221_REG_CONF_AVG_256,
    INA3221_REG_CONF_AVG_512,
    INA3221_REG_CONF_AVG_1024
} ina3221_avg_mode_t;

//Operating modes
typedef enum {
    INA3221_REG_CONF_MODE_POWER_DOWN = 0,
    INA3221_REG_CONF_MODE_SHUNT_SINGLE_SHOT,
    INA3221_REG_CONF_MODE_BUS_SINGLE_SHOT,
    INA3221_REG_CONF_MODE_SHUNT_BUS_SINGLE_SHOT,
    INA3221_REG_CONF_MODE_POWER_DOWN_4,
    INA3221_REG_CONF_MODE_SHUNT_CONTINUOUS,
    INA3221_REG_CONF_MODE_BUS_CONTINUOUS,
    INA3221_REG_CONF_MODE_SHUNT_BUS_CONTINUOUS
} ina3221_operating_mode_t;


/* Lookup table for Bus and Shunt conversion times in usec */
static const u_int16_t ina3221_conv_time[] = {
	140, 204, 332, 588, 1100, 2116, 4156, 8244,
};

/* Lookup table for number of samples using in averaging mode */
static const uint16_t ina3221_avg_samples[] = {
	1, 4, 16, 64, 128, 256, 512, 1024,
};

static const uint16_t INA3221_CONFIG_DEFAULT = 0x7127;
static const uint32_t INA3221_RSHUNT_DEFAULT = 10000; // 10000 microOhm

static int32_t _shuntVoltage_uV[] = {0, 0, 0};
static int32_t _busVoltage_uV[] = {0, 0, 0};

class INA3221 {
    // Configuration register
    typedef struct {


        //Operating mode. These bits select continuous, single-shot
        // (triggered), or power-down mode of operation. These bits default
        // to continuous shunt and bus mode.
        // 000 = Power-down
        // 001 = Shunt voltage, single-shot (triggered)
        // 010 = Bus voltage, single-shot (triggered)
        // 011 = Shunt and bus, single-shot (triggered)
        // 100 = Power-down
        // 101 = Shunt voltage, continuous
        // 110 = Bus voltage, continuous
        // 111 = Shunt and bus, continuous (default)
        uint16_t operating_mode : 3;

        //uint16_t mode_shunt_en : 1;
        //uint16_t mode_bus_en : 1;
        //uint16_t mode_continious_en : 1;

        // Bus-voltage conversion time. These bits set the conversion time
        // for the bus-voltage measurement.
        // 000 = 140μs
        // 001 = 204μs
        // 010 = 332μs
        // 011 = 588μs
        // 100 = 1.1ms (default)
        // 101 = 2.116ms
        // 110 = 4.156ms
        // 111 = 8.244ms
        uint16_t shunt_conv_time : 3;
        uint16_t bus_conv_time : 3;

        // Averaging mode. These bits set the number of samples that are
        // collected and averaged together.
        // 000 = 1 (default)
        // 001 = 4
        // 010 = 16
        // 011 = 64
        // 100 = 128
        // 101 = 256
        // 110 = 512
        // 111 = 1024
        uint16_t avg_mode : 3;

        // Channel enable mode. These bits allow each channel to be
        // independently enabled or disabled.
        // 0 = Channel disable
        // 1 = Channel enable (default)
        uint16_t ch3_en : 1;
        uint16_t ch2_en : 1;
        uint16_t ch1_en : 1;
        uint16_t reset : 1;
    } __attribute__((packed)) conf_reg_t;

    // Mask/Enable register
    typedef struct {
        uint16_t conv_ready : 1;
        uint16_t timing_ctrl_alert : 1;
        uint16_t pwr_valid_alert : 1;
        uint16_t warn_alert_ch3 : 1;
        uint16_t warn_alert_ch2 : 1;
        uint16_t warn_alert_ch1 : 1;
        uint16_t shunt_sum_alert : 1;
        uint16_t crit_alert_ch3 : 1;
        uint16_t crit_alert_ch2 : 1;
        uint16_t crit_alert_ch1 : 1;
        uint16_t crit_alert_latch_en : 1;
        uint16_t warn_alert_latch_en : 1;
        uint16_t shunt_sum_en_ch3 : 1;
        uint16_t shunt_sum_en_ch2 : 1;
        uint16_t shunt_sum_en_ch1 : 1;
        uint16_t reserved : 1;
    } __attribute__((packed)) masken_reg_t;

    // Arduino's I2C library
    TwoWire *_i2c;

    // I2C address
    ina3221_addr_t _i2c_addr;

    // Shunt resistance in mOhm
    uint32_t _shuntRes[INA3221_CH_NUM];

    // Series filter resistance in Ohm
    uint32_t _filterRes[INA3221_CH_NUM];

    // Value of Mask/Enable register.
    masken_reg_t _masken_reg;

    // Reads 16 bytes from a register.
    void _read(ina3221_reg_t reg, uint16_t *val);

    // Writes 16 bytes to a register.
    void _write(ina3221_reg_t reg, uint16_t *val);

   public:
    INA3221(ina3221_addr_t addr) : _i2c_addr(addr){};
    // Initializes INA3221
    bool begin(TwoWire *theWire = &Wire);

    // Sets shunt resistor value in mOhm
    void setShuntRes_uOhm(uint32_t res_ch1, uint32_t res_ch2, uint32_t res_ch3);

    // Sets filter resistors value in Ohm
    void setFilterRes(uint32_t res_ch1, uint32_t res_ch2, uint32_t res_ch3);

    // Sets I2C address of INA3221
    void setAddr(ina3221_addr_t addr) {
        _i2c_addr = addr;
    }

    // Gets configuration register value.
    uint16_t getConfiguration();

    // Gets a register value.
    uint16_t getReg(ina3221_reg_t reg);

    // Resets INA3221
    void reset();

    // Sets operating mode to power-down
    void setModePowerDown();

    // Sets operating mode to continious
    void setModeContinious();

    // Sets operating mode to triggered (single-shot)
    void setModeTriggered();

    // Enables shunt-voltage measurement
    void setShuntMeasEnable();

    // Disables shunt-voltage mesurement
    void setShuntMeasDisable();

    // Enables bus-voltage measurement
    void setBusMeasEnable();

    // Disables bus-voltage measureement
    void setBusMeasDisable();

    // Set operating mode
    void setOperatingMode(ina3221_operating_mode_t mode);

    // Sets averaging mode. Sets number of samples that are collected
    // and averaged togehter.
    void setAveragingMode(ina3221_avg_mode_t mode);

    // Sets bus-voltage conversion time.
    void setBusConversionTime(ina3221_conv_time_t convTime);

    // Sets shunt-voltage conversion time.
    void setShuntConversionTime(ina3221_conv_time_t convTime);

    // Sets power-valid upper-limit voltage. The power-valid condition
    // is reached when all bus-voltage channels exceed the value set.
    // When the powervalid condition is met, the PV alert pin asserts high.
    void setPwrValidUpLimit(int16_t voltagemV);

    // Sets power-valid lower-limit voltage. If any bus-voltage channel drops
    // below the power-valid lower-limit, the PV alert pin pulls low.
    void setPwrValidLowLimit(int16_t voltagemV);

    // Sets the value that is compared to the Shunt-Voltage Sum register value
    // following each completed cycle of all selected channels to detect
    // for system overcurrent events.
    void setShuntSumAlertLimit(int32_t voltagemV);

    // Sets the current value that is compared to the sum all currents.
    // This function is a helper for setShuntSumAlertLim(). It onverts current
    // value to shunt voltage value.
    void setCurrentSumAlertLimit(int32_t currentmA);

    // Enables warning alert latch.
    void setWarnAlertLatchEnable();

    // Disables warning alert latch.
    void setWarnAlertLatchDisable();

    // Enables critical alert latch.
    void setCritAlertLatchEnable();

    // Disables critical alert latch.
    void setCritAlertLatchDisable();

    // Reads flags from Mask/Enable register.
    // When Mask/Enable register is read, flags are cleared.
    // Use getTimingCtrlAlertFlag(), getPwrValidAlertFlag(),
    // getCurrentSumAlertFlag() and getConvReadyFlag() to get flags after
    // readFlags() is called.
    void readFlags();

    // Gets timing-control-alert flag indicator.
    bool getTimingCtrlAlertFlag();

    // Gets power-valid-alert flag indicator.
    bool getPwrValidAlertFlag();

    // Gets summation-alert flag indicator.
    bool getCurrentSumAlertFlag();

    // Gets Conversion-ready flag.
    bool getConversionReadyFlag();

    // Gets manufacturer ID.
    // Should read 0x5449.
    uint16_t getManufID();

    // Gets die ID.
    // Should read 0x3220.
    uint16_t getDieID();

    // Enables channel measurements
    void setChannelEnable(ina3221_ch_t channel);

    // Disables channel measurements
    void setChannelDisable(ina3221_ch_t channel);

    // Sets warning alert shunt voltage limit
    void setWarnAlertShuntLimit(ina3221_ch_t channel, int32_t voltageuV);

    // Sets critical alert shunt voltage limit
    void setCritAlertShuntLimit(ina3221_ch_t channel, int32_t voltageuV);

    // Sets warning alert current limit
    void setWarnAlertCurrentLimit(ina3221_ch_t channel, int32_t currentmA);

    // Sets critical alert current limit
    void setCritAlertCurrentLimit(ina3221_ch_t channel, int32_t currentmA);

    // Includes channel to fill Shunt-Voltage Sum register.
    void setCurrentSumEnable(ina3221_ch_t channel);

    // Excludes channel from filling Shunt-Voltage Sum register.
    void setCurrentSumDisable(ina3221_ch_t channel);

    // Gets shunt voltage in uV.
    int32_t getShuntVoltage_uV(ina3221_ch_t channel);

    // Gets warning alert flag.
    bool getWarnAlertFlag(ina3221_ch_t channel);

    // Gets critical alert flag.
    bool getCritAlertFlag(ina3221_ch_t channel);

    // Estimates offset voltage added by the series filter resitors
    int32_t estimateOffsetVoltage(ina3221_ch_t channel, uint32_t busVoltage);

    // Gets current in A.
    float getCurrent(ina3221_ch_t channel);

    // Gets current compensated with calculated offset voltage.
    float getCurrentCompensated(ina3221_ch_t channel);

    // Gets bus voltage in uV.
    int32_t getBusVoltage_uV(ina3221_ch_t channel);
};

#endif

// The INA3221 performs two measurements on up to three power supplies of interest. The voltage developed from
// the load current passing through a shunt resistor creates a shunt voltage that is measured between the IN+ and
// IN– pins. The device also internally measures the power-supply bus voltage at the IN– pin for each channel. The
// differential shunt voltage is measured with respect to the IN– pin, and the bus voltage is measured with respect
// to ground.

// CAUTION
// Based on the fixed 8mV bus-voltage register LSB (for any channel), a full-scale register value results
// in 32.76V. However, the actual voltage applied to the INA3221 input pins must not exceed 26V.

// The INA3221 is typically powered by a separate power supply that ranges from 2.7V to 5.5V. The monitored
// supply buses range from 0V to 26V.

// There are no special power-supply sequencing considerations between the common-mode input ranges and the
// device power-supply voltage because each are independent of the other; therefore, the bus voltages can be
// present with the supply voltage off and reciprocally.

// The INA3221 takes two measurements for each channel: one for shunt voltage and one for bus voltage.

// Each measurement can be independently or sequentially measured, based on the mode setting (bits 2-0 in
// the Configuration register). When the INA3221 is in normal operating mode (that is, the MODE bits of the
// Configuration register are set to 111), the device continuously converts a shunt-voltage reading followed by a
// bus-voltage reading. This procedure converts one channel, and then continues to the shunt voltage reading of
// the next enabled channel, followed by the bus-voltage reading for that channel, and so on, until all enabled
// channels have been measured.

// The programmed Configuration register mode setting applies to all channels.
// Any channels that are not enabled are bypassed in the measurement sequence, regardless of mode setting.

// The INA3221 has two operating modes, continuous and single-shot, that determine the internal ADC operation
// after these conversions complete. When the INA3221 is set to continuous mode (using the MODE bit settings),
// the device continues to cycle through all enabled channels until a new configuration setting is programmed.
// The Configuration register MODE control bits also enable modes to be selected that convert only the shunt or
// bus voltage. This feature further allows the device to fit specific application requirements.
// In single-shot (triggered) mode, setting any single-shot convert mode to the Configuration register (that is, the
// Configuration register MODE bits set to 001, 010, or 011) triggers a single-shot conversion. This action produces
// a single set of measurements for all enabled channels. To trigger another single-shot conversion, write to the
// Configuration register a second time, even if the mode does not change. When a single-shot conversion is
// initiated, all enabled channels are measured one time and then the device enters a power-down state. The
// INA3221 registers can be read at any time, even while in power-down. The data present in these registers are
// from the last completed conversion results for the corresponding register. The conversion ready flag bit (Mask/
// Enable register, CVRF bit) helps coordinate single-shot conversions, and is especially helpful during longer
// conversion time settings. The CVRF bit is set after all conversions are complete. The CVRF bit clears under the
// following conditions:
// 1. Writing to the Configuration register, except when configuring the MODE bits for power-down mode; or
// 2. Reading the Mask/Enable register.
// In addition to the two operating modes (continuous and single-shot), the INA3221 also has a separate selectable
// power-down mode that reduces the quiescent current and turns off current into the INA3221 inputs. Power-down
// mode reduces the impact of supply drain when the device is not used. Full recovery from power-down mode
// requires 40µs. The INA3221 registers can be written to and read from while the device is in power-down mode.
// The device remains in power-down mode until one of the active MODE settings are written to the Configuration
// register.
