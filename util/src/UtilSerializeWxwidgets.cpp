#include "UtilSerializeWxwidgets.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace boost { namespace serialization {

template<class Archive>
void save(Archive & ar, const wxString & string, const unsigned int version)
{
    std::string s = string.ToStdString();
    ar & s;
}

template<class Archive>
void load(Archive & ar, wxString & string, const unsigned int version)
{
    std::string s;
    ar & s;
    wxString input(wxSafeConvertMB2WX(s.c_str()));
    string = input;
}

template void save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const wxString & string, const unsigned int version);
template void load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar,       wxString & string, const unsigned int version);

template<class Archive>
void save(Archive & ar, const wxFileName& filename, const unsigned int version)
{
    ar & filename.GetLongPath();
}

template<class Archive>
void load(Archive & ar, wxFileName& filename, const unsigned int version)
{
    wxString longpath;
    ar & longpath;
    filename.Assign(longpath);
}

template void save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const wxFileName& filename, const unsigned int version);
template void load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar,       wxFileName& filename, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxRect & r, const unsigned int version)
{
    ar & r.x;
    ar & r.y;
    ar & r.width;
    ar & r.height;
}

template void serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, wxRect & r, const unsigned int version);
template void serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, wxRect & r, const unsigned int version);

template<class Archive>
void save(Archive & ar, const wxRegion & region, const unsigned int version)
{
    int n = 0;
    wxRegionIterator it(region);
    while (it)
    {
        n++;
        it++;
    }
    ar & n;
    it.Reset();
    while (it)
    {
        wxRect rect = it.GetRect(); // Needed for GCC
        ar & rect;
        it++;
    }
}

template<class Archive>
void load(Archive & ar, wxRegion & region, const unsigned int version)
{
    int n = 0;
    ar & n;
    wxRect r;
    while (n > 0)
    {
        ar & r;
        region.Union(r);
        n--;
    }
}

template void save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const wxRegion & region, const unsigned int version);
template void load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar,       wxRegion & region, const unsigned int version);

}} // namespace boost::serialization

