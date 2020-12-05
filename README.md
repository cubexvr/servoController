4 channel controller code for arduino mega for AASD-15A and similar driver

arduino sketch for generating those pulses needed to control servo drivers such as the AASD-15A used in the sfx100 project and the pt-actuators. The specific parameters are meant for the sfx100 actuator (ca. 110 mm stroke, aasd-driver, 90st motor) but it could easily be adapted to other configurations. There are currently no convenience/security functions. It currently only works with the arduino mega 2560 because I'm using hardware timer registers directly for better performance. For smoother movement I do interpolation, i.e. I distribute the pulses evenly across the sample interval.

see also https://www.xsimulator.net/community/threads/4dof-controller-code-for-arduino-mega-for-aasd-15a-and-similar-driver.14421/

