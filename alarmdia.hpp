#include "wx/event.h"
#include "wx/stattext.h"
#include "wx/sound.h"
#include "wx/wx.h"
#include "wx/sound.h"

class AlarmDia : public wxFrame
{
public:
    AlarmDia();

    wxStaticText *staticTextMessage;
    wxBoxSizer *sizer;
    wxSound *alarmSound;
private:
    void keyPressEvent(wxKeyEvent& event);
};