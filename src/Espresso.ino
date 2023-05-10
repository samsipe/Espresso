/*
 * Project: Espresso
 * Description: Control Espresso Machine from HomeBridge and Apple HomeKit
 * Author: Sam Sipe
 * Date: 10 May 2023
 */

// Includes
#include "Particle.h"
#include "OneWire.h"
#include "spark-dallas-temperature.h"
#include "pid.h"

// Inits
int powerRelay = D4;  // Power Relay
int boilerRelay = D6; // Boiler Relay

OneWire oneWire(D2);                // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature dallas(&oneWire); // Pass oneWire reference to Dallas Temperature.
double caseTemp;                    // Here we are declaring the integer variable analog value, which we will use later to store the value of the tempSensor.
double groupTemp;                   // Temperature of the Espresso Machine's group head
double tempLoop;                    // For temp change detection
int timeLoop = Time.local();
int timeOn = Time.local(); // For tracking how long the machine has been on

// Setup
void setup()
{
    // Pin Configs
    pinMode(powerRelay, OUTPUT);
    pinMode(boilerRelay, OUTPUT);

    // Pin States on Startup
    digitalWrite(powerRelay, LOW);
    digitalWrite(boilerRelay, LOW);

    // Particle Functions //
    Particle.function("espresso", espressoPower);
    // Particle.function("boiler", boilerToggle);

    // Particle Variables //
    Particle.variable("groupTemp", groupTemp);
    Particle.variable("caseTemp", caseTemp);

    // setup the library
    dallas.begin();
    int deviceCount = dallas.getDeviceCount();
    Particle.publish("Temp Sensors Found", String(deviceCount), PRIVATE);
}

// Loop
void loop()
{
    dallas.requestTemperatures();          // Request temperatures
    groupTemp = dallas.getTempCByIndex(0); // get the temperature of the Espresso Machine's group head
    caseTemp = dallas.getTempCByIndex(1);  // get the temperature of the Espresso Machine Case

    if (Time.local() - timeLoop >= 3 && digitalRead(powerRelay) == HIGH && abs(groupTemp - tempLoop) > .1) // 3 second delay and 1/10 degree change
    {
        Particle.publish("Group Temp", String::format("%.2f", groupTemp), PRIVATE);

        tempLoop = groupTemp;
        timeLoop = Time.local();
    }

    if ((Time.local() - timeOn >= 14400) && (digitalRead(powerRelay) == HIGH)) // 4 hour boiler shutoff
    {
        digitalWrite(boilerRelay, LOW);
        Particle.publish("Boiler", "OFF", PRIVATE);
    }

    /* Request time synchronization from the Particle Cloud */
    if (Time.now() % 86400 == 0)
    {
        Particle.syncTime();                                  // Request time synchronization from the Particle Device Cloud
        waitUntil(Particle.syncTimeDone);                     // Wait until the device receives time from Particle Device Cloud (or connection to Particle Device Cloud is lost)
        Log.info("Current time: %s", Time.timeStr().c_str()); // Print current time
        delay(1000);
    }
}

// Functions
int espressoPower(String command)
{
    if (command == "1")
    {
        digitalWrite(powerRelay, HIGH);
        delay(500);
        digitalWrite(boilerRelay, HIGH);
        timeOn = Time.local();
        Particle.publish("Espresso", "ON", PRIVATE);
        return 1;
    }
    else if (command == "0")
    {
        digitalWrite(boilerRelay, LOW);
        // Particle.publish("Boiler", "OFF", PRIVATE);
        delay(500);
        digitalWrite(powerRelay, LOW);
        Particle.publish("Espresso", "OFF", PRIVATE);
        return 0;
    }
    else if (command == "?")
    {
        if (digitalRead(powerRelay) == HIGH)
        {
            digitalWrite(boilerRelay, HIGH);
            Particle.publish("Espresso", "ON", PRIVATE);
            return 1;
        }
        else if (digitalRead(powerRelay) == LOW)
        {
            digitalWrite(boilerRelay, LOW);
            Particle.publish("Espresso", "OFF", PRIVATE);
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
