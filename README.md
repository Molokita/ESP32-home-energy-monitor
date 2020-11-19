# ESP32-home-energy-monitor
Home Energy Monitor based on the Espressif ESP32s

Yet another ESP32-based power monitoring project.
What's the difference between this and other projects of the same kind?
I wanted to build a home power monitoring device as basic as possible using only MQTT to publish the data to my local broker, to later present it using openHAB. Also I wanted something to learn from which could be scalable to multiphase power monitoring with as little changes in code as needed.


Dependencies:
emonLib fork from Savjee: https://github.com/Savjee/EmonLib-esp32
ESPhelper Arduino library from ItKindaWorks: https://github.com/ItKindaWorks/ESPHelper
