#include "UtilFrameRate.h"
#include "UtilSerializeBoost.h"

const FrameRate FrameRate::s24p = FrameRate(24 * 1000, 1001);
const FrameRate FrameRate::s25p = FrameRate(       25, 1);
const FrameRate FrameRate::s30p = FrameRate(30 * 1000, 1001);

typedef boost::tuple<wxString, FrameRate> FrameRateEntry;
const std::vector<FrameRateEntry> sPossibleFrameRates = boost::assign::tuple_list_of
    ( wxT("23.97"), FrameRate::s24p )
    ( wxT("25"),    FrameRate::s25p )
    ( wxT("29.97"), FrameRate::s30p );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FrameRate::FrameRate(int num, int den)
    :   boost::rational<int>(num,den)
    {
}
FrameRate::FrameRate(AVRational avr)
    :   boost::rational<int>(avr.num,avr.den)
{
}

FrameRate::FrameRate(wxString framerate)
    :   boost::rational<int>(FrameRate::s25p) // Default value. Only used if the given framerate is unknown.
{
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        if (framerate.IsSameAs(boost::get<0>(sPossibleFrameRates[i])))
        {
            assign(boost::get<1>(sPossibleFrameRates[i]).numerator(), boost::get<1>(sPossibleFrameRates[i]).denominator());
            break;
        }
    };
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

// static
std::vector<FrameRate> FrameRate::getSupported()
{
    std::vector<FrameRate> result;
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        result.push_back(boost::get<1>(sPossibleFrameRates[i]));
    };
    return result;
}

wxString FrameRate::toString() const
{
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        if (*this == boost::get<1>(sPossibleFrameRates[i]))
        {
            return boost::get<0>(sPossibleFrameRates[i]);
        }
    };
    wxString result; result << numerator() << "/" << denominator();
    return result;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const FrameRate& obj )
{
    os << std::setw(8) << obj.numerator() << std::setw(8) << obj.denominator();
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void FrameRate::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object< boost::rational<int> >(*this);
}
template void FrameRate::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void FrameRate::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);