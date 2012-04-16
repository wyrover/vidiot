#ifndef UTIL_ENUM_SELECTOR
#define UTIL_ENUM_SELECTOR

#include "UtilLog.h"
#include <boost/foreach.hpp>

template <class ITEMTYPE>
class EnumSelector
    : public wxChoice
{
public:
    typedef std::map<ITEMTYPE,wxString> Mapping;
    EnumSelector(wxWindow* parent, std::map<ITEMTYPE,wxString> mapping, ITEMTYPE default )
        :   wxChoice(parent, wxID_ANY)
    {
        int index = 0;
        int selection = -1;
        BOOST_FOREACH( Mapping::value_type entry, mapping )
        {
            Append(entry.second);
            if (entry.first == default)
            {
                selection = index;
            }
            mSelectionToItem[index] = entry.first;
            index++;
        }
        ASSERT_MORE_THAN_EQUALS_ZERO(selection);
        SetSelection(selection);
    }
    virtual ~EnumSelector()
    {
    }
    ITEMTYPE getValue()
    {
        return mSelectionToItem[GetSelection()];
    }
private:
    std::map<int,ITEMTYPE> mSelectionToItem;
};

#endif // UTIL_ENUM_SELECTOR