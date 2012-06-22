#ifndef MODEL_RENDER_H
#define MODEL_RENDER_H

#include <wx/filename.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>

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
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// The initialization of all codec related administration is done via this class,
    /// to avoid exposing the VideoCodec/AudioCodec header files (and thus, the avcodec
    /// dependency) to other parts (particularly, to avoid integer types troubles).
    static void initialize();

    Render(model::SequencePtr sequence);

	virtual ~Render();

    void generate();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    OutputFormatPtr getOutputFormat();
    void setOutputFormat(OutputFormatPtr format);

private:

    model::WeakSequencPtr mSequence;
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