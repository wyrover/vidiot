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

#pragma once

#include "UtilSingleInstance.h"

#include "AudioPeaks.h"
#include "UtilInt.h"

namespace model {

class AudioPeaks;

struct FileMetaData;
typedef boost::shared_ptr<FileMetaData> FileMetaDataPtr;

class FileMetaDataCache
    :   public SingleInstance<FileMetaDataCache>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    FileMetaDataCache();

    virtual ~FileMetaDataCache();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    boost::optional<pts> getLength(const wxFileName& file);
    void setLength(const wxFileName& file, const pts& length);

    boost::optional<AudioPeaks> getPeaks(const wxFileName& file);
    void setPeaks(const wxFileName& file, const AudioPeaks& peaks);

protected:

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////


    std::map<wxFileName, FileMetaDataPtr> mMetaData;

    mutable boost::mutex mMutex; // todo replace boost threading with std threading

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// This method also verifies if the stored data was created for the file
    /// with the same modification time. If not, then any stored information
    /// is erased and nullptr is returned.
    FileMetaDataPtr getDataForFile(const wxFileName& file);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const File& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::FileMetaDataCache, 1)
BOOST_CLASS_EXPORT_KEY(model::FileMetaDataCache)
