// Copyright 2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "UtilMail.h"

namespace util { namespace mail {

const wxString toAddress{"epra@users.sourceforge.net"};
const wxString toName{"Eric Raijmakers"};

#ifdef _MSC_VER

#include <windows.h>
#include <mapi.h>

bool sendDebugReport(wxString subject, wxString body, boost::optional<wxString> file)
{
    // Original source: http://www.codeproject.com/Articles/12757/Programmatically-adding-attachments-to-emails
    // Licensed under The Code Project Open License (CPOL) 1.02 (http://www.codeproject.com/info/cpol10.aspx)
    HINSTANCE hMAPI = ::LoadLibraryA(("MAPI32.DLL"));
    if (!hMAPI) { return false; }

    ULONG(PASCAL *SendMail)(ULONG, ULONG_PTR, MapiMessage*, FLAGS, ULONG);
    (FARPROC&)SendMail = GetProcAddress(hMAPI, ("MAPISendMail"));
    if (!SendMail)
    {
        return false;
    }

    MapiMessage message;
    ::ZeroMemory(&message, sizeof(message));
    MapiFileDesc fileDesc;
    ::ZeroMemory(&fileDesc, sizeof(fileDesc));
    MapiRecipDesc recipientsDesc[1];
    ::ZeroMemory(&recipientsDesc[0], sizeof(MapiRecipDesc));

    if (file &&
        util::path::toFileName(*file).FileExists())
    {
        fileDesc.nPosition = (ULONG)-1;
        const char* s = file->mb_str();
        fileDesc.lpszPathName = (LPSTR)s;
        fileDesc.lpszFileName = (LPSTR)s;
        message.nFileCount = 1;
        message.lpFiles = &fileDesc;
    }

    const char* sSubject = subject.mb_str();
    const char* sBody = body.mb_str();
    message.lpszSubject = const_cast<char*>(sSubject);
    message.lpszNoteText = const_cast<char*>(sBody);
    message.nRecipCount = 1;
    message.lpRecips = &recipientsDesc[0];
    const char* sAddress = toAddress.mb_str();
    message.lpRecips[0].lpszAddress = const_cast<char*>(sAddress);
    message.lpRecips[0].lpEntryID = 0;
    const char* sName = toName.mb_str();
    message.lpRecips[0].lpszName = const_cast<char*>(sName);
    message.lpRecips[0].ulEIDSize = 0;
    message.lpRecips[0].ulRecipClass = MAPI_TO;

    int nError = SendMail(0, (ULONG_PTR)0, &message, MAPI_LOGON_UI | MAPI_DIALOG, 0);
    if (nError != SUCCESS_SUCCESS &&
        nError != MAPI_USER_ABORT &&
        nError != MAPI_E_LOGIN_FAILURE)
    {
        return false;
    }

    return true;
}

#else // _MSC_VER

bool sendDebugReport(wxString subject, wxString body, boost::optional<wxString> file)
{
    wxString command;
    wxString xdg{"/usr/bin/xdg-email"};

    if (!util::path::toFileName(xdg).IsFileExecutable())
    {
        return false;
    }

    if (file)
    {
        body += _("\nPlease attach the file '") + *file + _("' if it was not yet attached to this mail.\nUnfortunately, some email clients do not support adding attachments automatically.\n");
    }

    command << xdg
            << " --subject \"" << subject << "\" "
            << " --body \"" << body << "\" ";
    if (file)
    {
        command << " --attach " << *file << " ";
    }
    command << "\"" << toName << " <" << toAddress << ">\" " // Added twice because xdg-email prefixes the first address with ///
            << "\"" << toName << " <" << toAddress << ">\" ";
    VAR_ERROR(command);

    wxExecute(command);
    return true;
}

#endif

}} // namespace
