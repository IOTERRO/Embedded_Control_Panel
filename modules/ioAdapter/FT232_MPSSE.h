/*
    Copyright (c) 2024 - FutureIsTech
    Author: Omar Terro
    Creation Date: 02/02/2024
    All rights reserved
    Project: Using FT232H in MPSSE I2C mode with libmpsse

    Description:
    The Multi-Protocol Synchronous Serial Engine, or MPSSE, is the heart of the FT232H chip, allowing it to speak many different protocols such as I2C, SPI, and more. When the chip is in MPSSE mode, it changes the D0 to D3 pins to have special serial protocol functions:
    - D0: Clock signal output. This line can be configured as a clock that runs at speeds between ~450Hz to 30MHz.
    - D1: Serial data output. This is for outputting a serial signal, like the MOSI line in an SPI connection.
    - D2: Serial data input. This is for reading a serial signal, like the MISO line in an SPI connection.
    - D3: Serial select signal. This is a chip select or chip enable signal to tell a connected device that the FT232H is ready to talk to it.

    In addition to the serial protocol pins above, the MPSSE mode allows you to control other pins as general-purpose digital inputs or outputs. These are great for controlling chip select, reset, or other lines on chips. You can even use the GPIO to read switches, blink LEDs, and more!

    The pins which are controllable as GPIO in MPSSE mode are D4 to D7 and C0 to C7, for a total of 12 GPIO pins. These pins can be configured individually as digital inputs or outputs.

    Note: Pins C8 and C9 are not controllable as GPIO pins. These two pins have a special function that can be set in the EEPROM of the chip - you'll learn more about these later.

    To use the MPSSE mode of the chip, you'll need to use some special software and programming libraries. There are actually a few options for software that can talk to the FT232H and its MPSSE component.

    Used Library: libmpsse (https://ftdichip.com/software-examples/mpsse-projects/libmpsse-i2c-examples/)

    FT232H Pinout Diagram (MPSSE I2C mode):

    +------+-------------------------+-----------------------------------+
    | Name | Description             | Function                          |
    +------+-------------------------+-----------------------------------+
    | D0   | I2C Clock (SCL)         | Output (Clock)                    |
    | D1   | I2C Data (SDA)          | Bidirectional (Data Out)          |
    | D2   | -                       | Bidirectional (Data In)           |
    | D3   | -                       | Output (Chip Select)              |
    | D4   | GPIO                    | Configurable (In/Out)             |
    | D5   | GPIO                    | Configurable (In/Out)             |
    | D6   | GPIO                    | Configurable (In/Out)             |
    | D7   | GPIO                    | Configurable (In/Out)             |
    | C0   | GPIO                    | Configurable (In/Out)             |
    | C1   | GPIO                    | Configurable (In/Out)             |
    | C2   | GPIO                    | Configurable (In/Out)             |
    | C3   | GPIO                    | Configurable (In/Out)             |
    | C4   | GPIO                    | Configurable (In/Out)             |
    | C5   | GPIO                    | Configurable (In/Out)             |
    | C6   | GPIO                    | Configurable (In/Out)             |
    | C7   | GPIO                    | Configurable (In/Out)             |
    +------+-------------------------+-----------------------------------+

    FT232H Pinout Diagram (EEPROM):

    +------+-------------------------+-----------------------------------+
    | Name | Description             | Function                          |
    +------+-------------------------+-----------------------------------+
    | C8   | -                       | Special Function (EEPROM)         |
    | C9   | -                       | Special Function (EEPROM)         |
    +------+-------------------------+-----------------------------------+
*/


#pragma once

#include <map>
#include <memory>
#include <boost/thread.hpp>
#include <shared_mutex>

#include "libftd2xx/ftd2xx.h"

#include "inout.h"
#include "export.h"


namespace IoAdapter
{
    class IO_ADAPTER_API FT232_MPSSE final : public io::inOut
    {
    public:
        FT232_MPSSE();
        // Delete the default copy constructor
        FT232_MPSSE(const FT232_MPSSE&) = delete;
        FT232_MPSSE& operator=(const FT232_MPSSE&) = delete;
        // Delete the default move constructor
        FT232_MPSSE(FT232_MPSSE&&) = delete;
        FT232_MPSSE& operator=(FT232_MPSSE&&) = delete;
        ~FT232_MPSSE() override;


        bool pinMode(Gpio gpio, const PinMode mode) override;
        bool set(Gpio, GpioState) override;
        bool get(Gpio, GpioState&) override;

    private:
        void doWork();
        int init();
        int openCahnnel();
        void closeHandle() const;
        bool clearAllPins();
        bool readAllPins(uint8_t cmd, uint8_t& result) const;
        bool getPinsState(uint16_t& pinsState) const;

        std::map<Gpio, PinMode> _pinsMode = {
                                                 {Gpio::D0, PinMode::Sf}, {Gpio::D1, PinMode::Sf},
                                                 {Gpio::D2, PinMode::Sf}, {Gpio::D3, PinMode::Sf},
                                                 {Gpio::D4, PinMode::Output}, {Gpio::D5, PinMode::Output},
                                                 {Gpio::D6, PinMode::Output}, {Gpio::D7, PinMode::Output},
                                                 {Gpio::C0, PinMode::Output}, {Gpio::C1, PinMode::Output},
                                                 {Gpio::C2, PinMode::Output}, {Gpio::C3, PinMode::Output},
                                                 {Gpio::C4, PinMode::Output}, {Gpio::C5, PinMode::Output},
                                                 {Gpio::C6, PinMode::Output}, {Gpio::C7, PinMode::Output},
                                                 {Gpio::C8, PinMode::Output}, {Gpio::C9, PinMode::Output}
        };

        std::map<Gpio, GpioState> _pinsState = {
                                                 {Gpio::D0, GpioState::Unknown}, {Gpio::D1, GpioState::Unknown},
                                                 {Gpio::D2, GpioState::Unknown}, {Gpio::D3, GpioState::Unknown},
                                                 {Gpio::D4, GpioState::Low}, {Gpio::D5, GpioState::Low},
                                                 {Gpio::D6, GpioState::Low}, {Gpio::D7, GpioState::Low},
                                                 {Gpio::C0, GpioState::Low}, {Gpio::C1, GpioState::Low},
                                                 {Gpio::C2, GpioState::Low}, {Gpio::C3, GpioState::Low},
                                                 {Gpio::C4, GpioState::Low}, {Gpio::C5, GpioState::Low},
                                                 {Gpio::C6, GpioState::Low}, {Gpio::C7, GpioState::Low},
                                                 {Gpio::C8, GpioState::Low}, {Gpio::C9, GpioState::Low}
        };

        uint8_t _dir{};// b0: Input , b1:  Output
        std::shared_ptr<FT_HANDLE> _handle = std::make_shared<FT_HANDLE>();

        boost::thread _thread;
        uint16_t _previousPinsState;
        mutable std::shared_mutex _mutex;

    };
}
