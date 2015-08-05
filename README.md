bebot-powerboard-bsp
====================

This repository contains the source for the two firmware images on the bebot-powerboard-bsp. The bebot-powerboard-bsp is a dual ATMega328P design, with one microcontroller responsible for power management and charging (always on), and a second microcontroller for sensing and actuating the base.

##LED Definitions for the PM Firmware##

| Battery LED | Charge LED | Meaning |
|-------------|------------|---------|
| ON          | ON         | Battery is present and fully charged |
| ON          | OFF        | Battery is present but not charging |
| ON          | BLINK      | Battery is present and charging |
| OFF         | ON         | Battery is not present, charger is ready  |
| OFF         | OFF        | Battery is not present, charger is in standby |
| OFF         | BLINK      | Battery is not present, charger has an error |
| BLINK       | ON         | Undefined fault |
| BLINK       | OFF        | Battery fault |
| BLINK       | BLINK      | Low battery |
