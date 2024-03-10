/**
 * Copyright (c) 2024 - FutureIsTech
 *
 * Author: Omar Terro
 *
 * All rights reserved
 *
 * Output IO Interface
 */

#pragma once

#include <boost/signals2.hpp>

namespace io
{

    class inOut
    {
    public:

        // Define GPIO pins enumeration in the interface
        enum class Gpio
        {
            //FT232H
            D0 = 0,
            D1 = 1,
            D2 = 2,
            D3 = 3,
            D4 = 4,
            D5 = 5,
            D6 = 6,
            D7 = 7,
            C0 = 8,
            C1 = 9,
            C2 = 10,
            C3 = 11,
            C4 = 12,
            C5 = 13,
            C6 = 14,
            C7 = 15,
            C8 = 16,
            C9 = 17
        };

        enum class PinMode
        {
            Output,
            Input,
            Sf,//Special Function : if the pin was used for i2c,spi,jtag...
        };

        enum class GpioState
        {
            Low = 0,
            High = 1,
            Unknown = -1
        };


        virtual ~inOut() = default;

        /**
         * @brief Configures the pin mode.
         *
         * @param gpio The GPIO pin.
         * @param mode The mode to set.
         * @return True if successful, false otherwise.
         */
        virtual bool pinMode(Gpio gpio, PinMode mode) = 0;


        /**
         * @brief Set the output value.
         *
         * @param gpio The GPIO pin.
         * @param state The state to set.
         * @return True if successful, false otherwise.
         */
        virtual bool set(Gpio gpio, GpioState state) = 0;

        /**
         * @brief Get the output value.
         *
         * @param gpio The GPIO pin.
         * @param state Reference to store the retrieved state.
         * @return True if successful, false otherwise.
         */
        virtual bool get(Gpio gpio, GpioState& state) = 0;


        /**
         * @brief Retrieves the state of the pins.
         *
         * @param pinsState Reference to a uint16_t variable where the state of the pins will be stored.
         *
         * @return True if the pins state was successfully retrieved, false otherwise.
         */
        //virtual bool getPinsState(uint16_t& pinsState) const = 0;

        /**
        * @brief In Observers
        * @note For RAII pattern see boost::signals2::scoped_connection
        */
        boost::signals2::signal<void(uint16_t /* value */)> valueChanged;

    };

} // namespace io
