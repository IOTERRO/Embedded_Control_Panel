/**
 * @file PCA9685.h
 * @brief Definitions and enums for controlling the PCA9685 PWM Driver.
 *
 * Copyright (c) 2024 - FutureIsTech
 * Author: Omar Terro
 * Creation Date: 10/03/2024
 * All rights reserved
 * Project: Using PCA9685 PWM Driver
 *
 * Description:
 * This header file provides definitions and enums for controlling the PCA9685 PWM Driver.
 * The PCA9685 is a 16-channel, 12-bit PWM LED driver with I2C interface, designed to control
 * LEDs (light-emitting diodes) and provide smooth, flicker-free color mixing. It can also be used
 * for controlling servo motors by generating precise PWM signals.
 *
 * The header file includes enums for configuring various registers of the PCA9685, such as LED ON/OFF,
 * PWM frequency, and global control registers.
 *
 * The PCA9685 offers precise control over LED brightness and flicker-free operation, making it suitable
 * for applications such as automotive lighting, backlighting for LCD displays, decorative lighting, and
 * servo motor control.
 *
 * For more information, refer to the PCA9685 datasheet.
 
 * @brief Calculate PWM values for LED control.
 * PWM (Pulse Width Modulation) is a technique used to control the brightness of LEDs
 * by varying the duty cycle of a square wave signal. In the PCA9685 LED controller,
 * each LED output has two 12-bit registers: LEDn_ON and LEDn_OFF. These registers
 * control the turn-on and turn-off time of the LED output, allowing precise control
 * over brightness.
 *
 * To calculate PWM values:
 *
 * 1. Determine the desired delay time and duty cycle:
 *    - Delay Time: The percentage of time the LED output remains off at the beginning
 *                  of each cycle.
 *    - Duty Cycle: The percentage of time the LED output remains on during each cycle.
 *
 * 2. Convert the percentage values to counts:
 *    - The 12-bit PWM counter runs from 0 to 4095.
 *    - Calculate the counts corresponding to the desired percentage.
 *
 * 3. Set the LEDn_ON registers:
 *    - LEDn_ON_H and LEDn_ON_L represent the turn-on time of the LED output.
 *    - Calculate the counts for LED on time and split it into high and low bytes.
 *
 * 4. Set the LEDn_OFF registers:
 *    - LEDn_OFF_H and LEDn_OFF_L represent the turn-off time of the LED output.
 *    - Calculate the counts for LED off time using the delay time, duty cycle, and 4095.
 *    - Split the calculated off time into high and low bytes.
 *
 * Example:
 *   - Delay Time: 10%
 *   - Duty Cycle: 20%
 *
 *   Delay Time counts: 10% of 4095 = 410
 *   Duty Cycle counts: 20% of 4095 = 819
 *
 *   LED0_ON_H = 0x01
 *   LED0_ON_L = 0x99 (corresponding to 410 counts)
 *
 *   Off Time = Delay Time + Duty Cycle - 1 = 410 + 819 - 1 = 1228 counts
 *   LED0_OFF_H = 0x04
 *   LED0_OFF_L = 0xCC (corresponding to 1228 counts)
 
                                      4095|
            ---------                     |    ---------
            |       |                     |    |       |
            |       |                     |    |       |  
            |       |                     |    |       |
        ----        ---------------------------        --------
        --->|819    |                     |--->|819    |  
        LED_ON      |                     |LED_ON      |   
        ------------>1228                 |----------->1228
        LED_OFF                           |   LED_OFF

* Example 2: (assumes that the LED4 output is used and (delay time) + (PWM duty cycle > 100 %)
*   Delay time = 90 %; PWM duty cycle = 90 % (LED on time = 90 %; LED off time = 10 %).
*   Delay time = 90 % = 3686.4 ~ 3686 counts − 1 = 3685 = E65h.
*   LED4_ON_H = Eh; LED4_ON_L = 65h
*   LED on time = 90 % = 3686 counts.
*   Since the delay time and LED on period of the duty cycle is greater than 4096 counts,
*   the LEDn_OFF count will occur in the next frame. Therefore, 4096 is subtracted from
*   the LEDn_OFF count to get the correct LEDn_OFF count.
*   Off time = 4CBh (decimal 3685 + 3686 = 7372 − 4096 = 3275)
*   LED4_OFF_H = 4h; LED4_OFF_L = CBh
*/



