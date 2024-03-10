/*
    Copyright (c) 2024 - FutureIsTech
    Author: Omar Terro
    Creation Date: 02/02/2024
    All rights reserved

    This class implements an adapter to control the GPIO pins of an FT232H in MPSSE mode.
    It is designed to be used with devices requiring an I2C interface controlled via GPIO.

    Usage Constraints:
    - This code is specific to the FT232H adapter and is intended for use in applications requiring I2C communication via GPIO.
    - The GPIO pins specified in this code must be properly configured as input or output as needed for your application.
    - Ensure that the libMPSSE library is correctly installed and configured on your system.
*/


#include <iostream>

#include "FT232_MPSSE.h"

#include "libmpsse_i2c.h"
#include "Bitwise.h"


enum class PinsDir
{
    In = 0,
    Out
};

enum class GpioValue {
    Low = 0,
    High
};

enum class I2cPinsConfig
{
    C0 = 0,
    C1,
    C2,
    C3,
    C4,
    C5,
    C6,
    C7
};


constexpr PinsDir InitialDir = PinsDir::Out;
constexpr GpioValue InitialVal = GpioValue::High;
constexpr PinsDir  FinalDir = PinsDir::Out;
constexpr GpioValue FinalVal = GpioValue::Low;

#define InitialDirection (((static_cast<uint8_t>(InitialDir) << static_cast<uint8_t>(I2cPinsConfig::C0)) | \
                            (static_cast<uint8_t>(InitialDir) << static_cast<uint8_t>(I2cPinsConfig::C1)) | \
                            (static_cast<uint8_t>(InitialDir) << static_cast<uint8_t>(I2cPinsConfig::C2)) | \
                            (static_cast<uint8_t>(InitialDir) << static_cast<uint8_t>(I2cPinsConfig::C3)) | \
                            (static_cast<uint8_t>(InitialDir) << static_cast<uint8_t>(I2cPinsConfig::C4)) | \
                            (static_cast<uint8_t>(InitialDir) << static_cast<uint8_t>(I2cPinsConfig::C5)) | \
                            (static_cast<uint8_t>(InitialDir) << static_cast<uint8_t>(I2cPinsConfig::C6)) | \
                            (static_cast<uint8_t>(InitialDir) << static_cast<uint8_t>(I2cPinsConfig::C7))))

#define InitialValues (((static_cast<uint8_t>(InitialVal) << static_cast<uint8_t>(I2cPinsConfig::C0)) | \
                        (static_cast<uint8_t>(InitialVal) << static_cast<uint8_t>(I2cPinsConfig::C1)) | \
                        (static_cast<uint8_t>(InitialVal) << static_cast<uint8_t>(I2cPinsConfig::C2)) | \
                        (static_cast<uint8_t>(InitialVal) << static_cast<uint8_t>(I2cPinsConfig::C3)) | \
                        (static_cast<uint8_t>(InitialVal) << static_cast<uint8_t>(I2cPinsConfig::C4)) | \
                        (static_cast<uint8_t>(InitialVal) << static_cast<uint8_t>(I2cPinsConfig::C5)) | \
                        (static_cast<uint8_t>(InitialVal) << static_cast<uint8_t>(I2cPinsConfig::C6)) | \
                        (static_cast<uint8_t>(InitialVal) << static_cast<uint8_t>(I2cPinsConfig::C7))))

#define FinalDirection (((static_cast<uint8_t>(FinalDir) << static_cast<uint8_t>(I2cPinsConfig::C0)) | \
                          (static_cast<uint8_t>(FinalDir) << static_cast<uint8_t>(I2cPinsConfig::C1)) | \
                          (static_cast<uint8_t>(FinalDir) << static_cast<uint8_t>(I2cPinsConfig::C2)) | \
                          (static_cast<uint8_t>(FinalDir) << static_cast<uint8_t>(I2cPinsConfig::C3)) | \
                          (static_cast<uint8_t>(FinalDir) << static_cast<uint8_t>(I2cPinsConfig::C4)) | \
                          (static_cast<uint8_t>(FinalDir) << static_cast<uint8_t>(I2cPinsConfig::C5)) | \
                          (static_cast<uint8_t>(FinalDir) << static_cast<uint8_t>(I2cPinsConfig::C6)) | \
                          (static_cast<uint8_t>(FinalDir) << static_cast<uint8_t>(I2cPinsConfig::C7))))


