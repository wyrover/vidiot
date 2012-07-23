#include "IFile.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

IFile::IFile()
{
}

IFile::IFile(const IFile& other)
{
}

// todo remove cpp file????

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void IFile::serialize(Archive & ar, const unsigned int version)
{
    //ar & boost::serialization::base_object<IControl>(*this); // todo obsolete, since IControl serializes nothing????
}
template void IFile::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void IFile::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace