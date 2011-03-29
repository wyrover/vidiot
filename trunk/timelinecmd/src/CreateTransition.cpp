#include "CreateTransition.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace gui { namespace timeline { namespace command {

CreateTransition::CreateTransition(gui::timeline::Timeline& timeline, model::Clips clips)
:   AClipEdit(timeline)
,   mClips(clips)
{
    VAR_INFO(this)(clips);
    mCommandName = _("Create transition");
}

CreateTransition::~CreateTransition()
{
}

void CreateTransition::initialize()
{
    VAR_INFO(this);

    ReplacementMap linkmapper;

    LOG_DEBUG << "STEP 3: Ensure that links are maintained.";
    replaceLinks(linkmapper);
}

}}} // namespace
