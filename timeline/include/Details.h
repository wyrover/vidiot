// Copyright 2013 Eric Raijmakers.
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

#ifndef DETAILS_H
#define DETAILS_H

namespace gui { namespace timeline {

class DetailsPanel;
class Timeline;

class Details
    :   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Details(wxWindow* parent, Timeline* timeline);
    virtual ~Details();

    void update();

    //////////////////////////////////////////////////////////////////////////
    // GET A CERTAIN DETAILS TYPE
    //////////////////////////////////////////////////////////////////////////

    template <typename DETAILS>
    DETAILS* get()
    {
        for (DetailsPanel* details : mDetails)
        {
            DETAILS* result = dynamic_cast<DETAILS*>(details);
            if (result)
            {
                return result;
            }
        }
        FATAL("Not found");
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    wxWindow* getCurrent() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxStaticText* mHeader;
    wxWindow* mCurrent;
    std::list<DetailsPanel*> mDetails;

};

}} // namespace

#endif // DETAILS_H