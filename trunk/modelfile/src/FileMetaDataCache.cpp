// Copyright 2015-2016 Eric Raijmakers.
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

#include "FileMetaDataCache.h"

#include <boost/serialization/map.hpp>
#include "AudioPeaks.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"

namespace model {

    struct FileMetaData
    {
        FileMetaData() {}
        explicit FileMetaData(wxDateTime lastmodified)
            : LastModified{ lastmodified.IsValid() ? lastmodified : wxDateTime::UNow() } // !IsValid: Could happen if file was deleted just before the cache entry is created.
    {
    }

    wxDateTime LastModified;
    boost::optional<pts> Length = boost::none;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(LastModified);
        ar & BOOST_SERIALIZATION_NVP(Length);
        if (version < 2)
        {
            // In version 2, the peaks were moved to the AudioClip class.
            boost::optional< AudioPeaks > Peaks;
            ar & BOOST_SERIALIZATION_NVP(Peaks);
        }
    }
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FileMetaDataCache::FileMetaDataCache()
{
    VAR_DEBUG(this);
}

FileMetaDataCache::~FileMetaDataCache()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

boost::optional<pts> FileMetaDataCache::getLength(const wxFileName& file)
{
    boost::mutex::scoped_lock lock(mMutex);
    return getDataForFile(file)->Length;
}

void FileMetaDataCache::setLength(const wxFileName& file, const pts& length)
{
    boost::mutex::scoped_lock lock(mMutex);
    getDataForFile(file)->Length.reset(length);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

FileMetaDataPtr FileMetaDataCache::getDataForFile(const wxFileName& file)
{
    // NOT: boost::mutex::scoped_lock lock(mMutex); -- Lock taken in calling method.
    auto ret = mMetaData.insert(std::make_pair(file, boost::make_shared<FileMetaData>(file.GetModificationTime()))); // Inserts new item if no entry present yet.
    FileMetaDataPtr data = ret.first->second;
    if (file.Exists())
    {
        wxDateTime currentFileTime{ file.GetModificationTime() };
        ASSERT(currentFileTime.IsValid())(file);
        wxDateTime cachedFileTime{ data->LastModified };
        ASSERT(cachedFileTime.IsValid())(file);
        currentFileTime.SetMillisecond(0); // Sometimes get 012 milliseconds changes within
        cachedFileTime.SetMillisecond(0);  // one test case. Ignore changes < 1 second.
        if (!currentFileTime.IsEqualTo(cachedFileTime))
        {
            VAR_INFO(file.GetLongPath())(currentFileTime)(cachedFileTime);
            mMetaData.erase(ret.first);
            return getDataForFile(file); // Will cause insert of new empty item.
        }
    }
    // else: File deleted already return last known values?
    return data;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const FileMetaDataCache& obj)
{
    os  << &obj;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void FileMetaDataCache::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_NVP(mMetaData);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void FileMetaDataCache::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void FileMetaDataCache::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

template void FileMetaData::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void FileMetaData::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::FileMetaDataCache)
BOOST_CLASS_VERSION(model::FileMetaData, 2)