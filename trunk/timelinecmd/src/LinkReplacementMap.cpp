// Copyright 2014-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "LinkReplacementMap.h"

#include "UtilLog.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

LinkReplacementMap::LinkReplacementMap()
    :   mReplacements()
    ,   mExpandedReplacements()
{
    VAR_INFO(this);
}

LinkReplacementMap::~LinkReplacementMap()
{
}

//////////////////////////////////////////////////////////////////////////
// INTERFACE
//////////////////////////////////////////////////////////////////////////

void LinkReplacementMap::add(const model::IClipPtr& original, const model::IClips& replacements)
{
    ASSERT_MAP_CONTAINS_NOT((mReplacements), original);
    mReplacements[original] = replacements;
}

model::IClips LinkReplacementMap::danglingLinks()
{
    model::IClips result;
    for (auto link : mReplacements)
    {
        model::IClipPtr original = link.first;
        model::IClipPtr originallink = original->getLink();
        if (originallink && mReplacements.find(originallink) == mReplacements.end())
        {
            result.push_back(originallink);
        }
    }
    return result;
}

// static
void LinkReplacementMap::replace()
{
    LOG_DEBUG;
    expand();
    for (ReplacementMap::value_type link : mExpandedReplacements)
    {
        model::IClipPtr clip1 = link.first;
        model::IClips new1 = link.second;
        model::IClips::iterator it1 = new1.begin();

        model::IClipPtr clip2 = clip1->getLink();
        if (clip2) // The clip doesn't necessarily have a link with another clip
        {
            ASSERT(!clip2->isA<model::EmptyClip>())(clip2); // Linking to an empty clip is not allowed

            // If clip clip1 is replaced with replacement1, then its link link1 MUST also be replaced
            // (since link1 must be replaced with a clip whose link is replacement1). This must be guaranteed
            // by all AClipEdit derived classes (not solved generally in the base class since that would
            // cause lots of redundant replacements).
            ASSERT(mExpandedReplacements.find(clip2) != mExpandedReplacements.end())(clip1)(clip2)(mExpandedReplacements);

            model::IClips new2 = mExpandedReplacements[clip2];
            model::IClips::iterator it2 = new2.begin();

            auto NoLinkingAllowed = [](model::IClipPtr clip)->bool
            {
                return (clip->isA<model::EmptyClip>() || clip->isA<model::Transition>()); // Linking to/from empty clips and transitions is not allowed. Skip these.
            };

            while ( it1 != new1.end() && it2 != new2.end() )
            {
                model::IClipPtr newclip1 = *it1;
                model::IClipPtr newclip2 = *it2;
                if (NoLinkingAllowed(newclip1))
                {
                    ++it1;
                    continue;
                }
                if (NoLinkingAllowed(newclip2))
                {
                    ++it2;
                    continue;
                }
                newclip1->setLink(newclip2);
                newclip2->setLink(newclip1);
                ++it1;
                ++it2;
            }
            // For all remaining clips in both lists: not linked.
            for (; it1 != new1.end(); ++it1)
            {
                (*it1)->setLink(model::IClipPtr());
            }
            for (; it2 != new2.end(); ++it2)
            {
                (*it2)->setLink(model::IClipPtr());
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void LinkReplacementMap::expand()
{
    LOG_DEBUG;
    std::set<model::IClipPtr> allReplacements;
    for ( auto entry : mReplacements )
    {
        allReplacements.insert(entry.second.begin(),entry.second.end());
    }
    for ( auto entry : mReplacements )
    {
        if (allReplacements.find(entry.first) == allReplacements.end())
        {
            // The expansion is done for all non-intermediate clips in the map. Since the replacement
            // map is only required for linking of clips, only the original clips and their replacement
            // are relevant. Intermediate clips are not.
            mExpandedReplacements[entry.first] = expand(entry.second);
        }
    }
}

model::IClips LinkReplacementMap::expand(const model::IClips& original)
{
    model::IClips result;
    for ( model::IClipPtr clip : original )
    {
        if (mReplacements.find(clip) == mReplacements.end())
        {
            // The replacement clip is not replaced
            result.push_back(clip);
        }
        else
        {
            // The replacement clip has been replaced also
            model::IClips replacements = expand(mReplacements[clip]);
            result.insert(result.end(),replacements.begin(),replacements.end());
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const LinkReplacementMap& obj)
{
    os << obj.mReplacements << '|' << obj.mExpandedReplacements;
    return os;
}

}}} // namespace
