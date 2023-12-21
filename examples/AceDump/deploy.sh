#!/bin/sh -xe

set -e

#arduino-cli lib install PinChangeInterrupt
# arduino-cli compile --fqbn aceduino:avr:m168xt4m AceDump
# arduino-cli upload -p /dev/ttyUSB0 --fqbn aceduino:avr:m168xt4m AceDump
arduino-cli upload -P avrispmk2 --fqbn aceduino:avr:m168xt4m AceDump


arduino-cli compile -e --clean --fqbn aceduino:avr:m168xt4m AceDump/examples/AceDump

arduino-cli upload --verify --programmer avrispmkii --fqbn aceduino:avr:m168xt4m AceDump/examples/AceDump
