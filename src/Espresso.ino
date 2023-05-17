/*
 * Project: Espresso
 * Description: Control Espresso Machine from HomeBridge and Apple HomeKit
 * Author: Sam Sipe
 * Date: 16 May 2023
 * Version: 0.3.0
 */

// Includes
#include "Particle.h"
#include "OneWire.h"
#include "spark-dallas-temperature.h"
#include "pid.h"

SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

SerialLogHandler usbLogHandler(LOG_LEVEL_TRACE);

// Pins
int tempSensor = D2;   // Temperature Sensor
int powerRelay = D4;   // Power Relay
int boilerRelay = D6;  // Boiler Relay
int indicatorLED = D7; // Indicator LED

// Variables
OneWire oneWire(tempSensor);              // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature dallas(&oneWire);       // Pass oneWire reference to Dallas Temperature.
double caseTemp, groupTemp, tempLoop;     // Temperature Variables
unsigned long timeOn, timeLoop, lastSync; // Time loops
unsigned long msSeenCloud = 0;            // remember when we last saw the cloud
static int retries = 0;                   // how many retries to connect are left

#define CLOUD_TIMEOUT 60000 // how many ms are acceptable for lost cloud connection (needs to be > 45sec)
#define CLOUD_MAXRETRY 5    // how many unsuccessful connection attempts befor a System.reset
#define SHUTOFF_SEC 14400   // 4 hours in seconds for shut off
#define SHUTOFF_TEMP 42     // 42ºC for shut off
#define ONE_DAY_SEC 87000   // 24 hours and 10 minutes in seconds

int timeOnMin()
{
    if (digitalRead(powerRelay) == HIGH)
        return (Time.local() - timeOn) / 60; // Time on in minutes
    else
        return 0;
}

// Setup
void setup()
{
    // Serial and Cloud
    Serial.begin(115200);
    Particle.connect();

    // Set times
    timeOn = Time.local();
    timeLoop = Time.local();
    lastSync = Time.local();

    // Pin Configs
    pinMode(powerRelay, OUTPUT);
    pinMode(boilerRelay, OUTPUT);
    pinMode(indicatorLED, OUTPUT);

    // Pin States on Startup
    digitalWrite(powerRelay, LOW);
    digitalWrite(boilerRelay, LOW);
    digitalWrite(indicatorLED, LOW);

    // Particle Functions //
    Particle.function("espresso", espressoPower);

    // Particle Variables //
    Particle.variable("groupTemp", groupTemp);
    Particle.variable("caseTemp", caseTemp);
    Particle.variable("timeOn", timeOnMin);

    // setup the library
    dallas.begin();
    int deviceCount = dallas.getDeviceCount();
    Particle.publish("Temp Sensors Found", String(deviceCount), PRIVATE);
}

inline void softDelay(uint32_t msDelay)
{
    for (uint32_t ms = millis(); millis() - ms < msDelay; Particle.process())
        ;
}

inline void espressoStartup()
{
    timeOn = Time.local();
    Particle.publish("Espresso", "ON", PRIVATE);
    digitalWrite(powerRelay, HIGH);
    delay(5000); // smooth startup
    digitalWrite(boilerRelay, HIGH);
}

inline void espressoShutoff()
{
    Particle.publish("Espresso", "OFF", PRIVATE);
    digitalWrite(boilerRelay, LOW);
    delay(5000); // smooth shutdown
    digitalWrite(powerRelay, LOW);
}

void maintainWiFi()
{
    // check cloud connection and if required reconnect
    if (Particle.connected())
    {
        msSeenCloud = millis();
        retries = 0;
    }
    else if (millis() - msSeenCloud > CLOUD_TIMEOUT)
    {
        Particle.disconnect();
        softDelay(500);
        WiFi.disconnect();
        softDelay(1500);
        WiFi.off();
        softDelay(4000);
        digitalWrite(indicatorLED, HIGH); // leave on the indicator LED if this code is run

        if (retries++ < CLOUD_MAXRETRY)
        {
            Log.info("Reconnect to Cloud (%d)", retries);

            WiFi.off();
            WiFi.on();
            Particle.connect();

            if (waitFor(Particle.connected, CLOUD_TIMEOUT))
            { // only works with SYSTEM_THREAD(ENABLED)
                retries = 0;
            }
            else
            {
                Log.info("\tfailed - retry in %d sec", CLOUD_TIMEOUT / 1000);
                Particle.disconnect();
            }

            msSeenCloud = millis(); // don't check too early
        }
        else
        {
            System.reset(); // no success in reconnection - restart from scratch
        }
    }
}

// Loop
void loop()
{
    dallas.requestTemperatures();          // Request temperatures
    groupTemp = dallas.getTempCByIndex(0); // get the temperature of the Espresso Machine's group head
    caseTemp = dallas.getTempCByIndex(1);  // get the temperature of the Espresso Machine Case

    if (digitalRead(powerRelay) == HIGH)
    {
        if (Time.local() - timeLoop >= 3 && abs(groupTemp - tempLoop) > .1) // 3 second delay and 1/10 degree change
        {
            Particle.publish("Group Temp", String::format("%.2f", groupTemp), PRIVATE);

            tempLoop = groupTemp;
            timeLoop = Time.local();
        }

        if (Time.local() - timeOn > SHUTOFF_SEC) // Boiler safety shutoff
        {
            espressoShutoff();
            Particle.publish("Safety", String::format("Timeout after %s minutes", timeOnMin), PRIVATE);
        }
        else if (caseTemp > SHUTOFF_TEMP)
        {
            espressoShutoff();
            Particle.publish("Safety", String::format("Case Temp: %.2f ºC", caseTemp), PRIVATE);
        }
        else if (groupTemp > SHUTOFF_TEMP * 4)
        {
            espressoShutoff();
            Particle.publish("Safety", String::format("Group Temp: %.2f ºC", groupTemp), PRIVATE);
        }
        else
        {
            /* TODO: PID goes here */
        }
    }

    if (Time.local() - lastSync > ONE_DAY_SEC)
    {
        // Request time synchronization from the Particle Cloud
        Particle.syncTime();
        lastSync = Time.local();
        waitUntil(Particle.syncTimeDone);                     // Wait here until the sync is complete
        Log.info("Current time: %s", Time.timeStr().c_str()); // Print current time
    }

    maintainWiFi();
}

// Functions
int espressoPower(String command)
{
    if (command == "1")
    {
        espressoStartup();
        return 1;
    }
    else if (command == "0")
    {
        espressoShutoff();
        return 0;
    }
    else if (command == "?")
    {
        if (digitalRead(powerRelay) == HIGH)
        {
            digitalWrite(boilerRelay, HIGH);
            return 1;
        }
        else if (digitalRead(powerRelay) == LOW)
        {
            digitalWrite(boilerRelay, LOW);
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}
