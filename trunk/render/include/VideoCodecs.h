#ifndef RENDER_VIDEO_CODECS_H
#define RENDER_VIDEO_CODECS_H

namespace model { namespace render {

typedef std::map<CodecID,VideoCodecPtr> VideoCodecMap;

class VideoCodecs
{
public:
    static void initialize();
    static void add(wxString name, VideoCodec codec);

    static boost::bimap<int,wxString> mapToName;

    static VideoCodecPtr getDefault();
    static std::list<CodecID> all();

    /// \return 0 if a codec with the given id was not found
    /// This method finds the given codec in the registered list of codecs
    static VideoCodecPtr find(CodecID id);
private:
    static VideoCodecMap sVideoCodecs;
};

}} // namespace

#endif // RENDER_VIDEO_CODECS_H