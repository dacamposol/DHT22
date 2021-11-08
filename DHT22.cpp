#include "DHT22.h"

DHT22::DHT22(PinName pin)
{
    pin_name = pin;

    DigitalInOut dht(pin_name);
    dht.output();
    dht.write(1);
    thread_sleep_for(2);
}

short DHT22::shortFromBits(bool bits[])
{
    short result = 0;
    for (int i = 0; i<8; i++) {
        result += bits[i]*(1 << (7-i));
    }
    return result;
}

// Implementation for reading a DHT22 according to the datasheet in
// https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf
int DHT22::read()
{
    Timer period;

    DigitalInOut dht(pin_name);

    // First, we send a low signal for 1ms to the sensor
    dht.output();
    dht.write(0);
    thread_sleep_for(2);

    // Now, we pull-up the voltage and wait for the MCU answer
    dht.write(1);
    dht.input();
    // According to the documentation, the DHT will take between 20-40us to answer, so we just wait the minimum time
    // Also, if the sensor doesn't change its status from 1, then it's broken.
    period.start();
    while (dht) {
        if (period.read() > 1) {
            return -1;
        }
    }
    period.stop();

    // Now the DHT has set to zero, it shall last 80us and then be another 80us in up
    wait_us(80);
    wait_us(80);

    // Now it starts the data transmision, we expect first 50us before the first bit
    // We expect 5 packets of 8 bits:
    // - Integral RH Data
    // - Decimal RH Data
    // - Integral Temp Data
    // - Decimal Temp Data
    // - Checksum
    bool packets[5][8];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            // We wait the start of the bit with wait just in case we have been carrying error
            // wait_us(50);
            while(!dht); // Low signal of 50us. Each bit has to start with it.
            period.reset();
            period.start();
            while(dht);
            period.stop();

            // According to the documentation, it's 26us vs 70us
            packets[i][j] = period.read_us() > 60;
        }
    }

    // Now we stop the sensor
    dht.output();
    dht.write(1);

    // And we treat the received data

    // First, we treat the MSB (bit 0) of the temp, if it's 1, then it's negative.
    // The real value for the temperature is stored in the bits 1 ~ 16
    int MSB = 1;
    if (packets[2][0])
        MSB = -1;
    packets[2][0] = 0;

    short high_humidity = shortFromBits(packets[0]);
    short low_humidity = shortFromBits(packets[1]);
    short high_temp = shortFromBits(packets[2]);
    short low_temp = shortFromBits(packets[3]);

    // If the values coincide with the checksum, then change the last values of temperature and humidity
    // Otherwise, print the problem and exit the method
    if ((high_humidity + low_humidity + high_temp + low_temp) % 256 == shortFromBits(packets[4])) {
        m_humidity = (high_humidity * 256 + low_humidity) / 10;
        m_temperature = MSB * (high_temp * 256 + low_temp) / 10;
    } else {
        return -1;
    }

    return 1;
}

float DHT22::getTemperature()
{
    return m_temperature;
}

float DHT22::getHumidity()
{
    return m_humidity;
}