#define FinalValues	  static_cast<uint8_t>(FinalVal)



enum class MpsseCommand : uint8_t {
    SetDataBitsLowbyte = 0x80,
    SetDataBitsHighbyte = 0x82,
    GetDataBitsLowbyte = 0x81,
    GetDataBitsHighbyte	= 0x83,
    SendImmediate	= 0x87
};


using namespace IoAdapter;

FT232_MPSSE::FT232_MPSSE():
    _thread(boost::thread(&FT232_MPSSE::doWork, this)),
    _previousPinsState(0)
{
    init();
}

FT232_MPSSE::~FT232_MPSSE()
{
    clearAllPins();
    Cleanup_libMPSSE();
    closeHandle();
}

bool FT232_MPSSE::pinMode(const Gpio gpio, const PinMode mode)
{
    // Exclusive lock for write access
    const std::unique_lock<std::shared_mutex> lock(_mutex);

    if (_handle == nullptr)
        return false;

    if (_pinsMode.at(gpio) == PinMode::Sf)
        return false;

    // Read current configuration
    uint32_t currentMask = 0xFFFFFFFF;
    for (const auto& [pinNumber, pinMode] : _pinsMode)
    {
        if (pinMode == PinMode::Output)
        {
            Bitwise::setBit(currentMask, static_cast<int>(pinNumber));
        }
        else
        {
            Bitwise::clearBit(currentMask, static_cast<int>(pinNumber));
        }
    }

    if (static_cast<int>(gpio) > 7)
    {
        //select C0:C7 (Most significant bit)
        _dir = currentMask >> 8 & 0xFF;//ex: b00001001 01000000 --> selection >> 8 & 0xFF : b01000000
    }
    else
    {
        //select D0:D7 (Low significant bit)
        _dir = currentMask & 0xF0;//ex: b00001001 01000000 --> selection & 0xF0 : b00001001 & maintain the first 4 bits to 0 because there has a special functions with i2c ( clck, data...)
    }

    _pinsMode.at(gpio) = mode;
    return true;
}

/*
   Exemples:
   -------
   0x80: Mpsse command to set D[7:0].
   0x00: Output values for D[7:0] (placeholder)
   0xFF: GPIO directions for D[7:0] (1 = output)

   0x82: Mpsse command to set C[7:0].
   0x01: Output values for C[7:0] (placeholder)
   0x01: GPIO directions for C[7:0] (1 = output)
 
 */
bool FT232_MPSSE::set(Gpio gpio, const GpioState state)
{
    // Exclusive lock for write access
    const std::unique_lock<std::shared_mutex> lock(_mutex);

    if (_handle == nullptr)
        return false;

    // Check if the specified pin is a valid I/O pin and configured as an output
    if (_pinsState.at(gpio) == GpioState::Unknown || _pinsMode.at(gpio) != PinMode::Output)
    {
        std::cerr << "Pin " << static_cast<int>(gpio) << " is not configured is not a valid I/O pin or is not "
                                                         " configured as an output" << std::endl;
        return false;
    }

    // Read current configuration
    uint32_t currentMask = 0x00;
    for (const auto& [pinNumber, pinState] : _pinsState)    
    {
        if (pinState == GpioState::High)
        {
            Bitwise::setBit(currentMask, static_cast<int>(pinNumber));
        }
        else
        {
            Bitwise::clearBit(currentMask, static_cast<int>(pinNumber));
        }
    }

    auto newMask = currentMask;

    //Handle the requested state
    if (state == GpioState::High)
    {
        // Set bit for OUTPUT mode
        Bitwise::setBit(newMask, static_cast<int>(gpio));
    }
    else
    {
        // Clear bit for INPUT mode
        Bitwise::clearBit(newMask, static_cast<int>(gpio));
    }

    uint8_t gpioCommand[3];
    if(static_cast<int>(gpio) > 7)
    {
        gpioCommand[0] = static_cast<uint8_t>(MpsseCommand::SetDataBitsHighbyte);//[C0:C7)
        //select C0:C7 (Most significant bit)
        gpioCommand[1] = newMask >> 8 & 0xFF;//ex: b00001001 01000000 --> selection >> 8 & 0xFF : b01000000
    }
    else
    {
        gpioCommand[0] = static_cast<uint8_t>(MpsseCommand::SetDataBitsLowbyte);//[D0:D7)
        //select D0:D7 (Low significant bit)
        gpioCommand[1] = newMask & 0xF0;//ex: b00001001 01000000 --> selection & 0xF0 : b00001001 & maintain the first 4 bits to 0 because there has a special functions with i2c ( clck, data...)
    }

    gpioCommand[2] = _dir;

    unsigned long bytesWritten = 0;
    const auto status = FT_Write(*_handle, gpioCommand, sizeof(gpioCommand), &bytesWritten);
    if (status != FT_OK) {
        std::cerr << "Failed to write to GPIO (error code: " << status << ")" << std::endl;
        closeHandle();
        return false;
    }

    _pinsState.at(gpio) = state;
    return true;
}

