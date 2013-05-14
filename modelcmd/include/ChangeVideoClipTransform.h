#ifndef CHANGE_VIDEO_CLIP_TRANSFORM_H
#define CHANGE_VIDEO_CLIP_TRANSFORM_H

#include "RootCommand.h"
#include "Enums.h"

namespace model {

class ChangeVideoClipTransform
    :   public ::command::RootCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit ChangeVideoClipTransform(model::VideoClipPtr videoclip);
    virtual ~ChangeVideoClipTransform();

    void setOpacity(int opacity);
    void setScaling(VideoScaling scaling, boost::optional< boost::rational< int > > factor = boost::none);
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

    int mOldOpacity;

    VideoScaling mOldScaling;
    boost::rational< int > mOldScalingFactor;

    VideoAlignment mOldAlignment;
    wxPoint mOldPosition;

    boost::optional<int> mNewOpacity;
    boost::optional<VideoScaling> mNewScaling;
    boost::optional< boost::rational< int > > mNewScalingFactor;
    boost::optional<VideoAlignment> mNewAlignment;
    boost::optional<wxPoint> mNewPosition;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const ChangeVideoClipTransform& obj );
};

} // namespace

#endif // CHANGE_VIDEO_CLIP_TRANSFORM_H