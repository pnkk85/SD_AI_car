#!/bin/bash

# configure CAN
sudo slcand -o -s8 -t hw -S 3000000 /dev/ttyACM0 slcan0
sudo ifconfig slcan0 up
