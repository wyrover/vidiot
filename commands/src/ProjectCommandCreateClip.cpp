#include "ProjectCommandCreateClip.h"
#include "UtilLog.h"
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>


ProjectCommandCreateClip::ProjectCommandCreateClip()
:   ProjectCommand()
{
    VAR_INFO(this)(0);
    mCommandName = _("Create clip");
}

ProjectCommandCreateClip::~ProjectCommandCreateClip()
{
}

bool ProjectCommandCreateClip::Do()
{
    VAR_INFO(this);
    //if (!mNewClips)
    //{
    //    // Create new clips
    //    // Store old clips
    //}
//                mChildren.push_back(mParent->addClip());
//                model::ProjectViewPtr ProjectAsset::addClip(model::ProjectViewPtr slicedAsset)
    return true;
}

bool ProjectCommandCreateClip::Undo()
{
    VAR_INFO(this);
    //mParent->removeChild(mNewClips);
    return true;
}
