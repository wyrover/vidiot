// Copyright 2013 Eric Raijmakers.
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

#ifndef MODEL_EMPTY_CLIP_H
#define MODEL_EMPTY_CLIP_H

#include "Clip.h"
#include "IAudio.h"
#include "IVideo.h"

namespace model {

/// Class can not be used to simultaneously deliver audio AND video.
/// Make separate clips if both empty audio and video are required.
class EmptyClip
    :   public Clip
    ,   public IAudio
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyClip();

    /// Create a new empty clip.
    /// \param length size of the clip
    EmptyClip(pts length);

    virtual EmptyClip* clone() const override;

    virtual ~EmptyClip();

    /// Make an empty clip that has the same values as the original clip.
    /// When a clip is replaced with 'emptyness' the values for *adjustBegin
    /// and *adjustEnd of the emptyness should equal the original clip. That
    /// is required in case transitions are adjacent.
    ///
    /// This method ensures that, when replacing clips, the resulting empty space
    /// has (in its Render object) enough space to accommodate any adjacent transitions.
    /// \return empty clip that is a 'replica' of original , with the same offset/length etc, but only for non-transitions. For transitions empty space with the same length as original is returned.
    /// \param original clip to be cloned
    static EmptyClipPtr replace( IClipPtr original );

    /// Make an empty clip that has the same length as the original list of clips.
    /// \post resulting clip getMaxAdjustBegin() equals clips.front()->getMaxAdjustBegin()
    /// \post resulting clip getMinAdjustEnd() equals clips.back()->getMinAdjustEnd()
    /// \pre All clips are part of the same track
    static EmptyClipPtr replace(model::IClips clips);

    //////////////////////////////////////////////////////////////////////////
    // CLIP
    //////////////////////////////////////////////////////////////////////////

    pts getLength() const override;
    void moveTo(pts position) override;

    void setLink(IClipPtr link) override;

    pts getMinAdjustBegin() const override;
    pts getMaxAdjustBegin() const override;
    void adjustBegin(pts adjustment) override;

    pts getMinAdjustEnd() const override;
    pts getMaxAdjustEnd() const override;
    void adjustEnd(pts adjustment) override;

    std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>()) const override;

    std::ostream& dump(std::ostream& os) const override;
    char* getType() const override;

    FilePtr getFile() const override;

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(const AudioCompositionParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(const VideoCompositionParameters& parameters) override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    explicit EmptyClip(const EmptyClip& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mLength; ///< Length of empty clip
    pts mProgress; ///< Current render position in pts units (delivered video frames/audio chunks count)

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const EmptyClip& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::EmptyClip, 1)
BOOST_CLASS_EXPORT_KEY(model::EmptyClip)

#endif // MODEL_EMPTY_CLIP_H