// Copyright 2013-2015 Eric Raijmakers.
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

#pragma once

#include "UtilLog.h"

template <class ITEMTYPE>
class EnumSelector
    : public wxChoice
{
public:
    typedef std::map<ITEMTYPE,wxString> Mapping;
    EnumSelector(wxWindow* parent, const Mapping& mapping, const ITEMTYPE& defaultValue )
        :   wxChoice(parent, wxID_ANY)
        ,   mMapping(mapping)
    {
        int index = 0;
        for ( auto entry : mMapping )
        {
            Append(entry.second);
            mSelectionToItem[index] = entry.first;
            mItemToSelection[entry.first] = index;
            index++;
        }
        select(defaultValue);
    }
    EnumSelector(const EnumSelector&) = delete;
    EnumSelector& operator=(const EnumSelector&) = delete;
    ~EnumSelector() = default;

    ITEMTYPE getValue()
    {
        return mSelectionToItem[GetSelection()];
    }
    ITEMTYPE getItem(int index)
    {
        ASSERT_MAP_CONTAINS(mSelectionToItem,index);
        return mSelectionToItem[index];
    }
    int getIndex(const ITEMTYPE& item)
    {
        ASSERT_MAP_CONTAINS(mItemToSelection,item);
        return mItemToSelection[item];
    }
    void select(const ITEMTYPE& value)
    {
        SetSelection(getIndex(value));
    }
private:
    Mapping mMapping;
    std::map<int,ITEMTYPE> mSelectionToItem;
    std::map<ITEMTYPE,int> mItemToSelection;
};
