// Simple sketch for reading a ds1820b temperature sensor.
// Spits out temp readings to the serial connection.

#include <OneWire.h>
#include <DallasTemperature.h>

/* Generally, on a ds18b20, yellow is data, red is +5V, and black is ground.
 * There's some nonsens you can do with "parasite power," wiring both red and black
 * to ground, but we're not gonna bother with that nonsense.
 *
 * Just hook black to ground, red to D2 (which we drive high to provide power),
 * and then data to D3, with a 4.7k Ohm pullup resistor between D3 and D2. It's
 * a bit sloppy to use an I/O pin to provide power, this unit doesn't take much,
 * and it lets us use three pins in a row, which is better for screw terminal
 * connections.
 */

#define YELLOW 3
#define BLACK GND
#define RED 2

OneWire oneWire(YELLOW);
DallasTemperature sensors(&oneWire);

void setup() {

  // Standard thing. Low baud rate doesn't really matter for our purposes.
  Serial.begin(9600);

  // Provide power.
  pinMode(RED, OUTPUT);
  digitalWrite(RED, HIGH);

  // Wait until the sensor is found.
  int numDevices;
  do {
    sensors.begin();
    numDevices = sensors.getDeviceCount();
    delay(100);
  } while(!numDevices);

  // After it's found, still needs a few seconds before we start reading temps.
  delay(3000);

}

void loop(){
  float t;
  sensors.requestTemperatures();
  t = DallasTemperature::toFahrenheit(sensors.getTempCByIndex(0));
  Serial.println(t);

  // Not really once a second, since reading the sensor takes time, but close enough.
  delay(1000);
}
