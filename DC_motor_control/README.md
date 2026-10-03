# Electronics Design - DC Motor Control

Designed a controller for a DC motor with a Hall effect encoder, using an object-oriented approach in C++. Inputs for forward/reverse and on/off were taken from push buttons (with software debouncing), and speed was taken from a potentiometer. Interrupts were used to monitor the hall effect sensor and calculate speed. Different control strategies were used including PID tuned 'by eye' and using the Ziegler-Nichols Method. A strategy to control the position of the shaft (rather than its speed) was also developed.
