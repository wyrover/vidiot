// Copyright 2015 Eric Raijmakers.
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

#include "AudioPeaks.h"
#include <boost/serialization/map.hpp>

namespace model {

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

// todo store last modified dates/times for all files in the cache, and invalidate when no longer valid.
// todo watch all these files. or do the check upon each get?

bool FileMetaDataCache::hasPeaks(const wxString& file) const
{
    // todo thread safety
//    ASSERT(wxThread::IsMain()); // To ensure thread safety
    return mPeaks.find(file) != mPeaks.end();
}

const AudioPeaks& FileMetaDataCache::getPeaks(const wxString& file) const
{

    // todo do the scheduling here. If there is no 'data available', schedule the generation and return '0'.
    // then add event 'meta data updated' for the underlying object (audiofile/clip in this case), and forward that
    // to the view class?
    // todo thread safety
//    ASSERT(wxThread::IsMain()); // To ensure thread safety
    ASSERT_MAP_CONTAINS(mPeaks, file);
    return mPeaks.find(file)->second;
}

void FileMetaDataCache::setPeaks(const wxString& file, const AudioPeaks& peaks)
{
    // todo thread safety
    mPeaks[file] = peaks;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

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
        ar & BOOST_SERIALIZATION_NVP(mPeaks);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void FileMetaDataCache::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void FileMetaDataCache::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::FileMetaDataCache)
