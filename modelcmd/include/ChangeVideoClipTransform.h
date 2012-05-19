#ifndef CHANGE_VIDEO_CLIP_TRANSFORM_H
#define CHANGE_VIDEO_CLIP_TRANSFORM_H

#include <wx/gdicmn.h>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include "RootCommand.h"
#include "Enums.h"

namespace model {
class VideoClip;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;

/// Abstract base for all modifications on the timeline/sequence.
class ChangeVideoClipTransform
    :   public ::command::RootCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit ChangeVideoClipTransform(model::VideoClipPtr videoclip);
    virtual ~ChangeVideoClipTransform();

    void setScaling(VideoScaling scaling, boost::optional<int> scalingdigits = boost::none);
    void setAlignment(VideoAlignment alignment);
    void setPosition(wxPoint position);

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::VideoClipPtr getVideoClip() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mInitialized;

    model::VideoClipPtr mVideoClip;

    VideoScaling mOldScaling;
    int moldScalingDigits;

    VideoAlignment mOldAlignment;
    wxPoint mOldPosition;

    boost::optional<VideoScaling> mNewScaling;
    boost::optional<int> mNewScalingDigits;
    boost::optional<VideoAlignment> mNewAlignment;
    boost::optional<wxPoint> mNewPosition;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const ChangeVideoClipTransform& obj );
};

} // namespace

#endif // CHANGE_VIDEO_CLIP_TRANSFORM_H