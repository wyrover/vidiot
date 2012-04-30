#ifndef UTIL_ENUM_SELECTOR
#define UTIL_ENUM_SELECTOR

#include <wx/choice.h>
#include "UtilLog.h"
#include <boost/foreach.hpp>

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
        //int selection = -1;
        BOOST_FOREACH( Mapping::left_reference entry, mMapping.left )
        {
            Append(entry.second);
            //if (entry.first == default)
            //{
            //    selection = index;
            //}
            mSelectionToItem[index] = entry.first;
            index++;
        }
        //ASSERT_MORE_THAN_EQUALS_ZERO(selection);
        //SetSelection(selection);
        select(default);
    }
    virtual ~EnumSelector()
    {
    }
    ITEMTYPE getValue()
    {
        return mSelectionToItem[GetSelection()];
    }
    void select(ITEMTYPE value)
    {
        int index = 0;
        BOOST_FOREACH( Mapping::left_reference entry, mMapping.left )
        {
            if (entry.first == value)
            {
                SetSelection(index);
                return;
            }
            index++;
        }
    }
private:
    Mapping mMapping;
    std::map<int,ITEMTYPE> mSelectionToItem;
};

#endif // UTIL_ENUM_SELECTOR