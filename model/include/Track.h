#ifndef MODEL_TRACK_H
#define MODEL_TRACK_H

#include <wx/event.h>
#include <boost/serialization/split_member.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "IControl.h"

namespace model {

class Track 
    :   public IControl
    ,   public wxEvtHandler
    ,   public boost::enable_shared_from_this<Track>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

	Track();
	virtual ~Track();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual boost::int64_t getNumberOfFrames();
    virtual void moveTo(boost::int64_t position);

    //////////////////////////////////////////////////////////////////////////
    // HANDLING CLIPS
    //////////////////////////////////////////////////////////////////////////

    void removeClips(Clips clips);
    /** Add clips AFTER 'position' */
    void addClips(Clips clips, ClipPtr position);

    const Clips& getClips();

protected:

    Clips mClips;
    Clips::const_iterator mItClips;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const;
    template<class Archive>
    void load(Archive & ar, const unsigned int version);
    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

} // namespace

BOOST_CLASS_VERSION(model::Track, 1)
BOOST_CLASS_EXPORT(model::Track)

#endif // MODEL_TRACK_H
