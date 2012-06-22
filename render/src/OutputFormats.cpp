#include "OutputFormats.h"

extern "C" {
#pragma warning(disable:4244)
#include <avformat.h>
#pragma warning(default:4244)
};

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "UtilLog.h"
#include "OutputFormat.h"
#include "VideoCodecs.h"
#include "AudioCodecs.h"
#include "UtilLogAvcodec.h"

namespace model { namespace render {

// static
OutputFormatList OutputFormats::sOutputFormats;

// static
void OutputFormats::add(OutputFormat format)
{
    sOutputFormats.push_back(boost::make_shared<OutputFormat>(format));
}

// static
void OutputFormats::initialize()
{
    sOutputFormats.clear();

    //add(OutputFormat().setName("avi"));

    LOG_INFO;
    for (AVOutputFormat* format = av_oformat_next(0); format != 0; format = av_oformat_next(format))
    {
        VAR_INFO(format);
        bool unknowncodec = false;
        AudioCodecPtr audiocodec;
        if (format->audio_codec != CODEC_ID_NONE)
        {
            audiocodec = AudioCodecs::find(format->audio_codec);
            if (!audiocodec) { unknowncodec = true; }
        }
        VideoCodecPtr videocodec;
        if (format->video_codec != CODEC_ID_NONE)
        {
            videocodec = VideoCodecs::find(format->video_codec);
            if (!videocodec) { unknowncodec = true; }
        }
        if ((audiocodec || videocodec) && (!unknowncodec))
        {
            // todo for each extension in list of extensions...
            add(OutputFormat(wxString(format->name),wxString(format->long_name),wxString(format->extensions),audiocodec,videocodec));
        }
    }
}

    // static
OutputFormatList OutputFormats::getList()
{
    return make_cloned<OutputFormat>(sOutputFormats);
}

// static
std::list<wxString> OutputFormats::getNames()
{
    std::list<wxString> result;
    BOOST_FOREACH( OutputFormatPtr format, sOutputFormats )
    {
        result.push_back(format->getLongName());
    }
    return result;
}

// static
OutputFormatPtr OutputFormats::getByName(wxString name)
{
    BOOST_FOREACH( OutputFormatPtr format, sOutputFormats )
    {
        if (name.IsSameAs(format->getLongName()))
        {
            return make_cloned<OutputFormat>(format);
        }
    }
    return OutputFormatPtr();
}

// static
OutputFormatPtr OutputFormats::getDefault()
{
    ASSERT_MORE_THAN_ZERO(sOutputFormats.size());
    return make_cloned<OutputFormat>(*(sOutputFormats.begin()));
}

}} //namespace