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

#ifndef MODEL_TRACK_H
#define MODEL_TRACK_H

#include "IControl.h"
#include "UtilLog.h"
#include "UtilSelf.h"
#include "UtilRTTI.h"

namespace model {

class Track
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public IControl
    ,   public Self<Track>
    ,   public IRTTI
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Track();

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    ///
    /// \return a clone of the track, that can be used for rendering
    /// The track is cloned just before the rendering is started. That ensures
    /// that the sequence can be edited further, while the 'previous version' is
    /// being rendered. This clone does not need to copy all attributes, since the
    /// only action done with it is rendering. In fact, only the minimal cloning
    /// should be done, for performance reasons.
    ///
    /// \see make_cloned
    Track(const Track& other);

    virtual Track* clone() const;

    virtual void onCloned();

    virtual ~Track();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const override;
    virtual void moveTo(pts position) override;
    virtual wxString getDescription() const override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // CLIPS
    //////////////////////////////////////////////////////////////////////////

    bool isEmpty() const;

    virtual void removeClips(IClips clips);
    virtual void addClips(IClips clips, IClipPtr position = IClipPtr());

    const IClips& getClips();

    /// Find the clip which provides the frame at the given pts.
    /// If pts is 'on a cut' then the clip AFTER the cut is returned.
    /// If there is no clip at this pts then an empty Ptr is returned.
    IClipPtr getClip(pts position) const;

    ///// Find the clips which provide the frames for the pts region [start, end).
    ///// Also partially overlapping clips are returned.
    ///// If there are no clips in the region then an empty list is returned.
    //IClips getClips(pts start, pts end);

    /// \return the index'th clip in the list of clips
    IClipPtr getClipByIndex(int index);

    /// /return size of area to the left of clip that is empty
    pts getLeftEmptyArea(IClipPtr clip);

    /// /return size of area to the right of clip that is empty
    pts getRightEmptyArea(IClipPtr clip);

    //////////////////////////////////////////////////////////////////////////
    // GET & SET
    //////////////////////////////////////////////////////////////////////////

    int getHeight() const;          ///< \return height of this track in the timeline view
    void setHeight(int height);     ///< \param new height of this track in the timeline view

    int getIndex() const;           ///< \return index of this track in the list of video/audio tracks
    void setIndex(int index);       ///< \param index new index of this track

    std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>()); ///\return list of all cuts in the track

    bool isEmptyAt( pts position ) const; ///< \return true if the track holds only emptyness at the given position

protected:

    //////////////////////////////////////////////////////////////////////////
    // ITERATION
    //////////////////////////////////////////////////////////////////////////

    bool iterate_atEnd();
    IClipPtr iterate_get();
    void iterate_next();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IClips mClips;

    IClips::const_iterator mItClips;

    int mHeight;    ///< Height of this track when viewed in a timeline
    int mIndex;     ///< Index in the list of video/audio tracks

    /// Some variables are only required for better performance.
    /// These contain 'duplicate/redundant' information.
    struct Cache
    {
        Cache()
            : length(0)
        {
        }
        pts length;
    };
    Cache mCache;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Updates the clips after insertion/removal etc.
    /// - Updates the pts'es for all clips in this track
    /// - Updates the clip's track pointer to this track
    void updateClips();

    /// Updates the cached track length. Sends an event if the length has changed.
    /// May cause new accesses to the model. Thus, ensure that the model contents
    /// (and any changes thereof) have been signaled to the view classes.
    void updateLength();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Track& obj );

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
BOOST_CLASS_VERSION(model::Track, 1)
BOOST_CLASS_EXPORT_KEY(model::Track)

#endif // MODEL_TRACK_H