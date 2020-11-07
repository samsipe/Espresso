/*
 * Project Espresso
 * Description: Control Espresso Machine from HomeBridge and Apple HomeKit
 * Author: Sam Sipe 
 * Date: 01 Aug 19
 */

// Includes

// Inits 
int relay1 = D0; // Power Relay
int relay2 = D1; // Boiler Relay
int tempSensor = A0; // This is where your tempSensor is plugged in. The other side goes to the "power" pin (below).
int power = A5; // This is the other end of your tempSensor. The other side is plugged into the "tempSensor" pin (above).
int tempValue; // Here we are declaring the integer variable analogvalue, which we will use later to store the value of the tempSensor.
int dump;
int count;

//Setup
void setup()
{
    // Pin Configs
    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);
    pinMode(tempSensor,INPUT);  // Our tempSensor pin is input (reading the tempSensor)
    pinMode(power,OUTPUT); // The pin powering the tempSensor is output (sending out consistent power)
    
    // Pin States on Startup
    digitalWrite(power,HIGH);
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);

    // Particle.functions
    Particle.function("espresso",espressoToggle);
    Particle.function("boiler",boilerToggle);
   
    // Particle.variables
    Particle.variable("temperature", tempValue);
}

//Loop
void loop()
{
//     tempValue = analogRead(tempSensor);
//     tempValue = map(tempValue, 0 , 2600, 0, 100);

//    if (count >=5000)
//    {
//        if (digitalRead(relay1) == HIGH)
//        {
//         Particle.publish("temperature", String(tempValue), PUBLIC);
//         count=0;
//         dump=tempValue;
//        }
//    }
//     count=count+1;
}

//Functions
int espressoToggle(String command) 
{
    if (command=="1") 
    {
        digitalWrite(relay1,HIGH);
        Particle.publish("Espresso", "ON", PRIVATE);
        return 1;
    }
    else if (command=="0") 
    {
        digitalWrite(relay2,LOW);
        Particle.publish("Boiler", "OFF", PRIVATE);
        delay(500);
        digitalWrite(relay1,LOW);
        Particle.publish("Espresso", "OFF", PRIVATE);
        return 0;
    }
    else if (command=="?") 
    {
        if (digitalRead(relay1) == HIGH)
        {
            Particle.publish("Espresso", "ON", PRIVATE);
            return 1;
        }
        else if (digitalRead(relay1) == LOW)
        {
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

int boilerToggle(String command) 
{
    if (command=="1") 
    {
        // if (digitalRead(relay1) == HIGH)
        // {
                digitalWrite(relay2,HIGH);
                Particle.publish("Boiler", "ON", PRIVATE);
                return 1;
        // }
        // else if (digitalRead(relay1) == LOW)
        // {
        //     digitalWrite(relay2,LOW);
        //     Particle.publish("Boiler", "OFF", PRIVATE);
        //     return 0;
        // }
        // else
        // {
        //     return -1;
        // }
        
    }
    else if (command=="0") 
    {
        digitalWrite(relay2,LOW);
        Particle.publish("Boiler", "OFF", PRIVATE);
        return 0;
    }
    else if (command=="?") 
    {
        if (digitalRead(relay2) == HIGH)
        {
            Particle.publish("Boiler", "ON", PRIVATE);
            return 1;
        }
        else if (digitalRead(relay2) == LOW)
        {
            Particle.publish("Boiler", "OFF", PRIVATE);
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