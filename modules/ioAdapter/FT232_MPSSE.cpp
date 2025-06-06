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

#define I2C_WRITE_COMPLETION_RETRY 10

using namespace IoAdapter;

FT232_MPSSE::FT232_MPSSE():
    _handle(nullptr),
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
    //const std::lock_guard<std::recursive_mutex> lock(_mutex);

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

    const auto status = writeToDevice(gpioCommand, sizeof(gpioCommand), bytesWritten);
    // auto status = FT_Write(_handle, gpioCommand, sizeof(gpioCommand), &bytesWritten);
    if (status != true) {
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

int FT232_MPSSE::setSpeed(I2CMaster::Speed speed)
{
    // I2C channel configuration
    ChannelConfig channelConf;
    channelConf.currentPinState = 0; // Current pin status (not used for I2C)
    channelConf.LatencyTimer = 16;
    //channelConf.Pin = InitialDirection | /* BIT7   -BIT0:   Initial direction of the pins	*/
    //    InitialValues << 8 | /* BIT15 -BIT8:   Initial values of the pins		*/
    //    FinalDirection << 16 | /* BIT23 -BIT16: Final direction of the pins		*/
    //    FinalValues << 24;
    /* initial dir and values are used for initchannel API and final dir and values are used by CloseChannel API */
    //channelConf.Pin = (1 << 0) | (1 << 1); // Set D0 and D1 as I2C pins
    channelConf.Pin = 0;
    channelConf.Options = 0; /* set this option to enable GPIO_Lx pinstate management */
   
    switch (speed)
    {
    case Speed::_10kbs:
        return -1;
    case Speed::_100kbs:
        channelConf.ClockRate = I2C_CLOCK_STANDARD_MODE;
        break;
    case Speed::_200kbs:
        return -1;
    case Speed::_400kbs:
        channelConf.ClockRate = I2C_CLOCK_FAST_MODE;
        break;
    case Speed::_1mbs:
        channelConf.ClockRate = I2C_CLOCK_FAST_MODE_PLUS;
        break;
    case Speed::_17mbs:
        return -1;
    case Speed::_34mbs:
        channelConf.ClockRate = I2C_CLOCK_HIGH_SPEED_MODE;
        break;
    }

    const auto status = I2C_InitChannel(_handle, &channelConf);
    if (status != FT_OK)
    {
        std::cerr << "Error configuring I2C channel." << std::endl;
        closeHandle();
        return -1;
    }

    return 0;
}

int FT232_MPSSE::readWord(const uint8_t addr, uint8_t cmd, uint16_t& value)
{
    if (_handle == nullptr)
    {
        std::cerr << "Need to be initialized before use" << std::endl;
        return -1;
    }

    DWORD xfer = 0;
    FT_STATUS status = I2C_DeviceWrite(_handle, addr, sizeof(cmd), &cmd, &xfer,
        I2C_TRANSFER_OPTIONS_START_BIT |
        I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES
    );
    if (status != FT_OK || xfer != sizeof(cmd))
    {
        closeHandle();
        return -1;
    }
    /* Repeated Start condition generated. */
    uint8_t data[2] = { 0, 0 };
    xfer = 0;
    status = I2C_DeviceRead(_handle, addr, sizeof(data), data, &xfer,
        I2C_TRANSFER_OPTIONS_START_BIT |
        I2C_TRANSFER_OPTIONS_STOP_BIT |
        I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE |
        I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES
    );

    if ((status != FT_OK) || (xfer != sizeof(data)))
    {
        std::cerr << "FT232_ReadWord : Error(" << status << ")" << std::endl;
        closeHandle();
        return -1;
    }
    value = static_cast<uint16_t>(data[0]) + static_cast<uint16_t>(data[1] << 8);
    return 0;
}


int FT232_MPSSE::writeWord(const uint8_t slaveAddress, const uint8_t cmd, const uint16_t value)
{
    const std::unique_lock<std::shared_mutex> lock(_mutex);
    if (_handle == nullptr)
    {
        std::cerr << "Need to be initialized before use" << std::endl;
        return -1;
    }

    uint8_t buffer[3];
    uint32_t bytesTransfered;
    bool writeComplete = false;
    uint32_t retry = 0;
    uint32_t bytesToTransfer = 0;
    bytesTransfered = 0;
    buffer[bytesToTransfer++] = cmd; /* Byte addressed inside EEPROM */
    buffer[bytesToTransfer++] = static_cast<uint8_t>(value);
    auto status = I2C_DeviceWrite(_handle, slaveAddress, bytesToTransfer, buffer,
                                  reinterpret_cast<LPDWORD>(&bytesTransfered),
                                  I2C_TRANSFER_OPTIONS_START_BIT | 
                                  I2C_TRANSFER_OPTIONS_STOP_BIT);

    if ((status != FT_OK) || (bytesTransfered != bytesToTransfer))
    {
        std::cerr << "FT232_ReadWord : Error(" << status << ")" << std::endl;
        closeHandle();
        return -1;
    }

    ///* poll to check completion */
    //while ((writeComplete == false) && (retry < I2C_WRITE_COMPLETION_RETRY))
    //{
    //    bytesToTransfer = 0;
    //    bytesTransfered = 0;
    //    buffer[bytesToTransfer++] = slaveAddress; /* Addressed inside EEPROM */
    //    status = I2C_DeviceWrite(_handle, slaveAddress, bytesToTransfer,
    //                             buffer, reinterpret_cast<LPDWORD>(&bytesTransfered),
    //                             I2C_TRANSFER_OPTIONS_START_BIT | 
    //                             I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES);
    //    if ((FT_OK == status) && (bytesToTransfer == bytesTransfered))
    //    {
    //        writeComplete = true;
    //    }
    //    else
    //    {
    //        std::cerr << " ... Write Failed" << std::endl;
    //        return -1;
    //    }
    //    retry++;
    //}
    return 0;
}

bool FT232_MPSSE::getPinsState(uint16_t& pinsState)
{
    if (_handle == nullptr)
    {
        std::cerr << "Need to be initialized before use" << std::endl;
        return false;
    }

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

bool FT232_MPSSE::writeToDevice(uint8_t *buffer, DWORD bytesToTransfer, DWORD& bytesTransfered)
{
    if (_handle == nullptr)
    {
        std::cerr << "Need to be initialized before use" << std::endl;
        return false;
    }

    const std::unique_lock<std::shared_mutex> lock(_mutex);
    if (const auto status = FT_Write(_handle, buffer, bytesToTransfer, &bytesTransfered); status != FT_OK) {
        return false;
    }
    return true;
}

bool FT232_MPSSE::clearAllPins()
{

    if (_handle == nullptr)
    {
        std::cerr << "Need to be initialized before use" << std::endl;
        return false;
    }

    //Clear all D4:D7 pins
    // 0x80: Mpsse Command to set D[7:0].
    // 0x00: Output values for D[7:0] (placeholder)
    // 0xF0: GPIO directions for D[7:0] (1 = output, 0 = input)
    uint8_t gpioCommand[] = { 0x80, 0x00, 0xF0 };
    unsigned long bytesWritten = 0;
    auto status = FT_Write(_handle, gpioCommand, sizeof(gpioCommand), &bytesWritten);
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
    status = FT_Write(_handle, buffer, sizeof(buffer), &bytesWritten);
    if (status != FT_OK) {
        std::cerr << "Failed to write to GPIO (error code: " << status << ")" << std::endl;
        closeHandle();
        return false;
    }

    return true;
}

bool FT232_MPSSE::readAllPins(uint8_t cmd, uint8_t& result)
{
    if (_handle == nullptr)
    {
        std::cerr << "Need to be initialized before use" << std::endl;
        return false;
    }

    uint8_t buffer[2];
    DWORD bytesTransfered = 0;
    DWORD bytesToTransfer = 0;
    UCHAR readBuffer[10];

    buffer[bytesToTransfer++] = cmd;
    buffer[bytesToTransfer++] = static_cast<uint8_t>(MpsseCommand::SendImmediate);

    auto status = writeToDevice(buffer, bytesToTransfer, bytesTransfered);
    if (status != true) {
        std::cerr << "Failed to write to GPIO (error code: " << status << ")" << std::endl;
        closeHandle();
        return false;
    }
  
    bytesToTransfer = 1;
    bytesTransfered = 0;
    {
        const std::unique_lock<std::shared_mutex> lock(_mutex);
        status = FT_Read(_handle, readBuffer, bytesToTransfer, &bytesTransfered);
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
    }

    return true;
}

int FT232_MPSSE::openCahnnel(const DWORD channelIndex)
{
    // Opening the I2C channel
    const FT_STATUS status = I2C_OpenChannel(channelIndex, &_handle);
    if (status != FT_OK)
    {
        std::cerr << "Error opening I2C channel." << std::endl;
        return -1;
    }
    
    return 0;
}

void FT232_MPSSE::closeHandle()
{
    I2C_CloseChannel(_handle);
    _handle = nullptr;
}

int FT232_MPSSE::init()
{
    const std::unique_lock<std::shared_mutex> lock(_mutex);

    // Initializing the libMPSSE library
    Init_libMPSSE();
   
    if (_handle == nullptr)
    {
        unsigned long channels = 0;
        auto status = I2C_GetNumChannels(&channels);

        const DWORD channelIndex = 0;
        for (unsigned long channel = 0; channel < channels; channel++)
        {
            FT_DEVICE_LIST_INFO_NODE devList;
            status = I2C_GetChannelInfo(channel, &devList);
            Sleep(1);
            std::cout << "\t\t\t\t\t\tI2C_GetNumChannels returned " << status << " for channel = " << channel << std::endl;
            /*print the dev info*/
            std::cout << "\t\t\t\t\t\tFlags=0x" << devList.Flags << std::endl;
            std::cout << "\t\t\t\t\t\tType=0x" << devList.Type << std::endl;
            std::cout << "\t\t\t\t\t\tID=0x" << devList.ID << std::endl;
            std::cout << "\t\t\t\t\t\tLocId=0x" << devList.LocId << std::endl;
            std::cout << "\t\t\t\t\t\tSerialNumber=" << devList.SerialNumber << std::endl;
            std::cout << "\t\t\t\t\t\tDescription=" << devList.Description << std::endl;
            std::cout << "\t\t\t\t\t\tftHandle=" << devList.ftHandle << " (should be zero)" << std::endl;
        }

        Sleep(10);

        //Open channel
        if(-1 == openCahnnel(channelIndex))
        {
            return -1;
        }

        Sleep(100);

        //set speed
        if (-1 == setSpeed(Speed::_100kbs))
        {
            closeHandle();
            return -1;
        }

        clearAllPins();
    }
    return 0;
}

void FT232_MPSSE::doWork()
{
    uint16_t pinsState = 0;
    DeviceState state = _handle !=nullptr ? DeviceState::Ready : DeviceState::Wait;

    while (true)
    {
        switch (state)
        {
        case DeviceState::Wait:
            {
                std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
                std::chrono::steady_clock::time_point endTime = startTime + std::chrono::seconds(2);
                while (_handle == nullptr)
                {
                    if (std::chrono::steady_clock::now() >= endTime) {
                        state = DeviceState::NotReady;
                        break; // Break the loop if 2 seconds have passed
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                state = DeviceState::Ready;
                std::cout << "\t\t\t\t\t\t(-- I am Ready --)" << std::endl;
            }
            break;
            case DeviceState::NotReady:
                {
                    Sleep(1000);
                    if(!init())
                    {
                        std::cout << "\t\t\t\t\t\t(-- I am Ready --)" << std::endl;
                        state = DeviceState::Ready;
                    }
                }
            break;
            case DeviceState::Ready:
                {
                    Sleep(200);
                    if (_handle != nullptr)
                    {
                        if (getPinsState(pinsState))
                        {
                            bool done = false;
                            for (const auto& [pinNumber, pinMode] : _pinsMode)
                            {
                                if (pinMode == PinMode::Input)
                                {
                                    if (Bitwise::getBitState(pinsState, static_cast<int>(pinNumber)) != Bitwise::getBitState(
                                        _previousPinsState, static_cast<int>(pinNumber)))
                                    {
                                        _previousPinsState = pinsState;
                                        done = true;
                                    }
                                }
                            }
                            if (done)
                            {
                                valueChanged(pinsState);
                            }
                        }
                        else if(nullptr == _handle)
                        {
                            state = DeviceState::NotReady;
                        }
                    }
                }
                break;
        }
    }
}