// class that shows a dialog to create, edit, enable/disable, or delete multiple alarms
// buttons on bottom row: [Add New] [Remove Disabled] [Close]


#include "wx/colour.h"
#include "wx/dialog.h"
#include "wx/choice.h"
#include "wx/event.h"
#include "wx/checkbox.h"
#include "wx/filedlg.h"
#include "wx/button.h"
#include "wx/setup.h"
#include "wx/spinctrl.h"
#include "wx/textctrl.h"
#include "wx/tglbtn.h"
#include "wx/sizer.h"
#include "wx/button.h"
#include "wx/window.h"
#include "wx/frame.h"


class Entry;

struct AlarmTime {
std::string dayAndTime;
std::string note;
};

int parseTime(const std::string&);

class AlarmsDlg : public wxDialog
{
public:
    AlarmsDlg(wxWindow *parent, wxColor fg, wxColor bg);
    wxColor fgcol; // enabled state for save button
    wxColor bgcol;
    wxColor disbg = 0x373737; // disabled state for save button
    wxColor disfg = 0x4B4B4B;
    wxBoxSizer *dlg_entriesSizer; // display area in the dialog of list of entries
    wxBoxSizer *dlg_sizerButtons; // sizer container for dialog control buttons
    wxBoxSizer *dlg_mainSizer;    // master sizer for the dialog
    wxButton *dlg_close;
    wxButton *dlg_save;
    std::vector<Entry *> entryVec;
    std::vector<AlarmTime>& getAlarms();
    void OnClose(wxCommandEvent &);
    void OnSave(wxCommandEvent &);
    void OnDirty(wxCommandEvent &);
};

class Entry : public wxEvtHandler
{
public:
    Entry(wxDialog *parent, int n);

    int entry_number;
    wxCheckBox *entry_checkBoxEnable;
    wxChoice *entry_choiceDay;
    wxSpinCtrl *entry_spinHour;
    wxSpinCtrl *entry_spinMinute;
    wxChoice *entry_choiceAMPM;
    wxTextCtrl  *entry_textCtrlNote;
    wxFileDialog  *entry_fileDialogSound;
    wxButton    *entry_buttonSndFile;
    wxBoxSizer *entrySizer;
    
    void set_colors(wxColor fg, wxColor bg);
    void OnSoundFileButtonClicked(wxCommandEvent &); 

};