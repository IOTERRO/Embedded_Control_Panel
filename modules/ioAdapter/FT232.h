#pragma once

//#define _USED

#ifdef _USED

#include <ftd2xx.h>
#include <map>
#include <memory>

#include "out.h"
#include "export.h"

namespace IoAdapter
{
    class IO_ADAPTER_API FT232: public io::Out
    {
    public:
        FT232();
        // Delete the default copy constructor
        FT232(const FT232&) = delete;
        FT232& operator=(const FT232&) = delete;
        // Delete the default move constructor
        FT232(FT232&&) = delete;
        FT232& operator=(FT232&&) = delete;
        ~FT232();


        bool pinMode(Gpio gpio, const PinMode mode) override;
        bool set(Gpio, GpioState) override;
        bool get(Gpio, GpioState&) override;

    private:
        bool init();
        void openHandle() const;
        void closeHandle() const;
        void clearAllPins() const;
        bool setEepromPins(Gpio, GpioState);
        bool configureEepromPins();

        //FT_HANDLE _handle;
        std::shared_ptr<FT_HANDLE> _handle = std::make_shared<FT_HANDLE>();
        std::map<Gpio, PinMode> _pinsMode  = {
                                                 {Gpio::D0, PinMode::Output}, {Gpio::D1, PinMode::Output},
                                                 {Gpio::D2, PinMode::Output}, {Gpio::D3, PinMode::Output},
                                                 {Gpio::D4, PinMode::Output}, {Gpio::D5, PinMode::Output},
                                                 {Gpio::D6, PinMode::Output}, {Gpio::D7, PinMode::Output},
                                                 {Gpio::C0, PinMode::Output}, {Gpio::C1, PinMode::Output},
                                                 {Gpio::C2, PinMode::Output}, {Gpio::C3, PinMode::Output},
                                                 {Gpio::C4, PinMode::Output}, {Gpio::C5, PinMode::Output},
                                                 {Gpio::C6, PinMode::Output}, {Gpio::C7, PinMode::Output},
                                                 {Gpio::C8, PinMode::Output}, {Gpio::C9, PinMode::Output}
                                               };

        std::map<Gpio, GpioState> _pinsState={
                                                 {Gpio::D0, GpioState::Low}, {Gpio::D1, GpioState::Low},
                                                 {Gpio::D2, GpioState::Low}, {Gpio::D3, GpioState::Low},
                                                 {Gpio::D4, GpioState::Low}, {Gpio::D5, GpioState::Low},
                                                 {Gpio::D6, GpioState::Low}, {Gpio::D7, GpioState::Low},
                                                 {Gpio::C0, GpioState::Low}, {Gpio::C1, GpioState::Low},
                                                 {Gpio::C2, GpioState::Low}, {Gpio::C3, GpioState::Low},
                                                 {Gpio::C4, GpioState::Low}, {Gpio::C5, GpioState::Low},
                                                 {Gpio::C6, GpioState::Low}, {Gpio::C7, GpioState::Low},
                                                 {Gpio::C8, GpioState::Low}, {Gpio::C9, GpioState::Low}
                                               };

    };
}
#endif // _USED
