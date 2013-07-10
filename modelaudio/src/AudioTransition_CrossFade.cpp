#include "AudioTransition_CrossFade.h"

#include "AudioChunk.h"
#include "AudioClip.h"
#include "AudioCompositionParameters.h"
#include "Convert.h"
#include "TransitionFactory.h"
#include "UtilLog.h"

namespace model { namespace audio { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CrossFade::CrossFade()
    :	AudioTransition()
    ,   mCache()
{
    VAR_DEBUG(this);
}

CrossFade::CrossFade(const CrossFade& other)
    :   AudioTransition(other)
    ,   mCache()
{
    VAR_DEBUG(*this);
}

CrossFade* CrossFade::clone() const
{
    return new CrossFade(static_cast<const CrossFade&>(*this));
}

CrossFade::~CrossFade()
{
    VAR_DEBUG(this);
}

struct Cache
{
    Cache()
        :   mLastRenderedFrame(0)
        ,   mLeftChunk()
        ,   mRightChunk()
    {}
    int mLastRenderedFrame;
    AudioChunkPtr mLeftChunk;
    AudioChunkPtr mRightChunk;

    friend std::ostream& operator<<( std::ostream& os, const Cache& obj )
    {
        os << obj.mLastRenderedFrame << '|';
        if (obj.mLeftChunk)
        {
            os << *(obj.mLeftChunk);
        }
        else
        {
            os << '0';
        }
        if (obj.mRightChunk)
        {
            os << *(obj.mRightChunk);
        }
        else
        {
            os << '0';
        }
        return os;
    }
};

//////////////////////////////////////////////////////////////////////////
// AUDIOTRANSITION
//////////////////////////////////////////////////////////////////////////

void CrossFade::reset()
{
    mCache.reset();
}

AudioChunkPtr CrossFade::getAudio(samplecount position, IClipPtr leftClip, IClipPtr rightClip, const AudioCompositionParameters& parameters)
{
    if (!mCache)
    {
        mCache = boost::make_shared<Cache>();
        mCache->mLastRenderedFrame = parameters.ptsToSamples(position);
    }

    if (!mCache->mLeftChunk || mCache->mLeftChunk->getUnreadSampleCount() == 0)
    {
        mCache->mLeftChunk = leftClip ? boost::static_pointer_cast<AudioClip>(leftClip)->getNextAudio(parameters)  : AudioChunkPtr();
    }
    if (!mCache->mRightChunk || mCache->mRightChunk->getUnreadSampleCount() == 0)
    {
        mCache->mRightChunk = rightClip ? boost::static_pointer_cast<AudioClip>(rightClip)->getNextAudio(parameters) : AudioChunkPtr();
    }

    samplecount nSamples = std::numeric_limits<samplecount>::max();
    if (mCache->mLeftChunk)
    {
        ASSERT_MORE_THAN_ZERO(mCache->mLeftChunk->getUnreadSampleCount());
        nSamples = std::min(nSamples, mCache->mLeftChunk->getUnreadSampleCount());
    }
    if (mCache->mRightChunk)
    {
        ASSERT_MORE_THAN_ZERO(mCache->mRightChunk->getUnreadSampleCount());
        nSamples = std::min(nSamples, mCache->mRightChunk->getUnreadSampleCount());
    }

    ASSERT_MORE_THAN_ZERO(nSamples);
    ASSERT_ZERO(nSamples % parameters.getNrChannels()); // Ensure that the data for all speakers is there... If this assert ever fails: maybe there's file formats in which the data for a frame is 'truncated'?
    sample* p = 0;
    model::AudioChunkPtr result = boost::make_shared<model::AudioChunk>(p, parameters.getNrChannels(), nSamples, 0);

    samplecount total = getTotalSamples(parameters);
    sample* dataLeft = mCache->mLeftChunk ? mCache->mLeftChunk->getUnreadSamples() : 0;
    sample* dataRight = mCache->mRightChunk ? mCache->mRightChunk->getUnreadSamples() : 0;
    sample* dataResult = result->getBuffer();
    for (samplecount i = 0; i < nSamples; ++i)
    {
        float factorLeft = ((float)total - (float)position) / (float)total;
        float factorRight = (float)position / (float)total;
        sample left = dataLeft ? dataLeft[i] : 0;
        sample right = dataRight ? dataRight[i] : 0;
        dataResult[i] = left * factorLeft + right * factorRight;
        position++;
    }

    if (mCache->mLeftChunk) { mCache->mLeftChunk->read(nSamples); }
    if (mCache->mRightChunk) { mCache->mRightChunk->read(nSamples); }

    ASSERT(mCache);
    VAR_DEBUG(*mCache)(*result);

    return result;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const CrossFade& obj )
{
    os << static_cast<const AudioTransition&>(obj) << '|';
    if (obj.mCache)
    {
        os << *(obj.mCache);
    }
    else
    {
        os <<'0';
    }
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void CrossFade::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<AudioTransition>(*this);
}
template void CrossFade::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void CrossFade::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace