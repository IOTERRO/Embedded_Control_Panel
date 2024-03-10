#include <iostream>

#include "Bitwise.h"
#include "factory.h"

using namespace IoAdapter;
using namespace io;

// Get the device instance using the Factory class
const auto Device = Factory::getFt232H();

// Get the IO handler instance from the Factory class using the device instance obtained earlier
auto IoHandler = Factory::getIoHandler(Device);

static bool IsFlashButton = false;
static int FlashTime = 500 * 6;

void callback(const uint16_t state)
{
    if(Bitwise::getBitState(state, static_cast<int>(inOut::Gpio::D7)))
    {
        IsFlashButton = !IsFlashButton;
        FlashTime = IsFlashButton ? 500 : 3000;
        const std::string buttonState = IsFlashButton ? "High" : "LOW";
        std::cout << "State: D7 is " << buttonState << std::endl;
    }
}


int main()
{

    IoHandler->valueChanged.connect([&](auto state) { callback(state); });
    
    // Set pin D7 as input mode & C0 to output mode
    IoHandler->pinMode(inOut::Gpio::D7, inOut::PinMode::Input);
    IoHandler->pinMode(inOut::Gpio::C0, inOut::PinMode::Output);

    // Infinite loop to continuously read the state of pin D7 and drive C0 pin to low/high accordingly
    while (true)
    {
        if(IoHandler->set(inOut::Gpio::C0, inOut::GpioState::High))
        {
            std::cout << "State: C0 High" << std::endl;
        }
        Sleep(FlashTime);
        if(IoHandler->set(inOut::Gpio::C0, inOut::GpioState::Low))
        {
            std::cout << "State: C0 Low" << std::endl;
        }
        Sleep(FlashTime);
    }
    return 0;
}
