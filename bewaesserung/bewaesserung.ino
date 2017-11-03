//include libraries
#include <Wire.h>
#include "DS1307.h"
#include "DHT.h"

//define pin configuration
#define DHTPIN A0
#define WATERSENSOR 4
#define WATERPUMP 2

//commands as defines
#define WATER_ON   digitalWrite(2, LOW);
#define WATER_OFF  digitalWrite(2, HIGH);

//other defines
#define DHTTYPE DHT11   // DHT 11 

//configuration defines
#define MIN_TEMPERATURE 5    //minimum temperature allowed (to avoid ice)
#define EARTHMIN 360         //only water, if value is higher EARTHMIN
#define MORNING 8            //time in the morning
#define AFTERNOON 17         //time in the afternoon (summer time) -> winter 16
#define WATERRUNTIME 120     // how long eacht water run should be

//object definitions
DHT dht(DHTPIN, DHTTYPE);
DS1307 clock;

//global variables
bool done = false;
int oldMinute = -1;

//setup
void setup() {

  //begin serial communication with baudrate 9600
  Serial.begin(9600);

  //initialize rtc module
  clock.begin();

  //setting clock time
  // clock.fillByYMD(2017,10,2);//Jan 19,2013
  // clock.fillByHMS(17,21,0);//15:28 30"
  // clock.fillDayOfWeek(MON);//Saturday
  // clock.setTime();//write time to the RTC chip

  //initilaize temperature & humidity
  dht.begin();

  //initialize relais
  digitalWrite(WATERPUMP, HIGH);
  pinMode(WATERPUMP, OUTPUT);

  //water state sensor as input
  pinMode(WATERSENSOR, INPUT);

  //print header for csv output
  printCSVHeader();
}

//loop
void loop() {

  //get actual time
  clock.getTime();

  //start afternoon water
  if (clock.hour == (int)AFTERNOON && done == false)
  {
    Serial.println("Start water job in the afternoon");
    doWaterJob();
    done = true;
    Serial.println("Water job finished");
  }

  //start afternoon water
  else if (clock.hour == (int) MORNING && done == false)
  {
    Serial.println("Start water job in the morning");
    doWaterJob();
    done = true;
    Serial.println("Water job finished");
  }

  //reset 
  else if (clock.hour != (int) AFTERNOON && clock.hour !=(int) MORNING)
  {
    done = false;
  }

  //print CSV row
  if (clock.minute != oldMinute && clock.second == 0)
  {
    oldMinute = clock.minute;
    // printTime();
    // printEarth();
    //printSurround();
    // printWater();
    printCSVData();
  }
}

//watering process
void doWaterJob()
{
  if (waterCouldBeFrozen() == false && wateringNecessary() == true)
  {
    unsigned int startTime = millis();
    unsigned int runTime = 0;
    if (enoughWater() == true)
    {
      WATER_ON;
    }
    while (enoughWater() == true && (runTime <= (WATERRUNTIME * 1000)))
    {

      runTime = millis() - startTime;
      if (clock.minute != oldMinute && clock.second == 0)
      {
        oldMinute = clock.minute;
        printCSVData();
      }
    }
    WATER_OFF;
  }
}

bool waterCouldBeFrozen()
{
  int temperature = dht.readTemperature();
  if (temperature < MIN_TEMPERATURE) return true;
  return false;
}

bool wateringNecessary()
{
  if (analogRead(1) > EARTHMIN) return true;
  return false;
}

bool enoughWater()
{
  if (digitalRead(WATERSENSOR) == LOW)
  {
    return true;
  }
  return false;
}

//print csv header
void printCSVHeader()
{
  Serial.println("Datetime; Water; Temperature; Humidity; Earth; ");
}

//print csv row
void printCSVData()
{
  //print date and time
  clock.getTime();
  Serial.print(clock.dayOfMonth, DEC);
  Serial.print(".");
  Serial.print(clock.month, DEC);
  Serial.print(".");
  Serial.print(clock.year + 2000, DEC);
  Serial.print(" ");
  if (clock.hour < 10)Serial.print(0, DEC);
  Serial.print(clock.hour, DEC);
  Serial.print(":");
  if (clock.minute < 10)Serial.print(0, DEC);
  Serial.print(clock.minute, DEC);
  Serial.print(":");
  if (clock.second < 10)Serial.print(0, DEC);
  Serial.print(clock.second, DEC);
  Serial.print(";");

  //print Water
  if (enoughWater() == true)
  {
    Serial.print("OK;");
  }
  else
  {
    Serial.print("NOK;");
  }

  //print temperature an humidity
  int h =  dht.readHumidity();
  int t =  dht.readTemperature();

  if (isnan(t) || isnan(h))
  {
    Serial.print(";;");
  }
  else
  {
    Serial.print(t);
    Serial.print(";");
    Serial.print(h);
    Serial.print(";");
  }

  //print earth
  Serial.print(analogRead(1), DEC); //print the value to serial port
  Serial.println(";");
}
