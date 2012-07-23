#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "UtilFrameRate.h"

namespace model {

class Properties
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Properties();
    ~Properties();
    static PropertiesPtr get();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    FrameRate getFrameRate() const;
    wxSize getVideoSize() const;

    int getAudioNumberOfChannels() const; ///< \return number of audio channels, thus the number of independent speakers
    int getAudioFrameRate() const; ///< \return frame rate used for audio

    /// \return clone of the currently set default render
    render::RenderPtr getDefaultRender() const;

    /// Sets the default render to be a clone of the given render
    void setDefaultRender(render::RenderPtr render);

private:

    FrameRate mFrameRate;
    long mVideoWidth;
    long mVideoHeight;
    int mAudioChannels;
    int mAudioFrameRate;
    render::RenderPtr mDefaultRender;

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
BOOST_CLASS_VERSION(model::Properties, 1)

#endif // PROPERTIES_H