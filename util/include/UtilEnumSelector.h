// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef UTIL_ENUM_SELECTOR
#define UTIL_ENUM_SELECTOR

#include "UtilLog.h"

template <class ITEMTYPE>
class EnumSelector
    : public wxChoice
{
public:
    typedef boost::bimap<ITEMTYPE,wxString> Mapping;
    EnumSelector(wxWindow* parent, Mapping mapping, ITEMTYPE default )
        :   wxChoice(parent, wxID_ANY)
        ,   mMapping(mapping)
    {
        int index = 0;
        for ( Mapping::left_reference entry : mMapping.left )
        {
            Append(entry.second);
            mSelectionToItem[index] = entry.first;
            mItemToSelection[entry.first] = index;
            index++;
        }
        select(default);
    }
    virtual ~EnumSelector()
    {
    }
    ITEMTYPE getValue()
    {
        return mSelectionToItem[GetSelection()];
    }
    ITEMTYPE getItem(int index)
    {
        ASSERT_MAP_CONTAINS(mSelectionToItem,index);
        return mSelectionToItem[index];
    }
    int getIndex(ITEMTYPE item)
    {
        ASSERT_MAP_CONTAINS(mItemToSelection,item);
        return mItemToSelection[item];
    }
    void select(ITEMTYPE value)
    {
        SetSelection(getIndex(value));
    }
private:
    Mapping mMapping;
    std::map<int,ITEMTYPE> mSelectionToItem;
    std::map<ITEMTYPE,int> mItemToSelection;
};

#endif
