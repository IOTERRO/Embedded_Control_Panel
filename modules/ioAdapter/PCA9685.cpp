#include "PCA9685.h"
#include <cmath>
#include <boost/winapi/thread.hpp>

using namespace ioAdapter;

static unsigned OscillatorFrequency = 25 * 1000000;//25us

PCA9685::PCA9685(const std::shared_ptr<I2C::I2CMaster>& master, const uint8_t addr):
    I2CSlave(master, addr)
{
}

PCA9685::~PCA9685()
{
    
}


void PCA9685::setPwmFrequency(const unsigned  freq)
{
    // Enable SLEEP mode(set bit 4)
    writeWord(Register::MODE1, MODE1::SLEEP_1);

    // Calculate prescale value
    const uint8_t prescale = static_cast<uint8_t>(round(OscillatorFrequency / (freq * 4096))) - 1;
   
    // Write prescale value to PRE_SCALE register
    writeWord(Register::PRE_SCALE, prescale);

    // Exit SLEEP mode
    writeWord(Register::MODE1, 0x00);

}


void  PCA9685::firePwm(const uint16_t pwmChannel, const double dutyCycle, const double delayTime)
{
    constexpr int period = 4096;  // Total period

    // Calculate Thigh (high time) for the desired duty cycle
    const int Thigh = static_cast<int>(round(period * dutyCycle / 100));

    // Calculate delay count
    const int delayCount =static_cast<int>(delayTime) * period;

    // Calculate the pulse width count
    const int pulseWidthCount = Thigh;

    // Set LED ON and OFF registers for the servo control
    const auto regs = selectPwmChannel(pwmChannel);
    writeWord(static_cast<uint8_t>(regs.at(0)), delayCount & 0xFF);
    writeWord(static_cast<uint8_t>(regs.at(1)), delayCount >> 8 & 0xFF);
    writeWord(static_cast<uint8_t>(regs.at(2)), (pulseWidthCount + delayCount) & 0xFF);
    writeWord(static_cast<uint8_t>(regs.at(3)), ((pulseWidthCount + delayCount) >> 8) & 0xFF);
}

//channel 0 to 15
std::vector<PCA9685::Register> PCA9685::selectPwmChannel(const uint16_t channelNumber)
{
    std::vector<Register> reg;
    if (channelNumber < 17)
    {
        switch (channelNumber)
        {
        case 0:
            {
                reg.emplace_back(LED0_ON_L);
                reg.emplace_back(LED0_ON_H);
                reg.emplace_back(LED0_OFF_L);
                reg.emplace_back(LED0_OFF_H);
            }
            break;
        case 1:
            {
                reg.emplace_back(LED1_ON_L);
                reg.emplace_back(LED1_ON_H);
                reg.emplace_back(LED1_OFF_L);
                reg.emplace_back(LED1_OFF_H);
            }
            break;
        case 2:
            {
                reg.emplace_back(LED2_ON_L);
                reg.emplace_back(LED2_ON_H);
                reg.emplace_back(LED2_OFF_L);
                reg.emplace_back(LED2_OFF_H);
                }
            break;
        case 3:
            {
                reg.emplace_back(LED3_ON_L);
                reg.emplace_back(LED3_ON_H);
                reg.emplace_back(LED3_OFF_L);
                reg.emplace_back(LED3_OFF_H);
            }
            break;
        case 4:
            {
                reg.emplace_back(LED4_ON_L);
                reg.emplace_back(LED4_ON_H);
                reg.emplace_back(LED4_OFF_L);
                reg.emplace_back(LED4_OFF_H);
            }
            break;
        case 5:
            {
                reg.emplace_back(LED5_ON_L);
                reg.emplace_back(LED5_ON_H);
                reg.emplace_back(LED5_OFF_L);
                reg.emplace_back(LED5_OFF_H);
            }
            break;
        case 6:
            {
                reg.emplace_back(LED6_ON_L);
                reg.emplace_back(LED6_ON_H);
                reg.emplace_back(LED6_OFF_L);
                reg.emplace_back(LED6_OFF_H);
            }
            break;
        case 7:
            {
                reg.emplace_back(LED7_ON_L);
                reg.emplace_back(LED7_ON_H);
                reg.emplace_back(LED7_OFF_L);
                reg.emplace_back(LED7_OFF_H);
            }
            break;
        case 8:
            {
                reg.emplace_back(LED8_ON_L);
                reg.emplace_back(LED8_ON_H);
                reg.emplace_back(LED8_OFF_L);
                reg.emplace_back(LED8_OFF_H);
            }
            break;
        case 9:
            {
                reg.emplace_back(LED9_ON_L);
                reg.emplace_back(LED9_ON_H);
                reg.emplace_back(LED9_OFF_L);
                reg.emplace_back(LED9_OFF_H);
            }                    
            break;
        case 10:
            {
                reg.emplace_back(LED10_ON_L);
                reg.emplace_back(LED10_ON_H);
                reg.emplace_back(LED10_OFF_L);
                reg.emplace_back(LED10_OFF_H);
            }
            break;
        case 11:
            {
                reg.emplace_back(LED11_ON_L);
                reg.emplace_back(LED11_ON_H);
                reg.emplace_back(LED11_OFF_L);
                reg.emplace_back(LED11_OFF_H);
            }
            break;
        case 12:
            {
                reg.emplace_back(LED12_ON_L);
                reg.emplace_back(LED12_ON_H);
                reg.emplace_back(LED12_OFF_L);
                reg.emplace_back(LED12_OFF_H);
            }
            break;
        case 13:
            {
                reg.emplace_back(LED13_ON_L);
                reg.emplace_back(LED13_ON_H);
                reg.emplace_back(LED13_OFF_L);
                reg.emplace_back(LED13_OFF_H);
            }
            break;
        case 14:
            {
                reg.emplace_back(LED14_ON_L);
                reg.emplace_back(LED14_ON_H);
                reg.emplace_back(LED14_OFF_L);
                reg.emplace_back(LED14_OFF_H);
            }
            break;
        case 15:
            {
                reg.emplace_back(LED15_ON_L);
                reg.emplace_back(LED15_ON_H);
                reg.emplace_back(LED15_OFF_L);
                reg.emplace_back(LED15_OFF_H);
            }
            break;
        default:
            {
                reg.emplace_back(UNDEFINED);
                reg.emplace_back(UNDEFINED);
                reg.emplace_back(UNDEFINED);
                reg.emplace_back(UNDEFINED);
            }
            break;
        }
    }

    return reg;
}