#ifndef RENDER_Audio_CODECS_H
#define RENDER_Audio_CODECS_H

extern "C" {
#pragma warning(disable:4244)
#include <libavcodec/avcodec.h>
#pragma warning(default:4244)
}

#include <wx/string.h>
#include <boost/bimap.hpp>

namespace model { namespace render {

typedef std::map<CodecID,AudioCodecPtr> AudioCodecMap;

class AudioCodecs
{
public:
    static void initialize();
    static void add(wxString name, AudioCodec codec);

    static boost::bimap<int,wxString> mapToName;

    static AudioCodecPtr getDefault();

    /// \return 0 if a codec with the given id was not found
    /// This method finds the given codec in the registered list of codecs
    static AudioCodecPtr find(CodecID id);
private:
    static AudioCodecMap sAudioCodecs;
};

}} // namespace

#endif // RENDER_Audio_CODECS_H