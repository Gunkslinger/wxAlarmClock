Alarm clock app using wxWidgets that has 7 user definable alarms.
There is not installation script yet, but just
mkdir ~/.config/wxAlarcmClock and copy wxAlarmClock.json and Alarms.json into it.


The wxAlarmClock.json file is still very primitive. The only counter-intuitive item is "volume"
This controls the volume of the alarm when it plays. It is a percentage of the maximum volume
of your sound system.

A neat feature of this program is that it polls XScreenSaverQueryInfo and compares the idle member
of the XScreenSaverInfo struct to the previously queried value to test if the user pressed any key
or moved their mouse while the alarm is playing, and if so playing is aborted. This lets the user
shut the alarm off even if the monitor is turned off, since the focus of the app is not relevent. 
