#include "ioHandler.h"

#include <iostream>

using namespace ioAdapter;

ioHandler::ioHandler(std::shared_ptr<inOut> device) :
    _device(std::move(device))
{
    _device->valueChanged.connect([&](auto state)
    {
        valueChanged(state);
    });
}

ioHandler::~ioHandler()
{
    _device.reset();
}

bool ioHandler::pinMode(const Gpio gpio, const PinMode mode)
{
    const std::lock_guard<std::mutex> lock(_mutex);

    if (_device == nullptr)
    {
        std::cerr << "Error: Device pointer is null." << std::endl;
        return false;
    }

    if (!_device->pinMode(gpio, mode))
    {
        std::cerr << "Error setting pin mode." << std::endl;
        return false;
    }

    return true;
}

bool ioHandler::set(const Gpio gpio, const GpioState state)
{
    const std::lock_guard<std::mutex> lock(_mutex);

    if (_device == nullptr)
    {
        std::cerr << "Error: Device pointer is null." << std::endl;
        return false;
    }

    if (!_device->set(gpio, state))
    {
        std::cerr << "Error setting pin state." << std::endl;
        return false;
    }

    return true;
}

bool ioHandler::get(const Gpio gpio, GpioState& state)
{
    const std::lock_guard<std::mutex> lock(_mutex);

    if (_device == nullptr)
    {
        std::cerr << "Error: Device pointer is null." << std::endl;
        return false;
    }

    if (!_device->get(gpio, state))
    {
        std::cerr << "Error getting pin state." << std::endl;
        return false;
    }

    return true;
}
