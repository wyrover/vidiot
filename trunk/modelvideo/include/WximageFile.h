// Copyright 2014-2015 Eric Raijmakers.
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

#ifndef MODEL_WX_IMAGE_FILE_H
#define MODEL_WX_IMAGE_FILE_H

#include "VideoFile.h"

namespace model {

class WximageFile
    :   public VideoFile
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// \return true if the file given must be opened with this class
    /// \param path file to be checked
    /// /note only file types with alpha support in wxImage are handled via this class
    static bool canRead(wxFileName path);

    WximageFile();

    WximageFile(const wxFileName& path);

    virtual WximageFile* clone() const override;

    virtual ~WximageFile();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    pts getLength() const override;
    void moveTo(pts position) override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(const VideoCompositionParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    bool canBeOpened() override;

    wxSize getSize() override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    WximageFile(const WximageFile& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxImagePtr mInputImage;
    VideoFramePtr mOutputFrame;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void readImage();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const WximageFile& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::WximageFile, 1)
BOOST_CLASS_EXPORT_KEY(model::WximageFile)

#endif
