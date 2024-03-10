//#define _FT_USED

#ifdef _FT_USED

#include "FT232.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "Bitwise.h"

using namespace IoAdapter;

FT232::FT232()
{
    init();
}

FT232::~FT232()
{
    clearAllPins();
    closeHandle();
    _handle.reset();
}

bool FT232::init()
{
    openHandle();
    clearAllPins();
    return true;
}

void FT232::openHandle() const
{
    // Initialize the D2XX driver
    //const FT_STATUS ftStatus = FT_Open(0, _handle.get());
    const FT_STATUS ftStatus = FT_OpenEx("FT232H", FT_OPEN_BY_DESCRIPTION, _handle.get());
    if (ftStatus != FT_OK) {
        closeHandle();
        throw std::runtime_error("Failed to initialize D2XX driver");
    }
}

void FT232::closeHandle() const
{
    FT_Close(*_handle);
}

bool FT232::setEepromPins(const Gpio gpio, const GpioState gpioState)
{

    //configureEeprom(); TODO: enable it once needed only in order to configure c5,c6,c8,c9 as an IO pins

    /*
      Exemples:
      ---------
      b 1111 0001  --> 1111: (C9,C8,C6,C5 : OUTPUT)   0001: (C9,C8,C6: LOW) & (C5: HIGH)

      b 1111 0010  --> 1111: (C9,C8,C6,C5 : OUTPUT)   0010: (C9,C8,C5: LOW) & (C6: HIGH)
    */

    uint8_t ucMask;

    if(gpio == Gpio::C5)
    {
        //b11110001
        ucMask = gpioState == GpioState::High ? 0xF1 : 0xF0;
    }
    else if(gpio == Gpio::C6)
    {
        //b11110010
        ucMask = gpioState == GpioState::High ? 0xF2 : 0xF0;
    }
    else if(gpio == Gpio::C8)
    {
        //b11110100
        ucMask = gpioState == GpioState::High ? 0xF4 : 0xF0;
    }
    else if (gpio == Gpio::C9)
    {
        //b11111000
        ucMask = gpioState ==  GpioState::High ? 0xF8 : 0xF0;
    }
    else
    {
        //other then c5,c6,c8 or c9
        return false;
    }
    
    FT_SetBitMode(*_handle, ucMask, FT_BITMODE_CBUS_BITBANG);

    return true;
}

bool FT232::configureEepromPins()
{
    FT_PROGRAM_DATA ftData;

    WORD VendorIdBuf = 0x0403;
    WORD ProductIdBuf = 0x6010;
    char ManufacturerBuf[32] = "FTDI";
    char ManufacturerIdBuf[16] = "FT";
    char DescriptionBuf[64] = "FT232H";
    char SerialNumberBuf[16] = "FT123456";

    ftData.Signature1 = 0x00000000;
    ftData.Signature2 = 0xffffffff;
    ftData.Version = 3;

    ftData.VendorId = VendorIdBuf;
    ftData.ProductId = ProductIdBuf;
    ftData.Manufacturer = ManufacturerBuf;
    ftData.ManufacturerId = ManufacturerIdBuf;
    ftData.Description = DescriptionBuf;
    ftData.SerialNumber = SerialNumberBuf;


    FT_STATUS ftStatus = FT_EE_Read(*_handle, &ftData);
    if (ftStatus != FT_OK)
    {
        std::cerr << "Failed to read EEPROM data (error code: " << ftStatus << ")" << std::endl;
        closeHandle();
        return false;
    }

    // Configuration des pins CBUS
    ftData.Cbus0H = FT_232H_CBUS_TRISTATE;
    ftData.Cbus1H = FT_232H_CBUS_TRISTATE;
    ftData.Cbus2H = FT_232H_CBUS_TRISTATE;
    ftData.Cbus3H = FT_232H_CBUS_TRISTATE;
    ftData.Cbus4H = FT_232H_CBUS_TRISTATE;
    ftData.Cbus5H = FT_232H_CBUS_IOMODE; // I/O mode
    ftData.Cbus6H = FT_232H_CBUS_IOMODE; // I/O mode
    ftData.Cbus7H = FT_232H_CBUS_TRISTATE;
    ftData.Cbus8H = FT_232H_CBUS_IOMODE; // I/O mode
    ftData.Cbus9H = FT_232H_CBUS_IOMODE; // I/O mode

    ftStatus = FT_EE_Program(*_handle, &ftData);
    if (ftStatus != FT_OK) {
        std::cerr << "Failed to program EEPROM data (error code: " << ftStatus << ")" << std::endl;
        closeHandle();
        return false;
    }
    return true;
}


