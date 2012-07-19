#ifndef TRIM_INTERVALS_H
#define TRIM_INTERVALS_H

#include "AClipEdit.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

namespace command {

class TrimIntervals
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Remove marked intervals from timeline
    /// \param intervals list of intervals to be removed
    /// \param deleteMarked true if the marked intervals must be removed, false if all unmarked areas must be removed
    TrimIntervals(model::SequencePtr sequence, PtsIntervals intervals, bool deleteMarked = true);

    ~TrimIntervals();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    void doExtra() override;
    void undoExtra() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    PtsIntervals mIntervals;    ///< Intervals as marked in the sequence before the command was created
    PtsIntervals mRemoved;      ///< Intervals to be removed from the sequence (maybe all 'non-marked' areas)
    bool mDeleteMarked;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Split the tracks at the interval's begin and end positions.
    /// \return lists of adjacent clips between interval begin and end positions (for all intervals)
    std::set< model::IClips > splitTracksAndFindClipsToBeRemoved();

    /// Show an animated preview of the actual edit operation
    void showAnimation();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const TrimIntervals& obj );
};
}}} // namespace

#endif // TRIM_INTERVALS_H