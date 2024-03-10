#pragma once

#include <memory>

#include "inout.h"
#include "export.h"

#include <boost/thread.hpp>

namespace ioAdapter
{
    class IO_ADAPTER_API ioHandler : public io::inOut
    {
    public:
        ioHandler(std::shared_ptr<io::inOut> device);
        // Delete the default copy constructor
        ioHandler(const ioHandler&) = delete;
        ioHandler& operator=(const ioHandler&) = delete;
        // Delete the default move constructor
        ioHandler(ioHandler&&) = delete;
        ioHandler& operator=(ioHandler&&) = delete;
        ~ioHandler() override;

        bool pinMode(Gpio gpio, PinMode mode) override;
        bool set(Gpio gpio, GpioState state) override;
        bool get(Gpio gpio, GpioState& state) override;

    private:
        std::shared_ptr<io::inOut> _device;
        std::mutex _mutex;
    };
}
