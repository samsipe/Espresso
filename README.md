# Espresso

A [Particle](https://www.particle.io) Photon setup to control my Espresso Machine using solid state relays. The particle API is then connected to my Homebridge and Apple HomeKit, so I can automatically heat up my espresso machine in the mornings.

## Overview

There are two relays wired in parallel with the main and boiler switch on the espresso machine, meaning the machine can be turned on manually, by the relays, or both, without issue.

Additionally, there are two temperature sensors that get the case temperature and the temperature of the group head.

The code also allows for automatic shutoff, in case the espresso machine is left on for longer than four hours.

[Here is an image](images/photon_and_relays.jpeg) of the Photon and the relays, and [an image of how it is wired](images/ac_wiring.jpeg) into the espresso machine.

## To-Do

While my espresso machine doesn't need a PID (since it's a heat exchanger) having more control over the group head brew temperature would be useful. I plan to add a PID on the boiler temperature and a way to set it remotely via particle.

## Reach Out

If you want to know more about this project, or build your own IoT espresso machine, let me know.
