#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/event.h>
#include "wx/sound.h"
#include "pulse_audio.hpp"

class AlarmControlFrame : public wxFrame
{
public:
    AlarmControlFrame();
    wxSpinCtrl *spinHour;
    wxSpinCtrl *spinMinute;
    wxToggleButton *toggleButtonAMPM;
    wxButton *buttonStartStop;
    wxStaticText* labelAlarmTime;
    wxBoxSizer *spinSizer;
    wxBoxSizer *mainsizer;
    wxTimer *timer;
    wxSound *alarm;
    std::string old_volume;
    void OnExit(wxCommandEvent& event);
    void OnToggle(wxCommandEvent& event);
    void StartStop(wxCommandEvent& event);
    void CountDown(wxTimerEvent& event);
    void keyPressEvent(wxKeyEvent& event);
};