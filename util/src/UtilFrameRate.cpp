#include "UtilFrameRate.h"

#include <vector>
#include <boost/tuple/tuple.hpp>
#include <boost/assign/list_of.hpp>

namespace framerate {

typedef boost::tuple<wxString, FrameRate> FrameRateEntry; 
const std::vector<FrameRateEntry> sPossibleFrameRates = boost::assign::tuple_list_of
( wxT("24.97"), s24p )
( wxT("25"),    s25p )
( wxT("29.97"), s30p );

std::vector<FrameRate> getSupported()
{
    std::vector<FrameRate> result;
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        result.push_back(boost::get<1>(sPossibleFrameRates[i]));
    };
    return result;
}

wxString toString(FrameRate framerate)
{
    wxString result;
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        if (framerate == boost::get<1>(sPossibleFrameRates[i]))
        {
            result = boost::get<0>(sPossibleFrameRates[i]);
        }
    };
    return result;
}

FrameRate fromString(wxString framerate)
{
    FrameRate result = framerate::s25p;
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        if (framerate.IsSameAs(boost::get<0>(sPossibleFrameRates[i])))
        {
            result = boost::get<1>(sPossibleFrameRates[i]);
            break;
        }
    };
    return result;
};

} // namespace