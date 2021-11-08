#ifndef MBED_DHT22_H
#define MBED_DHT22_H

#include "mbed.h"

class DHT22
{
private:
    PinName pin_name;
    float m_temperature, m_humidity;
    
    short shortFromBits(bool bits[]);

public:
    /* Instantiates a new DHT22 object
     * - param PinName name of the pin where the DHT22 is connected
     */
    DHT22(PinName);
    
    /* Populates the values stored in the fields m_temperature and m_humidity
     * - PRE: The object DHT22 is instantiated and the sensor connected.
     */
    int read();
    
    /* Returns the value of the temperature captured by the sensor
     * - PRE: The method read() has been performed at least once.
     */
    float getTemperature();
    
    /* Returns the value of the humidity captured by the sensor
     * - PRE: The method read() has been performed at least once.
     */
    float getHumidity();
};

#endif