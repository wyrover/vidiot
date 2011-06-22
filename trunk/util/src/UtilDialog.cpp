#include "UtilDialog.h"

#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/textdlg.h>
#include <boost/foreach.hpp>

boost::optional<wxString> UtilDialog::sDir = boost::none;
boost::optional<std::list< wxString > > UtilDialog::sFiles = boost::none;
boost::optional<wxString> UtilDialog::sText = boost::none;

// static
void UtilDialog::setDir(wxString dir)
{
    sDir = boost::optional<wxString>(dir);
}

// static
wxString UtilDialog::getDir( const wxString & message, const wxString & default, wxWindow* parent )
{
    if (sDir)
    {
        wxString result = *sDir;
        sDir.reset();
        return result;
    }
    return wxDirSelector( message, default, wxDD_DEFAULT_STYLE, wxDefaultPosition, parent ); // Parent given to ensure modality
}

// static
void UtilDialog::setFiles(std::list<wxString> files)
{
    sFiles = boost::optional<std::list< wxString > >(files);
}

// static
std::list<wxString> UtilDialog::getFiles( const wxString& message, const wxString& filetypes, wxWindow* parent )
{
    std::list<wxString> result;
    if (sFiles)
    {
        result = *sFiles;
        sFiles.reset();
    }
    else
    {
        wxString wildcards = wxString::Format(filetypes,wxFileSelectorDefaultWildcardStr,wxFileSelectorDefaultWildcardStr);
        wxFileDialog dialog(parent, message, wxEmptyString, wxEmptyString, wildcards, wxFD_OPEN|wxFD_MULTIPLE);
        if (dialog.ShowModal() == wxID_OK)
        {
            wxArrayString paths;
            dialog.GetPaths(paths);
            BOOST_FOREACH( wxString path, paths )
            {
                result.push_back(path);
            }
        }
    }
    return result;
}

// static
void UtilDialog::setText(wxString text)
{
    sText = boost::optional<wxString>(text);
}

// static
wxString UtilDialog::getText( const wxString & title, const wxString & message, const wxString & default, wxWindow* parent )
{
    if (sText)
    {
        wxString result = *sText;
        sText.reset();
        return result;
    }
    return wxGetTextFromUser( message, title, default, parent, wxDefaultCoord, wxDefaultCoord, true);
}

