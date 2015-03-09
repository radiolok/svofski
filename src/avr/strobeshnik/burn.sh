#!/bin/sh
kill -HUP `ps ax | grep -e "[0-9] screen.*SLAB" | sed 's/^ //' |  cut -f 1 -d ' '`

sleep 2
./mega8_bootload strobeshnik.bin /dev/tty.SLAB_USBtoUART 19200
