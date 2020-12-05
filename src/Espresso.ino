/*
 * Project: Espresso
 * Description: Control Espresso Machine from HomeBridge and Apple HomeKit
 * Author: Sam Sipe
 * Date: 01 Aug 19
 */

// Includes
#include "Particle.h"
#include "OneWire.h"
#include "spark-dallas-temperature.h"
#include "pid.h"

// Inits
int powerRelay = D0; // Power Relay
int boilerRelay = D1; // Boiler Relay

OneWire oneWire(D2); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature dallas(&oneWire); // Pass oneWire reference to Dallas Temperature.
double caseTemp; // Here we are declaring the integer variable analogvalue, which we will use later to store the value of the tempSensor.
double groupTemp; // Temperature of the Espresso Machine's group head
double tempLoop; // For temp change detection
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
    Particle.publish("Temp Sensors Found", String(dallas.getDeviceCount()), PRIVATE);

}

// Loop
void loop()
{
    dallas.requestTemperatures();   // Request temperatures
    groupTemp = dallas.getTempCByIndex(1);  // get the temperature of the Espresso Machine's group head

    if (Time.local() - timeLoop >= 10)
    {
        caseTemp = dallas.getTempCByIndex(0);  // get the temperature of the Espresso Machine Case

        if (digitalRead(powerRelay) == HIGH && abs(caseTemp - tempLoop) > 1/16)
        {
            Particle.publish("Temperature", String::format("%.1f", caseTemp), PRIVATE);
            tempLoop = caseTemp;
        }

        timeLoop = Time.local();
    }

    if ( (Time.local() - timeOn >= 14400) && (digitalRead(powerRelay) == HIGH) ) // 4 hours shutoff
    {
        timeOn = Time.local();
        digitalWrite(boilerRelay, LOW);
    }

    /* Request time synchronization from the Particle Cloud */
    if (Time.local() == 14000)
    {
        Particle.syncTime();    // Request time synchronization from the Particle Device Cloud
        waitUntil(Particle.syncTimeDone); // Wait until the device receives time from Particle Device Cloud (or connection to Particle Device Cloud is lost)
        Log.info("Current time: %s", Time.timeStr().c_str()); // Print current time
        delay(1000);
    }
}

//Functions
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
            timeOn = Time.local();
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

// int boilerToggle(String command)
// {
//     if (command=="1")
//     {
//         // if (digitalRead(powerRelay) == HIGH)
//         // {
//                 digitalWrite(boilerRelay,HIGH);
//                 Particle.publish("Boiler", "ON", PRIVATE);
//                 return 1;
//         // }
//         // else if (digitalRead(powerRelay) == LOW)
//         // {
//         //     digitalWrite(boilerRelay,LOW);
//         //     Particle.publish("Boiler", "OFF", PRIVATE);
//         //     return 0;
//         // }
//         // else
//         // {
//         //     return -1;
//         // }
//     }
//     else if (command=="0")
//     {
//         digitalWrite(boilerRelay,LOW);
//         Particle.publish("Boiler", "OFF", PRIVATE);
//         return 0;
//     }
//     else if (command=="?")
//     {
//         if (digitalRead(boilerRelay) == HIGH)
//         {
//             Particle.publish("Boiler", "ON", PRIVATE);
//             return 1;
//         }
//         else if (digitalRead(boilerRelay) == LOW)
//         {
//             Particle.publish("Boiler", "OFF", PRIVATE);
//             return 0;
//         }
//         else
//         {
//             return -1;
//         }
//     }
//     else
//     {
//         return -1;
//     }
// }