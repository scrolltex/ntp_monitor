# NTP status monitor

## What is it for?
This program gets the status ntp and pps from ntpq and displays it in the window, coloring the state of the element.
In addition, the application displays the current time and date on this system.

Also in the program there is a button that reloads ntp server by executing folowing command:
```
sudo service ntp restart
```


## What do the colors mean?
### Time color:
* Green - offset < 0.1.
* Yellow - offset >= 0.1.
* Red - somethig wrong in status

### PPS block color:
* Green - pps is active.
* Yellow - pps not active.
* Red - somethig wrong in status

### GPS block color:
* Green - no errors in last 8 attempts.
* Yellow - have errors int last 8 attempts.
* Red - somethig wrong in status



## Stuff used on development stage

 * [Qt](https://www.qt.io/) v5.8
 * [ntpq dummy](ntpq_dummy) app for testing on windows
