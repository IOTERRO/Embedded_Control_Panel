#include "PCA9685.h"

using namespace ioAdapter;

PCA9685::PCA9685(const std::shared_ptr<I2C::I2CMaster>& master, const uint8_t addr):
    I2CSlave(master, addr)
{
    
}

PCA9685::~PCA9685()
{
    
}

void PCA9685::test()
{
    // Set MODE1 register for normal operation
    writeWord(Register::MODE1, static_cast<uint8_t>(MODE1::AI_1)); // Enable auto-increment

    // Set prescale value for PWM frequency (set to default 50Hz)
    /*
     *Formula to calculate PWM frequency: Oscillator Frequency / (4096 * prescale value)
     *In this case, with a 25MHz internal oscillator and a prescale value of 30, the PWM frequency would be: 25 * 10000000 / (30*4096) = 50 hz
     *Formula to calculate the prescale value:  Prescale Value = (Oscillator Frequency / (PWM Frequency * 4096)) - 1
    */

    writeWord(Register::PRE_SCALE, static_cast<uint8_t>(PRE_SCALE::PRE_SCALE_VALUE));


    // Set MODE2 register for output driver configuration (open-drain)
    writeWord(Register::MODE2, static_cast<uint8_t>(MODE2::OPEN_DRAIN_STRUCTURE));

    // Set PWM frequency (approximately 50Hz)
    // Calculation: 25MHz / (prescale * 4096) = 50Hz
    // Prescale = 30 (0x1E)
    // Note: For a different PWM frequency, adjust the prescale value accordingly

    // Configure PWM for servo motor on channel 0 (PWM pin 0)
    // Servo motors typically have a pulse width between 1000μs to 2000μs (1ms to 2ms)
    // Adjust the PWM duty cycle range to match the servo's requirements

    // Set ON count for PWM signal (servo start of the pulse)
    writeWord(Register::LED0_ON_L, 0x00); // ON count low byte
    writeWord(Register::LED0_ON_H, 0x00); // ON count high byte


    // Set OFF count for PWM signal (servo end of the pulse)
    // Example: for a 1.5ms pulse width (neutral position)
    constexpr uint16_t pulseWidth = 1500; // 1500μs
    constexpr uint16_t offCount = (pulseWidth * 4096) / 20000; // 20000μs = 50Hz
    writeWord(Register::LED0_OFF_L, offCount & 0xFF); // OFF count low byte
    writeWord(Register::LED0_OFF_H, (offCount >> 8) & 0x0F); // OFF count high byte


}
