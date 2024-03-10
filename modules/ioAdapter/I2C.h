#pragma once
#include <cstdint>
#include <vector>

namespace I2C
{

    class I2CMaster
    {
    public:
        virtual ~I2CMaster() = default;
        I2CMaster() = default;
        I2CMaster(const I2CMaster&) = delete;
        I2CMaster& operator=(const I2CMaster&) = delete;
        I2CMaster(I2CMaster&&) = delete;
        I2CMaster& operator=(I2CMaster&&) = delete;

        /**
         * @brief Initializes the component. Necessary before its use or in case of error.
         *
         * @return 0 if successful, -1 otherwise.
         */
        virtual int init() { return -1; }
        virtual int busCycleBegin() { return 0; }
        virtual int busCycleEnd() { return 0; }

        enum class Speed
        {
            _10kbs = 10,   /**< Low-speed mode */
            _100kbs = 100,  /**< Std mode */
            _200kbs = 200,  /**< */
            _400kbs = 400,  /**< Fast mode */
            _1mbs = 1000, /**< Fast mode+ */
            _17mbs = 1700, /**< High-speed mode */
            _34mbs = 3400, /**< High-speed mode */
        };

        /**
         * @brief Set the I2C/SMBus.
         *
         * @param speed I2C bus speed (min:_100kbs max:_400kbs).
         * @return 0 if successful, -1 otherwise.
         */
        virtual int setSpeed(Speed speed)
        {
            (void)speed;
            return -1;
        }

        // I2C - 7 bits slave address.
         /**
          * @brief Read I2C data from a slave.
          *
          * @param addr The 7 bits slave address.
          * @param buf Data bytes read from the slave.
          * @param len The number of bytes to read from the slave.
          * @return Number of read bytes or -1 on error.
          */
        virtual int /* ssize_t */ read(const uint8_t addr, uint8_t* buf, const size_t len) { (void)addr; (void)buf; (void)len; return -1; }
        /**
         * @brief Write I2C data to a slave.
         *
         * @param addr The 7 bit slave address.
         * @param buf Data bytes write to the slave.
         * @param len The number of bytes to write to the slave.
         * @return Number of write bytes or -1 on error.
         */
        virtual int /* ssize_t */ write(const uint8_t addr, const uint8_t* buf, const size_t len) { (void)addr; (void)buf; (void)len; return -1; }

        // SMBus - 7 bits slave address.
        /**
         * @brief SMBus "read byte" protocol
         *
         * @param addr The 7 bit slave address.
         * @param cmd The device command
         * @param value The byte received from the device.
         * @return 0 if successful, -1 otherwise.
         */
        virtual int readByte(uint8_t addr, uint8_t cmd, uint8_t& value) { (void)addr; (void)cmd; (void)value; return -1; }
        /**
         * @brief SMBus "read word" protocol
         *
         * @param addr The 7 bit slave address.
         * @param cmd The device command
         * @param value The word received from the device.
         * @return 0 if successful, -1 otherwise.
         */
        virtual int readWord(uint8_t addr, uint8_t cmd, uint16_t& value) { (void)addr; (void)cmd; (void)value; return -1; }
        /**
         * TODO - To be defined
         *
         * @return 0 if successful, -1 otherwise.
         */
        virtual int readBlock(uint8_t addr, uint8_t cmd, std::vector<uint8_t>& values) { (void)addr; (void)cmd; (void)values; return -1; }
        /**
         * @brief SMBus "write byte" protocol
         *
         * @param addr The 7 bit slave address.
         * @param cmd The device command
         * @param value The byte to be written
         * @return 0 if successful, -1 otherwise.
         */
        virtual int writeByte(const uint8_t addr, const uint8_t cmd, const uint8_t value) { (void)addr; (void)cmd; (void)value; return -1; }
        /**
         * @brief SMBus "write word" protocol
         *
         * @param addr The 7 bit slave address.
         * @param cmd The device command
         * @param value The word to be written
         * @return 0 if successful, -1 otherwise.
         */
        virtual int writeWord(const uint8_t addr, const uint8_t cmd, const uint16_t value) { (void)addr; (void)cmd; (void)value; return -1; }
        /**
         * TODO - To be defined
         *
         * @return 0 if successful, -1 otherwise.
         */
        virtual int writeBlock(const uint8_t addr, const uint8_t cmd, const std::vector<uint8_t>& values) { (void)addr; (void)cmd; (void)values; return -1; }
    };
}
