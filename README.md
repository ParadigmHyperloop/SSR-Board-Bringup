# SSR Board Interface

This repo contains the source code used to bring up the SSR
Boards on the Paradigm Hyperloop Pod

## Background

The SSR Boards contain Solid State Relays to control 24V power
to valves and other devices. It also contains 4-20mA transmitters
which are used to control devices that require a variable current
setpoint.

## Board Specs (v2.1)

As of version 2.1 of the [SSR Board]() there are:

- 16 Total Outputs
  - 13 24V DC Solid State Relays (Binary On / Off States)
  - 3 variable 4-20mA current setpoint transievers (XTR1111)
- 1 [PCA9685](https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf) I2C PWM Generator

The board's control circuitry (the PCA9685) and other components are powered by a 5V
input jack or via the 5V port on the I2C Header.  There are 16 24V input ports for each
of the 16 device inputs, 13 SSR Outputs, and 3 Current Setpoint Outputs.  See the KiCAD
schematics 