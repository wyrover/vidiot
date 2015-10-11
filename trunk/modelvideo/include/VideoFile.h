// Copyright 2013-2015 Eric Raijmakers.
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

#include "File.h"
#include "IVideo.h"

namespace model {

class VideoFile
    :   public File
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoFile();

    VideoFile(const wxFileName& path);

    virtual VideoFile* clone() const override;

    virtual ~VideoFile();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    void moveTo(pts position) override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(const VideoCompositionParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    virtual wxSize getSize();

    FrameRate getFrameRate();

    uint64_t getVideoPacketPts();

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoFile(const VideoFile& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mDecodingVideo;
    pts mPosition;                  ///< Current position of this clip (set via 'moveTo' or changed via 'getNext')
    VideoFramePtr mDeliveredFrame;  ///< The most recently returned frame in getNext. The pts value stored in this frame is the pts in the input time base (thus, the timebase of the file, and not the timebase of the project).
    SwsContext* mSwsContext;        ///< Software scaling context
    pts mVideoPacketPts;            ///< (input) pts value for most recent packet fed into the decoder.

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void startDecodingVideo(const VideoCompositionParameters& parameters);
    void stopDecodingVideo();

    //////////////////////////////////////////////////////////////////////////
    // FROM FILE
    //////////////////////////////////////////////////////////////////////////

    bool useStream(const AVMediaType& type) const override;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const VideoFile& obj);
    void saveDecodedFrame(AVCodecContext* codec, AVFrame* frame, const wxSize& size, int frameFinished);
    void saveScaledFrame(AVCodecContext* codec, const wxSize& size, VideoFramePtr frame);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::VideoFile, 1)
BOOST_CLASS_EXPORT_KEY(model::VideoFile)
