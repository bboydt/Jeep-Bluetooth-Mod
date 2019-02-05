# Jeep Bluetooth Mod

## About
When I started driving, I wanted Bluetooth in my 2004 Jeep Grand Cherokee. Several years later, I finally got around to building a Bluetooth mod. My radio has a plug for a cd changer, so I used an Arduino and a Bluetooth module to make a pseudo cd changer. It took me about a month, however I procrastinated a lot. [Here's a video of it working](https://youtu.be/_DS7fCqb5ws).

## Useful Info

 - My radio model is p05064354aj.
 - I'm using an Arduino UNO and a XS-3868 Bluetooth module
 - Chrysler radios, made before 2004, use the car’s internal PCI bus for communication with other accessories (i.e. a cd changer). It’s a good idea to be careful when writing to this bus because you could damage your car.
 - My car uses the J1850 VPW protocol.
 - Some good vocabulary to know for this protocol:
    - **Frame** : a packet of data
    - **Cyclical Redundancy Check (CRC)** : it’s a checksum
 - I used two optocouplers to communicate on the bus without frying my Arduino. Also, I put some LEDs on the optocouplers' outputs which was useful for debugging.

## Useful Links

 - [This has a lot of information on what to send to the radio to trick it into thinking you’re a cd changer.](https://www.mictronics.de/projects/cdc-protocols/#ChryslerJeep)
 - [This has good info on the J1850 protocol.](http://www.interfacebus.com/Automotive_SAE_J1850_Bus.html)
 - [A more technical document on J1850.](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.506.6682&rep=rep1&type=pdf)
 - [This guy’s project uses the same protocol so it was helpful.](https://github.com/connorwm/J1850VPW)
 - [Radio Pinout (there are also other radio pinouts here)](https://www.tehnomagazin.com/Auto-radio-car-connector/CHRYSLER-Car-Radio-Wiring-Connector.htm)

## Plans

 - I might move the J1850 VPW part of this project into its own library.
 - My current script for the Arduino is a little buggy, so it needs to be fixed up.

----

*Benjamin Thomas - boydbt48@gmail.com - 2019*
