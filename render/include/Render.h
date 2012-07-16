#ifndef MODEL_RENDER_H
#define MODEL_RENDER_H

#include <wx/event.h>
#include <wx/filename.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "UtilCloneable.h"
#include "UtilEvent.h"

namespace model {

class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
typedef boost::weak_ptr<Sequence> WeakSequencPtr;

namespace render {

class Render;
typedef boost::shared_ptr<Render> RenderPtr;
class AudioCodec;
typedef boost::shared_ptr<AudioCodec> AudioCodecPtr;
class VideoCodec;
typedef boost::shared_ptr<VideoCodec> VideoCodecPtr;
class OutputFormat;
typedef boost::shared_ptr<OutputFormat> OutputFormatPtr;

class Render
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public ICloneable
    ,   public boost::enable_shared_from_this<Render>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// The initialization of all codec related administration is done via this class,
    /// to avoid exposing the VideoCodec/AudioCodec header files (and thus, the avcodec
    /// dependency) to other parts (particularly, to avoid integer types troubles).
    static void initialize();

    Render();

    Render(const Render& other);

	virtual ~Render();

    virtual Render* clone() const override;

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator== (const Render& other) const;
    bool operator!= (const Render& other) const;
    OutputFormatPtr getOutputFormat() const;
    void setOutputFormat(OutputFormatPtr format);

    VideoCodecPtr getVideoCodec() const;
    void setVideoCodec(VideoCodecPtr codec);

    AudioCodecPtr getAudioCodec() const;
    void setAudioCodec(AudioCodecPtr codec);

    wxFileName getFileName() const;
    void setFileName(wxFileName filename);

    ///\ return true if the given filename can be used for rendering
    bool checkFileName() const;

    /// \return a clone of this object with the filename (last part) removed
    /// This returned object can be used in comparisons, where the file name
    /// part is not relevant for the comparison.
    RenderPtr withFileNameRemoved() const;

    //////////////////////////////////////////////////////////////////////////
    // RENDERING
    //////////////////////////////////////////////////////////////////////////

    void generate(SequencePtr sequence);

    static void schedule(SequencePtr sequence);
    static void scheduleAll();

private:

    wxFileName mFileName;
    OutputFormatPtr mOutputFormat;
    VideoCodecPtr mVideoCodec;
    AudioCodecPtr mAudioCodec;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Render& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

}} // namespace

BOOST_CLASS_VERSION(model::render::Render, 1)

#endif // MODEL_RENDER_H