#pragma once
#include <memory>

#include "I2C.h"

namespace ioAdapter
{
    class PCA9685: public I2C::I2CSlave
    {
    public:
        PCA9685(const std::shared_ptr<I2C::I2CMaster>& master,
            const uint8_t addr = 0x40);
        // Delete the default copy constructor
        PCA9685(const PCA9685&) = delete;
        PCA9685& operator=(const PCA9685&) = delete;
        // Delete the default move constructor
        PCA9685(PCA9685&&) = delete;
        PCA9685& operator=(PCA9685&&) = delete;
        ~PCA9685() override;


        void test();

    private:

        enum Register {
            MODE1 = 0x00,         // Mode register 1
            MODE2 = 0x01,         // Mode register 2
            SUBADR1 = 0x02,       // I2C-bus subaddress 1 (7:1 A1[7:1] R/W, 0 A1[0] R only, default value : 1110 001* , A1[0] : 0*)
            SUBADR2 = 0x03,       // I2C-bus subaddress 2 (7:1 A2[7:1] R/W, 0 A2[0] R only, default value : 1110 010* , A2[0] : 0*)
            SUBADR3 = 0x04,       // I2C-bus subaddress 3 (7:1 A3[7:1] R/W, 0 A3[0] R only, default value : 1110 100* , A3[0] : 0*)
            ALLCALLADR = 0x05,    // LED All Call I2C-bus address (7:1 AC[7:1] R/W, default value : 1110 000* , AC[0] : 0*)
            LED0_ON_L = 0x06,     // LED0 output and brightness control byte 0
            LED0_ON_H = 0x07,     // LED0 output and brightness control byte 1
            LED0_OFF_L = 0x08,    // LED0 output and brightness control byte 2
            LED0_OFF_H = 0x09,    // LED0 output and brightness control byte 3
            LED1_ON_L = 0x0A,     // LED1 output and brightness control byte 0
            LED1_ON_H = 0x0B,     // LED1 output and brightness control byte 1
            LED1_OFF_L = 0x0C,    // LED1 output and brightness control byte 2
            LED1_OFF_H = 0x0D,    // LED1 output and brightness control byte 3
            LED2_ON_L = 0x0E,     // LED2 output and brightness control byte 0
            LED2_ON_H = 0x0F,     // LED2 output and brightness control byte 1
            LED2_OFF_L = 0x10,    // LED2 output and brightness control byte 2
            LED2_OFF_H = 0x11,    // LED2 output and brightness control byte 3
            LED3_ON_L = 0x12,     // LED3 output and brightness control byte 0
            LED3_ON_H = 0x13,     // LED3 output and brightness control byte 1
            LED3_OFF_L = 0x14,    // LED3 output and brightness control byte 2
            LED3_OFF_H = 0x15,    // LED3 output and brightness control byte 3
            LED4_ON_L = 0x16,     // LED4 output and brightness control byte 0
            LED4_ON_H = 0x17,     // LED4 output and brightness control byte 1
            LED4_OFF_L = 0x18,    // LED4 output and brightness control byte 2
            LED4_OFF_H = 0x19,    // LED4 output and brightness control byte 3
            LED5_ON_L = 0x1A,     // LED5 output and brightness control byte 0
            LED5_ON_H = 0x1B,     // LED5 output and brightness control byte 1
            LED5_OFF_L = 0x1C,    // LED5 output and brightness control byte 2
            LED5_OFF_H = 0x1D,    // LED5 output and brightness control byte 3
            LED6_ON_L = 0x1E,     // LED6 output and brightness control byte 0
            LED6_ON_H = 0x1F,     // LED6 output and brightness control byte 1
            LED6_OFF_L = 0x20,    // LED6 output and brightness control byte 2
            LED6_OFF_H = 0x21,    // LED6 output and brightness control byte 3
            LED7_ON_L = 0x22,     // LED7 output and brightness control byte 0
            LED7_ON_H = 0x23,     // LED7 output and brightness control byte 1
            LED7_OFF_L = 0x24,    // LED7 output and brightness control byte 2
            LED7_OFF_H = 0x25,    // LED7 output and brightness control byte 3
            LED8_ON_L = 0x26,     // LED8 output and brightness control byte 0
            LED8_ON_H = 0x27,     // LED8 output and brightness control byte 1
            LED8_OFF_L = 0x28,    // LED8 output and brightness control byte 2
            LED8_OFF_H = 0x29,    // LED8 output and brightness control byte 3
            LED9_ON_L = 0x2A,     // LED9 output and brightness control byte 0
            LED9_ON_H = 0x2B,     // LED9 output and brightness control byte 1
            LED9_OFF_L = 0x2C,    // LED9 output and brightness control byte 2
            LED9_OFF_H = 0x2D,    // LED9 output and brightness control byte 3
            LED10_ON_L = 0x2E,    // LED10 output and brightness control byte 0
            LED10_ON_H = 0x2F,    // LED10 output and brightness control byte 1
            LED10_OFF_L = 0x30,   // LED10 output and brightness control byte 2
            LED10_OFF_H = 0x31,   // LED10 output and brightness control byte 3
            LED11_ON_L = 0x32,    // LED11 output and brightness control byte 0
            LED11_ON_H = 0x33,    // LED11 output and brightness control byte 1
            LED11_OFF_L = 0x34,   // LED11 output and brightness control byte 2
            LED11_OFF_H = 0x35,   // LED11 output and brightness control byte 3
            LED12_ON_L = 0x36,    // LED12 output and brightness control byte 0
            LED12_ON_H = 0x37,    // LED12 output and brightness control byte 1
            LED12_OFF_L = 0x38,   // LED12 output and brightness control byte 2
            LED12_OFF_H = 0x39,   // LED12 output and brightness control byte 3
            LED13_ON_L = 0x3A,    // LED13 output and brightness control byte 0
            LED13_ON_H = 0x3B,    // LED13 output and brightness control byte 1
            LED13_OFF_L = 0x3C,   // LED13 output and brightness control byte 2
            LED13_OFF_H = 0x3D,   // LED13 output and brightness control byte 3
            LED14_ON_L = 0x3E,    // LED14 output and brightness control byte 0
            LED14_ON_H = 0x3F,    // LED14 output and brightness control byte 1
            LED14_OFF_L = 0x40,   // LED14 output and brightness control byte 2
            LED14_OFF_H = 0x41,   // LED14 output and brightness control byte 3
            LED15_ON_L = 0x42,    // LED15 output and brightness control byte 0
            LED15_ON_H = 0x43,    // LED15 output and brightness control byte 1
            LED15_OFF_L = 0x44,   // LED15 output and brightness control byte 2
            LED15_OFF_H = 0x45,   // LED15 output and brightness control byte 3
            ALL_LED_ON_L = 0xFA, // Load all the LEDn_ON registers, byte 0
            ALL_LED_ON_H = 0xFB, // Load all the LEDn_ON registers, byte 1
            ALL_LED_OFF_L = 0xFC,// Load all the LEDn_OFF registers, byte 0
            ALL_LED_OFF_H = 0xFD,// Load all the LEDn_OFF registers, byte 1
            PRE_SCALE = 0xFE,     // Prescaler for output frequency
            TESTMODE = 0xFF       // Defines the test mode to be entered
        };

