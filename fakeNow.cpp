#include <iostream>
#include <ostream>
#include <string>
#include <map>
#include <wx/datetime.h>
#include <wx/file.h>


// Simulate a settable realtime clock which is scaled to 1 minute per call.
// When called by a func that runs once per second fakeNow provides a high speed
// wxDateTime result scaled to 1 minute per second (60:1) to speed up testing.
//
// *** currently this simulated clock returns regular 1:1 scale time and is being
//      used as a replacement for wxDateTime::Now() calls.
//
// To set the clock use WHILE THE CLOCK IS RUNNING:
//                      echo -n "Sun 05:59" > /tmp/fakeNow ; sleep 2; echo -n "" > /tmp/fakeNow

wxDateTime& fakeNow(bool inc){
    static wxDateTime dt = wxDateTime::Now();
    static wxDateTime::wxDateTime_t seconds = dt.GetSecond();
    static wxDateTime::wxDateTime_t minutes = dt.GetMinute();
    static wxDateTime::wxDateTime_t hours = dt.GetHour();
    static wxDateTime::wxDateTime_t days = dt.GetWeekDay();

    //fakeNow gets called multiple times per second, but should only update dt
    // once per second, so if inc == false just return the current value,
    // else fallthrough to the updating code
    if(inc == false) return dt;

    // here is where we can set the clock by means of the file in /tmp
    //wxFile data = wxFile("/tmp/fakeNow");
    
    // if(data.IsOpened()){
    //     wxString str = "";
    //     data.ReadAll(&str);
    //     std::cout << "/tmp/fakeNow = " << str.c_str() << std::endl;
    
    //     char day[4];

    //     std::map<std::string, int> dayOfWeekMap = { // takes string, returns int
    //         {"Sun", 0}, {"Mon", 1}, {"Tue", 2}, {"Wed", 3},
    //         {"Thu", 4}, {"Fri", 5}, {"Sat", 6}
    //     };
    
    //     // an empty string from the file will skip setting the clock and allow it to run
    //     if(str.Length() > 0){
    //         unsigned int h, m;

    //         std::sscanf(str, "%s %u:%u", day, &h, &m);
    //         days = dayOfWeekMap[day];
    //         hours = (wxDateTime::wxDateTime_t) h;
    //         minutes = (wxDateTime::wxDateTime_t) m;
    //     }
    //     data.Close();
    // } 

    if(++seconds == 60){
        seconds = 0;
        if(++minutes == 60){
            minutes = 0;
            if(++hours == 24){
                hours = 0;
                if(++days == wxDateTime::Inv_WeekDay){
                    days = (wxDateTime::WeekDay) 0;
                }
            }
        }
    }

    dt.SetToWeekDayInSameWeek((wxDateTime::WeekDay) days);
    dt.SetHour(hours);
    dt.SetMinute(minutes);
    dt.SetSecond(seconds);

    //printf("fakeNow(): %s\n", dt.Format("%a %H:%M:%S").ToStdString().c_str());
    
    return dt;
}