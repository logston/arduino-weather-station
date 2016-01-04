/*
 * An Arduino Uno R3 script for recording data
 * from two sensors and a clock
 *
 * Paul Logston 2016
 */
#include <Arduino.h>
// Include the libraries we need
#include <SD.h>
#include <DHT.h>
#include <Wire.h>
#include <DS3231.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <avr/pgmspace.h>

#define STATUS_LED 9

// SD card uses SPI, which takes place on
// digital pins 11, 12, and 13 (on most Arduino boards)
#define SD_CS 10

#define DHTPIN 8
#define DHTTYPE DHT11

// SD Card / Data persistence
char TEMPERATURES_FILE[] = "data.csv";
File myFile;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

// Define BMP180 pressure sensor
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

// DS3231
DS3231 Clock;

// Forward Declarations
void set_datetime_string();
void write_headers_to_sd();

/*
 * Main Setup Function
 */
void setup(void)
{
  pinMode(STATUS_LED, OUTPUT);

  // start serial port
  Serial.begin(9600);
  Serial.println();

  Serial.print(F("# Initializing SD card..."));
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
   pinMode(SD_CS, OUTPUT);

  if (!SD.begin(SD_CS)) {
    Serial.println(F("initialization failed!"));
    while (1) {}
  }
  Serial.println(F("initialization done."));

  myFile = SD.open(TEMPERATURES_FILE, FILE_WRITE);
  if (!myFile) {
    Serial.println(F("# Unable to open card for writing"));
    while (1) {}
  } else {
    myFile.println();
    myFile.println(F("# Restarting ..."));
    myFile.close();
  }

  // Start DHT sensor
  Serial.print(F("# Starting DHT sensor ... "));
  dht.begin();
  Serial.println(F("done."));

  /* Initialise the BMP180 pressure sensor */
  Serial.print(F("# Starting BMP180 sensor ... "));
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.println(F("oops, no BMP found"));
    return;
  }
  Serial.println(F("done."));

  write_headers_to_sd();

  if (false) {  // set to "true" to set clock
    Clock.setYear(16);    // Set the year (Last two digits of the year)
    Clock.setMonth(1);    // Set the month of the year
    Clock.setDate(4);    // Set the date of the month
    Clock.setHour(3);    // Set the hour
    Clock.setMinute(0);  // Set the minute
    Clock.setSecond(0);  // Set the second
  }
}

/*
 * Main Loop function
 */
void loop(void)
{

  set_datetime_string();

  // DHT monitor
  // (requires 250 ms to read data)

  float dht_humidity;
  float dht_temp;
  // get temp
  dht_temp = dht.readTemperature();
  if (isnan(dht_temp)) {
    dht_temp = -1.0;
  }
  // get humidity
  dht_humidity = dht.readHumidity();
  if (isnan(dht_humidity)) {
    dht_humidity = -1.0;
  }

  // BMP180
  // Get a new sensor event
  float bmp_pressure;
  float bmp_temp;

  sensors_event_t event;
  bmp.getEvent(&event);
  if (event.pressure) {
    // pressue in hPa
    bmp_pressure = event.pressure;
  } else {
    bmp_pressure = -1.0;
  }
  bmp.getTemperature(&bmp_temp);

  // DS3231 Temp sensor
  int ds3231_temp;
  ds3231_temp = Clock.getTemperature();

  // Print to Serial
  Serial.print(F(","));
  Serial.print(dht_temp);
  Serial.print(F(","));
  Serial.print(dht_humidity);
  Serial.print(F(","));
  Serial.print(bmp_temp);
  Serial.print(F(","));
  Serial.print(bmp_pressure);
  Serial.print(F(","));
  Serial.println(ds3231_temp);

  myFile = SD.open(TEMPERATURES_FILE, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    digitalWrite(STATUS_LED, HIGH);
    myFile.print(",");
    myFile.print(dht_temp);
    myFile.print(",");
    myFile.print(dht_humidity);
    myFile.print(",");
    myFile.print(bmp_temp);
    myFile.print(",");
    myFile.print(bmp_pressure);
    myFile.print(",");
    myFile.println(ds3231_temp);
    // close the file
    myFile.close();
    digitalWrite(STATUS_LED, LOW);
  } else {
    Serial.println(F("# Unable to write data to disk"));
  }

  delay(10000);
}

void
set_datetime_string()
{
  int second, minute, hour, date, month, year;
  bool False = false;
  char datetime_string[20];

  second = Clock.getSecond();
  minute = Clock.getMinute();
  hour = Clock.getHour(False, False); // h12=false, PM=false
  date = Clock.getDate();
  month = Clock.getMonth(False); // Centry=false
  year = Clock.getYear();

  sprintf(datetime_string, "\0");
  sprintf(datetime_string, "20%d,%d,%d,%d,%d,%d",
          year, month, date, hour, minute, second);

  Serial.print(datetime_string);

  myFile = SD.open(TEMPERATURES_FILE, FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    myFile.print(datetime_string);
    // close the file
    myFile.close();
  } else {
    Serial.println(F("# Unable to write datetime to disk"));
  }
}

void
write_headers_to_sd()
{

  const PROGMEM char datetime_header[] = "year,month,day,hour,minute,second,";
  const PROGMEM char dht_header[] = "DHT temp (C),DHT humidity (%),";
  const PROGMEM char bmp_header[] = "BMP temp (C),BMP pressure (hPa),";
  const PROGMEM char ds3231_header[] = "DS3231 temp (C)";

  // Print to serial
  Serial.print(datetime_header);
  Serial.print(dht_header);
  Serial.print(bmp_header);
  Serial.println(ds3231_header);

  // Save to SD
  myFile = SD.open(TEMPERATURES_FILE, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.print(datetime_header);
    myFile.print(dht_header);
    myFile.print(bmp_header);
    myFile.println(ds3231_header);
   // close the file
    myFile.close();
  } else {
    Serial.println(F("# Unable to write headers to disk"));
  }
}

