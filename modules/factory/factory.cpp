#include "factory.h"

#include <memory>

Factory::Factory()
{
    
}

Factory::~Factory()
{
    
}

std::shared_ptr<IoAdapter::FT232_MPSSE> Factory::getFt232H()
{
    return std::make_shared<IoAdapter::FT232_MPSSE>();
}

std::shared_ptr<ioAdapter::ioHandler> Factory::getIoHandler(const std::shared_ptr<io::inOut>& device)
{
    return std::make_shared<ioAdapter::ioHandler>(device);
}

std::shared_ptr<ioAdapter::PCA9685> Factory::getPwmDriver(const std::shared_ptr<I2C::I2CMaster>& device)
{
    return std::make_shared<ioAdapter::PCA9685>(device);
}
