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

#include "UtilLocale.h"

#include "Config.h"
#include "UtilPath.h"

std::vector<std::pair<wxString, wxString>> getSupportedLanguages()
{
    std::vector<std::pair<wxString, wxString>> result;

    std::map<wxString, wxString> sLanguages =
    {
        { "en", _("English") },
        { "de", _("German") },
        { "nl", _("Dutch") },
    };

    wxDir dir{ util::path::toPath(util::path::getLanguagesPath()) };
    wxString filename;
    for (bool cont = dir.GetFirst(&filename, "*", wxDIR_DIRS); cont; cont = dir.GetNext(&filename))
    {
        const wxLanguageInfo* langinfo{ wxLocale::FindLanguageInfo(filename) };
        if(langinfo != nullptr)
        {
            if (wxFileExists(dir.GetName() + wxFileName::GetPathSeparator() + filename + wxFileName::GetPathSeparator() + "vidiot.mo"))
            {
                wxString languageName{ langinfo->Description };
                if (sLanguages.find(filename) != sLanguages.end())
                {
                    // Use translatable language name if available
                    languageName = sLanguages.find(filename)->second;
                }
                result.push_back(std::make_pair(languageName, filename));
            }
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

wxString getDefaultLanguage()
{
    int systemLanguageId{ wxLocale::GetSystemLanguage() };
    if (systemLanguageId != wxLANGUAGE_UNKNOWN)
    {
        const wxLanguageInfo* langinfo = wxLocale::GetLanguageInfo(systemLanguageId);
        if (langinfo != nullptr)
        {
            return langinfo->CanonicalName;
        }
    }
    return "en";
}

wxString getLanguageCode()
{
    return Config::ReadString(Config::sPathWorkspaceLanguage);
}

wxLanguage getLanguageId(wxString languageCode)
{
    const wxLanguageInfo* langinfo = wxLocale::FindLanguageInfo(languageCode);
    if (langinfo == nullptr) { return wxLANGUAGE_ENGLISH; }
    return static_cast<wxLanguage>(langinfo->Language);
}
