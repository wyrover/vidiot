#ifndef RENDER_OUTPUT_FORMAT_H
#define RENDER_OUTPUT_FORMAT_H

extern "C" {
#pragma warning(disable:4244)
#include <libavcodec/avcodec.h>
#pragma warning(default:4244)
}

#include "UtilCloneable.h"

namespace model { namespace render {

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
    //:   public ICloneable // todo make the clonable template globally used
    :   public Cloneable<OutputFormat>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    OutputFormat();
    explicit OutputFormat(wxString name, wxString longname, std::list<wxString> extensions, CodecID defaultaudiocodec, CodecID defaultvideocodec);
    OutputFormat(const OutputFormat& other);
    virtual ~OutputFormat();

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator== (const OutputFormat& other) const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    OutputFormat& setName(wxString name);
    wxString getName() const;
    wxString getLongName() const;
    std::list<wxString> getExtensions() const;
    CodecID getDefaultAudioCodec() const;
    CodecID getDefaultVideoCodec() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mName;
    wxString mLongName;
    std::list<wxString> mExtensions;

    CodecID mDefaultAudioCodec;
    CodecID mDefaultVideoCodec;

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