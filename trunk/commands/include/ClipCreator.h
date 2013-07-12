#ifndef CLIP_CREATOR_H
#define CLIP_CREATOR_H

namespace command {

/// Factory class for creating new clips from a file on disk
class ClipCreator
{
public:

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    static std::pair<model::IClipPtr, model::IClipPtr> makeClips(model::FilePtr file);

};

} // namespace

#endif // CLIP_CREATOR_H