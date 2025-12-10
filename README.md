Alarm clock app using wxWidgets that has 7 user definable alarms.
There is no installation script yet, but just
mkdir ~/.config/wxAlarmClock and copy wxAlarmClock.json and Alarms.json into it.


The wxAlarmClock.json file is still very primitive. The only counter-intuitive item is "volume"
This controls the volume of the alarm when it plays. It is a percentage of the maximum volume
of your sound system.

A neat feature of this program is that while the alarm is playing it polls XScreenSaverQueryInfo
and compares the value of idle member of the XScreenSaverInfo struct to the previously queried value
to test if the user pressed any key or moved their mouse, and if so playing is aborted. This lets the
user shut the alarm off even if the monitor is turned off, since the focus of the app is not relevent. 

This program uses the Pulse Audio server. If you are using a different system then you'll need
to rewrite pulse_audio.cpp to work with your system. The two functions, get_old_vol() and set_volume()
just construct system commands to perform their tasks, so it shouldn't be too difficult.
