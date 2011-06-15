#include "UtilDialog.h"

#include <wx/dirdlg.h>

boost::optional<wxString> UtilDialog::sDir = boost::none;

wxString UtilDialog::getDir( const wxString & message, const wxString & default_path, wxWindow* parent )
{
    if (sDir)
    {
        wxString result = *sDir;
        sDir.reset();
        return result;
    }
    return wxDirSelector( message, default_path, wxDD_DEFAULT_STYLE, wxDefaultPosition, parent ); // Parent given to ensure modality
}

void UtilDialog::setDir(wxString dir)
{
    sDir = boost::optional<wxString>(dir);
}
