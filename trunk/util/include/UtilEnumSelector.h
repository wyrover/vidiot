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
        BOOST_FOREACH( Mapping::left_reference entry, mMapping.left )
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

#endif // UTIL_ENUM_SELECTOR