bool FT232_MPSSE::get(Gpio gpio, GpioState& state)
{
    if (_handle == nullptr)
        return false;

    // Check if the specified pin is configured as an input
    if (_pinsMode.at(gpio) != PinMode::Input || _pinsState.at(gpio) == GpioState::Unknown)
    {
        std::cerr << "Pin " << static_cast<int>(gpio) << " is not configured as an input" << std::endl;
        return false;
    }

    UCHAR readBuffer[10];

    if (static_cast<int>(gpio) > 7)
    {
        if(readAllPins(static_cast<uint8_t>(MpsseCommand::GetDataBitsHighbyte), *readBuffer))
        {
            state = Bitwise::getBitState(readBuffer[0], static_cast<int>(gpio) - 8) ? GpioState::High : GpioState::Low;
            return true;
        }
    }
    else
    {
        if(readAllPins(static_cast<uint8_t>(MpsseCommand::GetDataBitsLowbyte), *readBuffer))
        {
            state = Bitwise::getBitState(readBuffer[0], static_cast<int>(gpio)) ? GpioState::High : GpioState::Low;
            return true;
        }
    }

    std::cerr << "Error reading pin " << static_cast<int>(gpio) << std::endl;
    return false;
}


bool FT232_MPSSE::getPinsState(uint16_t& pinsState) const
{
    if (_handle == nullptr)
        return false;

    UCHAR readBufferHigh[10];
    UCHAR readBufferLow[10];


    if(!readAllPins(static_cast<uint8_t>(MpsseCommand::GetDataBitsHighbyte), *readBufferHigh))
    {
        std::cerr << "Error reading C0:C7 High byte pins" << std::endl;
        return false;
    }

    Sleep(100);

    if (!readAllPins(static_cast<uint8_t>(MpsseCommand::GetDataBitsLowbyte), *readBufferLow))
    {
        std::cerr << "Error reading D0:D7 Low byte pins" << std::endl;
        return false;
    }


    pinsState = static_cast<uint16_t>(readBufferLow[0]) | static_cast<uint16_t>(readBufferHigh[0] << 8);

    return true;

}

bool FT232_MPSSE::clearAllPins()
{
    // Exclusive lock for write access
    const std::unique_lock<std::shared_mutex> lock(_mutex);

    //Clear all D4:D7 pins
    // 0x80: Mpsse Command to set D[7:0].
    // 0x00: Output values for D[7:0] (placeholder)
    // 0xF0: GPIO directions for D[7:0] (1 = output, 0 = input)
    uint8_t gpioCommand[] = { 0x80, 0x00, 0xF0 };
    unsigned long bytesWritten = 0;
    auto status = FT_Write(*_handle, gpioCommand, sizeof(gpioCommand), &bytesWritten);
    if (status != FT_OK) {
        std::cerr << "Failed to write to GPIO (error code: " << status << ")" << std::endl;
        closeHandle();
        return false;
    }

    //Clear all c0:c7 pins
    // 0x82: Mpsse Command to set C[7:0].
    // 0x00: Output values for C[7:0] (placeholder)
    // 0xFF: GPIO directions for C[7:0] (1 = output)
    uint8_t buffer[3] = { 0x82, 0x00, 0xFF };
    status = FT_Write(*_handle, buffer, sizeof(buffer), &bytesWritten);
    if (status != FT_OK) {
        std::cerr << "Failed to write to GPIO (error code: " << status << ")" << std::endl;
        closeHandle();
        return false;
    }

    return true;
}

