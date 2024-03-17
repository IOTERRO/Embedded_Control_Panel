#pragma once


#include "export.h"

#include "FT232_MPSSE.h"
#include "PCA9685.h"
#include "ioHandler.h"

class  FACTORY_API Factory final
{
    public:
        Factory();
        // Delete the default copy constructor
        Factory(const Factory&) = delete;
        Factory& operator=(Factory) = delete;
        // Delete the default move constructor
        Factory(Factory&&) = delete;
        Factory& operator=(Factory&&) = delete;
        ~Factory();

        static std::shared_ptr<IoAdapter::FT232_MPSSE> getFt232H();

        static std::shared_ptr<ioAdapter::ioHandler> getIoHandler(const std::shared_ptr<io::inOut>& device);

        static std::shared_ptr<ioAdapter::PCA9685> getPwmDriver(const std::shared_ptr<I2C::I2CMaster>& device);
   };

