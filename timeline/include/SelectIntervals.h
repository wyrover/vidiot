#ifndef SELECT_INTERVALS_H
#define SELECT_INTERVALS_H

#include "GuiPtr.h"
#include <wx/dc.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>

class Interval
{
public:
    explicit Interval(long _begin)
        :   begin(_begin)
        ,   end(-1)
    {
    }
    /** -1 indicates not initialized yet. */
    long begin;
    /** -1 indicates not initialized yet. */
    long end;
};

class SelectIntervals
{
public:

    SelectIntervals();
    void init(GuiTimeLinePtr timeline);
    virtual ~SelectIntervals();

    void draw(wxDC& dc);

private:
    GuiTimeLinePtr mTimeline;

    typedef std::list<Interval> mIntervals;


public:
    typedef std::list<long> MarkerPositions;
    /** List of couples indicating begin and end of marked area */
    MarkerPositions mMarkerPositions;
    void addBeginMarker();
    void addEndMarker();
   
    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive> 
    void serialize(Archive & ar, const unsigned int version);
};

BOOST_CLASS_VERSION(SelectIntervals, 1)
BOOST_CLASS_EXPORT(SelectIntervals)

#endif // SELECT_INTERVALS_H