bool FT232_MPSSE::readAllPins(uint8_t cmd, uint8_t& result) const
{
    // Exclusive lock for write access
    const std::unique_lock<std::shared_mutex> lock(_mutex);

    uint8_t buffer[2];
    DWORD bytesTransfered = 0;
    DWORD bytesToTransfer = 0;
    UCHAR readBuffer[10];

    buffer[bytesToTransfer++] = cmd;
    buffer[bytesToTransfer++] = static_cast<uint8_t>(MpsseCommand::SendImmediate);

    FT_STATUS status = FT_Write(*_handle, buffer, bytesToTransfer, &bytesTransfered);
    if (status != FT_OK) {
        std::cerr << "Failed to write to GPIO (error code: " << status << ")" << std::endl;
        closeHandle();
        return false;
    }

    bytesToTransfer = 1;
    bytesTransfered = 0;
    status = FT_Read(*_handle, readBuffer, bytesToTransfer, &bytesTransfered);
    if (status != FT_OK) {
        std::cerr << "Failed to read pin D0 state ---> error code(" << status << ")" << std::endl;
        closeHandle();
        return false;
    }

    if (bytesToTransfer != bytesTransfered)
    {
        std::cerr << "bytesToTransfer different then bytesTransfered (" << FT_IO_ERROR << ")" << std::endl;
        return false;
    }

    result = readBuffer[0];
    return true;
}

int FT232_MPSSE::openCahnnel()
{
    // Opening the I2C channel
    constexpr DWORD channelIndex = 0; // Index of the I2C channel to open
    const FT_STATUS status = I2C_OpenChannel(channelIndex, _handle.get());
    if (status != FT_OK)
    {
        std::cerr << "Error opening I2C channel." << std::endl;
        return -1;
    }
    return 0;
}

void FT232_MPSSE::closeHandle() const
{
    I2C_CloseChannel(*_handle);
}

int FT232_MPSSE::init()
{
    // Initializing the libMPSSE library
    Init_libMPSSE();

    //Open channel
    openCahnnel();

    // I2C channel configuration
    ChannelConfig config;
    config.currentPinState = 0; // Current pin status (not used for I2C)
    config.LatencyTimer = 1;
    config.Pin = InitialDirection | /* BIT7   -BIT0:   Initial direction of the pins	*/
                 InitialValues << 8 | /* BIT15 -BIT8:   Initial values of the pins		*/
                 FinalDirection << 16 | /* BIT23 -BIT16: Final direction of the pins		*/
                 FinalValues << 24;
    /* initial dir and values are used for initchannel API and final dir and values are used by CloseChannel API */

    config.Options = I2C_ENABLE_PIN_STATE_CONFIG; /* set this option to enable GPIO_Lx pinstate management */
    config.ClockRate = I2C_CLOCK_STANDARD_MODE;

    const auto status = I2C_InitChannel(*_handle, &config);
    if (status != FT_OK)
    {
        std::cerr << "Error configuring I2C channel." << std::endl;
        closeHandle();
        return -1;
    }

    clearAllPins();

    return 0;
}

void FT232_MPSSE::doWork()
{
    uint16_t pinsState = 0;

    while (true)
    {
        Sleep(200);
        if(_handle != nullptr)
        {
            if (getPinsState(pinsState))
            {
                bool done = false;
                for (const auto& [pinNumber, pinMode] : _pinsMode)
                {
                    if (pinMode == PinMode::Input)
                    {
                        if(Bitwise::getBitState(pinsState, static_cast<int>(pinNumber)) != Bitwise::getBitState(_previousPinsState, static_cast<int>(pinNumber)))
                        {
                            _previousPinsState = pinsState;
                            done = true;
                        }
                    }
                }
                if(done)
                {
                    valueChanged(pinsState);
                }
            }
            else
            {
                Sleep(1000);
                init();
            }
        }
    }
}
