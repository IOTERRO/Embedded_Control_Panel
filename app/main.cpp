#include <iostream>

#include "Bitwise.h"
#include "factory.h"

using namespace IoAdapter;
using namespace io;

// Get the device instance using the Factory class
const auto Device = Factory::getFt232H();

// Get the IO handler instance from the Factory class using the device instance obtained earlier
auto IoHandler = Factory::getIoHandler(Device);

auto pwmDriver = Factory::getPwmDriver(Device);

static bool IsFlashButton = false;
static int FlashTime = 500 * 6;

auto inputPin = inOut::Gpio::C1;
auto outputPin = inOut::Gpio::D7;

void callback(const uint16_t state)
{
    if(Bitwise::getBitState(state, static_cast<int>(inputPin)))
    {
        //IsFlashButton = !IsFlashButton;
        //FlashTime = IsFlashButton ? 500 : 3000;
        //const std::string buttonState = IsFlashButton ? "High" : "LOW";
        std::cout << "State pin is " << "High" << std::endl;
    }
    else
    {
        std::cout << "State pin is " << "LOW" << std::endl;
    }
}

void pwmOperation()
{
    while (true)
    {
        //pwmDriver->test(); // Execute PWM operation
        pwmDriver->firePwm(0, 20, 0);//generate pwm on channel 0 ( dutyCycle = 88%)
        pwmDriver->firePwm(1, 20, 0);//generate pwm on channel 1 ( dutyCycle = 13%)
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}

void setupPwm()
{
    if(pwmDriver != nullptr)
    {
        pwmDriver->setPwmFrequency(50);
        pwmDriver->firePwm(0, 88, 0);//generate pwm on channel 0 ( dutyCycle = 88%)
        pwmDriver->firePwm(1, 13, 0);//generate pwm on channel 1 ( dutyCycle = 13%)
    }
}



int main()
{

    IoHandler->valueChanged.connect([&](auto state) { callback(state); });
    
    // Set pin D7 as input mode & C0 to output mode
    IoHandler->pinMode(inputPin, inOut::PinMode::Input);
    IoHandler->pinMode(outputPin, inOut::PinMode::Output);

    //pwm
    setupPwm();
    std::thread pwmThread(pwmOperation);

    // Infinite loop to continuously read the state of pin D7 and drive C0 pin to low/high accordingly
    while (true)
    {
        if(IoHandler->set(outputPin, inOut::GpioState::High))
        {
            std::cout << "State: C0 High" << std::endl;
        }
        Sleep(1000);
        if(IoHandler->set(outputPin, inOut::GpioState::Low))
        {
            std::cout << "State: C0 Low" << std::endl;
        }
        Sleep(1000);
    }

    pwmThread.join();
    return 0;
}
