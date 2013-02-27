#ifndef VIDEO_FRAME_COMPOSITION_PARAMETERS_H
#define VIDEO_FRAME_COMPOSITION_PARAMETERS_H

#include "IVideo.h"

namespace model {

class VideoCompositionParameters
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoCompositionParameters();
    VideoCompositionParameters(const VideoCompositionParameters& other);

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator==( const VideoCompositionParameters& other );

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    VideoCompositionParameters& setBoundingBox(wxSize boundingBox);
    wxSize getBoundingBox() const;

    VideoCompositionParameters& setDrawBoundingBox(bool draw);
    bool getDrawBoundingBox() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxSize mBoundingBox;
    bool mDrawBoundingBox;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoCompositionParameters& obj );

};

} // namespace

#endif // VIDEO_FRAME_COMPOSITION_PARAMETERS_H