bool FT232::pinMode(Gpio gpio, const PinMode mode)
{
    // Read current configuration
    uint32_t currentMask = 0xFFFFFFFF;
    for(const auto& [pinNumber, pinMode] : _pinsMode)
    {
        if(pinMode == PinMode::Output)
        {
            Bitwise::setBit(currentMask, static_cast<int>(pinNumber));
        }
        else
        {
            Bitwise::clearBit(currentMask, static_cast<int>(pinNumber));
        }
    }

    auto newMask = currentMask;

    //Handle the requested mode
    if(mode == PinMode::Output)
    {
        // Set bit for OUTPUT mode
        Bitwise::setBit(newMask, static_cast<int>(gpio));
    }
    else
    {
        // Clear bit for INPUT mode
        Bitwise::clearBit(newMask, static_cast<int>(gpio));
    }

    // Apply modified configuration to all GPIO pins
    const auto ftStatusConfig = FT_SetBitMode(*_handle, static_cast<uint8_t>(newMask), FT_BITMODE_ASYNC_BITBANG);
    if (ftStatusConfig != FT_OK) {
        closeHandle();
        std::cerr << "Failed to configure GPIO (error code: " << ftStatusConfig << ")" << std::endl;
        return false;
    }

   std::this_thread::sleep_for(std::chrono::milliseconds(100));

    _pinsMode.at(gpio) = mode;
    return true;
}

bool FT232::set(Gpio gpio, const GpioState state)
{

    // Check if the specified pin is configured as an output
    if (_pinsMode.at(gpio) != PinMode::Output)
    {
        std::cerr << "Pin " << static_cast<int>(gpio) << " is not configured as an output" << std::endl;
        return false;
    }

    if(static_cast<int>(gpio) > 7)
    {
        setEepromPins(gpio, state);
    }
    else
    {
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

        // Write to GPIO pins
        DWORD bytesWritten;
        const auto ftStatus = FT_Write(*_handle, &newMask, 1, &bytesWritten);
        if (ftStatus != FT_OK) {
            closeHandle();
            std::cerr << "Failed to write to GPIO (error code: " << ftStatus << ")" << std::endl;
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    }

    _pinsState.at(gpio) = state;

    return true;
}

bool FT232::get(Gpio gpio, GpioState &gpioState)
{
    uint32_t pinsState = 0x00;
    DWORD bytesRead;

    // Check if the specified pin is configured as an input
    if(_pinsMode.at(gpio) != PinMode::Input)
    {
        std::cerr << "Pin " << static_cast<int>(gpio) << " is not configured as an input" << std::endl;
        return false;
    }

    Sleep(100);


    FT_Purge(*_handle, FT_PURGE_RX);
    FT_SetTimeouts(*_handle, 100, 0);

    const FT_STATUS ftStatus = FT_Read(*_handle, &pinsState, sizeof(pinsState), &bytesRead);
    if (ftStatus == FT_OK) {
        gpioState = Bitwise::getBitState(pinsState, static_cast<int>(gpio)) ? GpioState::High : GpioState::Low;
    }
    else 
    {
        gpioState = GpioState::Unknown;
        std::cerr << "Failed to read pin D0 state ---> error code(" << ftStatus << ")" << std::endl;
        return false;
    }

    return true;
}

void FT232::clearAllPins() const
{
    // Set all bits as outputs and clear them
    const auto ftStatusConfig = FT_SetBitMode(*_handle, 0xFF, FT_BITMODE_ASYNC_BITBANG);
    if (ftStatusConfig != FT_OK) {
        closeHandle();
        throw std::runtime_error("Failed to configure GPIO");
    }


    // Introduce a small delay to allow changes to take effect
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    DWORD bytesWritten;
    BYTE tempData = 0x00; // Data to be written to GPIO pins
    const auto ftStatus = FT_Write(*_handle, &tempData, 1, &bytesWritten);
    if (ftStatus != FT_OK) {
        closeHandle();
        throw std::runtime_error("Failed to write to GPIO");
    }

    // Introduce a small delay to allow changes to take effect
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
#endif // _FT_USED