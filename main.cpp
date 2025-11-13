#include <wx/wx.h>
#include "mainwindow.hpp"

class MyApp : public wxApp
{
public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    AlarmControlFrame *AlarmCTRLframe = new AlarmControlFrame();
    AlarmCTRLframe->Show(true);
    return true;
}
