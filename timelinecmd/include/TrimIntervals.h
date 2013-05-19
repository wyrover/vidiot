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
    /// \param sequence sequence from which regions must be removed
    /// \param remove list of intervals to be removed
    /// \param name name of command (for undo/redo menu entries)
    TrimIntervals(model::SequencePtr sequence, PtsIntervals remove, wxString name);

    ~TrimIntervals();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;

    void doExtraBefore() override;
    void undoExtraAfter() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    PtsIntervals mIntervals;    ///< Intervals as marked in the sequence before the command was created
    PtsIntervals mRemoved;      ///< Intervals to be removed from the sequence (maybe all 'non-marked' areas)

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Show an animated preview of the actual edit operation
    void showAnimation();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const TrimIntervals& obj );
};
}}} // namespace

#endif // TRIM_INTERVALS_H