        enum MODE1
        {
            // Bit 7: RESTART
            RESTART_0 = 0x00,           // Restart disabled (default)
            RESTART_1 = 0x80,           // Restart enabled

            // Bit 6: EXTCLK
            EXTCLK_0 = 0x00,            // Use internal clock (default)
            EXTCLK_1 = 0x40,            // Use EXTCLK pin clock

            // Bit 5: AI (Auto-Increment)
            AI_0 = 0x00,                // Register Auto-Increment disabled (default)
            AI_1 = 0x20,                // Register Auto-Increment enabled

            // Bit 4: SLEEP
            SLEEP_0 = 0x00,             // Normal mode (default)
            SLEEP_1 = 0x10,             // Low power mode (Oscillator off)

            // Bit 3: SUB1
            SUB1_0 = 0x00,              // PCA9685 does not respond to I2C-bus subaddress 1 (default)
            SUB1_1 = 0x08,              // PCA9685 responds to I2C-bus subaddress 1

            // Bit 2: SUB2
            SUB2_0 = 0x00,              // PCA9685 does not respond to I2C-bus subaddress 2 (default)
            SUB2_1 = 0x04,              // PCA9685 responds to I2C-bus subaddress 2

            // Bit 1: SUB3
            SUB3_0 = 0x00,              // PCA9685 does not respond to I2C-bus subaddress 3 (default)
            SUB3_1 = 0x02,              // PCA9685 responds to I2C-bus subaddress 3

