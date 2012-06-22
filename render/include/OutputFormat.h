#ifndef RENDER_OUTPUT_FORMAT_H
#define RENDER_OUTPUT_FORMAT_H

#include <wx/string.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilCloneable.h"

namespace model { namespace render {

class AudioCodec;
typedef boost::shared_ptr<AudioCodec> AudioCodecPtr;
class VideoCodec;
typedef boost::shared_ptr<VideoCodec> VideoCodecPtr;
class OutputFormat;
typedef boost::shared_ptr<OutputFormat> OutputFormatPtr;

// todo idea: make ICloneable template:
template <typename DERIVED>
class Cloneable //: public ICloneable
{
public:
    virtual DERIVED* clone() const
    {
        return new DERIVED(static_cast<const DERIVED&>(*this));
    }
    virtual ~Cloneable() {}
};

class OutputFormat
    //:   public ICloneable
    :   public Cloneable<OutputFormat>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    OutputFormat();
    explicit OutputFormat(wxString name, wxString longname, wxString extension, AudioCodecPtr audiocodec, VideoCodecPtr videocodec);
    OutputFormat(const OutputFormat& other);
    virtual ~OutputFormat();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    OutputFormat& setName(wxString name);
    wxString getName() const;
    wxString getLongName() const;
    wxString getExtension() const;
    AudioCodecPtr getAudioCodec() const;
    VideoCodecPtr getVideoCodec() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mName;
    wxString mLongName;
    wxString mExtension;

    AudioCodecPtr mAudioCodec;
    VideoCodecPtr mVideoCodec;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const OutputFormat& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

}} // namespace

#endif // RENDER_OUTPUT_FORMAT_H