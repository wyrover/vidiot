#ifndef DELETE_SELECTED_CLIPS_H
#define DELETE_SELECTED_CLIPS_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class DeleteSelectedClips
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit DeleteSelectedClips(model::SequencePtr sequence);

    ~DeleteSelectedClips();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize() override;
    void doExtraBefore() override;
    void undoExtraAfter() override;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const DeleteSelectedClips& obj );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mShift;             ///< True if the empty area left over after deletion should be trimmed
    model::IClips mSelected; ///< List of clips that was selected before the edit operation was done

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Store the list of currently selected clips
    void storeSelection();

    /// Restore the list of selected clips as was saved before applying the edit
    void restoreSelection();

};

}}} // namespace

#endif // DELETE_SELECTED_CLIPS_H