            // Bit 0: ALLCALL
            ALLCALL_0 = 0x00,           // PCA9685 does not respond to LED All Call I2C-bus address (default)
            ALLCALL_1 = 0x01            // PCA9685 responds to LED All Call I2C-bus address
        };

        enum class MODE2 {
            RESERVED_BITS = 0x00,   // Reserved bits, read-only

            // Bit 4: Output logic state inversion
            INVRT = 0x10,           // Output logic state not inverted (default)
            OUTPUT_LOGIC_INVERTED = 0x00, // Output logic state inverted

            // Bit 3: Outputs change on STOP/ACK command
            OCH = 0x08,             // Outputs change on STOP command (default)
            OUTPUTS_CHANGE_ON_ACK = 0x00, // Outputs change on ACK command

            // Bit 2: Output driver configuration
            OUTDRV = 0x04,          // Totem pole structure for LED outputs (default)
            OPEN_DRAIN_STRUCTURE = 0x00, // Open-drain structure for LED outputs

            // Bits 1-0: Output state when drivers not enabled
            OUTNE_00 = 0x00,        // LEDn = 0 when OE = 1 (default)
            OUTNE_01 = 0x01,        // LEDn = 1 when OE = 1
            OUTNE_10 = 0x02,        // LEDn = high-impedance when OE = 1 (OUTDRV = 0)
        };


        //LEDx_ON_L,  LEDx_ON_H, LEDx_OFF_L and LEDx_OFF_H (LEDx_ON_L & LEDx_OFF_L accessible for 8 LSBs so no need to create an enum for them)
        enum class LEDx_ON_H{
            // Bit 7:5 are reserved
            RESERVED = -1,// Read only
            // Bit 4: LED0 full ON (default: 0)
            LED0_FULL_ON = 0b00000000,
            // Bit 3-0: LED0_ON count for LED0, 4 MSBs
        };

        enum class LEDx_OFF_H{
            // Bit 7:5 are reserved
            RESERVED,
            // Bit 4: LED0 full OFF (default: 1)
            LED0_FULL_OFF = 0b00010000,
            // Bit 3-0: LED0_OFF count for LED0, 4 MSBs
        };

        //ALL_LED_ON_L,  ALL_LED_ON_H, ALL_LED_OFF_L and ALL_LED_OFF_H and the prescale(ALL_LED_ON_L & ALL_LED_OFF_L accessible for 8 LSBs so no need to create an enum for them)
        enum class ALL_LED_ON_H {
            RESERVED,
            ALL_LED_FULL_ON = 0b00010000, // Bit 4: ALL_LED full ON (default: 1)
            LEDN_ON_COUNT_MSB
        };

        enum class ALL_LED_OFF_H {
            RESERVED,
            ALL_LED_FULL_OFF = 0b00010000, // Bit 4: ALL_LED full OFF (default: 1)
            LEDN_OFF_COUNT_MSB
        };

        enum class PRE_SCALE {
            PRE_SCALE_VALUE = 0x1E // Default prescale value: 30 (b0001 1110)
        };

        enum class softwareReset
        {
            Swrst = 0x06//software reset value
        };

        enum class AllCallI2C
        {
            AllCall = 0xE0,// Value to set the ALLCALLADR register (0xE0 or 1110 0000)
        };
    };
}
