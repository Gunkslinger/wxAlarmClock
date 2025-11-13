#include "alarmdia.hpp"
#include "wx/any.h"
#include "wx/event.h"
#include "wx/sizer.h"
#include "wx/stattext.h"
#include "enums.hpp"
#include <cstdio>
#include <ostream>


AlarmDia::AlarmDia()
    : wxFrame(nullptr, ID_DIA, "Alarm Dialog")
{
    SetSize(600, 200);

    wxColour bgcolour(55, 55, 55, 255);
    wxColour fgcolour(220, 220, 200, 255);
    wxColour butbgcolour(75, 75, 75, 255);
    wxColour butfgcolour(220, 220, 200, 255);

    SetForegroundColour(fgcolour);
    SetBackgroundColour(bgcolour);

    staticTextMessage = new wxStaticText(this, ID_MESSAGE, "Alarm Clock.\nHit any key to silence");
    sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(staticTextMessage);

    SetSizer(sizer);

    // Bind(wxEVT_KEY_DOWN, &TimeLineCTRL::keyPressed, this);
    Bind(wxEVT_KEY_DOWN, &AlarmDia::keyPressEvent, this, ID_DIA);
    
    Show();
    // alarmSound = new wxSound("./cuckoo2.wav");
    // alarmSound->Play(wxSOUND_ASYNC|wxSOUND_LOOP);
    printf("ALARM! ALARM! ALARM! from AlarmDia\n");

}

// Press a key to shut the alarm up -- not working
void AlarmDia::keyPressEvent(wxKeyEvent& event)
{
    printf("Key pressed\n");
    // alarmSound->Stop();
    //Close();
}