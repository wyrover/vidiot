// Copyright 2014 Eric Raijmakers.
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

#ifndef LINK_REPLACEMENT_MAP_H
#define LINK_REPLACEMENT_MAP_H

namespace gui { namespace timeline { namespace command {

/// Holds administration of clips and the clips that these clips
/// have been replaced with during edits.
class LinkReplacementMap
{
public:

    typedef std::map<model::IClipPtr, model::IClips> ReplacementMap;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit LinkReplacementMap();
    virtual ~LinkReplacementMap();

    //////////////////////////////////////////////////////////////////////////
    // INTERFACE
    //////////////////////////////////////////////////////////////////////////


    // Add a new mapping
    // \param original clip that is being replaces
    /// \param replacmenents clips that are replacing the clip
    void add(const model::IClipPtr& original, const model::IClips& replacements);

    model::IClips danglingLinks();

    /// Repair 'linking of clips' information after replacing several clips.
    /// For each
    /// Clip c1 - linked to clip l1 - which is replaced with r1,r2,...,rn
    /// and
    /// Clip l1 - linked to clip c1 - which is replaced with s1,s2,...,sn
    /// New links are made as follows:
    /// r1<->s1, r2<->s2, ..., rn<->sn
    ///
    /// If l1 was not replaced yet, it is being replaced with a new clip r1 which is a 100% clone of l1.
    /// If one of the two replacements list is shorter than the other, the extra clips in the other list
    /// are linked to nothing.
    /// \param conversionmap mapping for 'maintaining links' that will be updated when splitting
    void replace();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

        /// Holds all replacements that were done for this command.
    /// Used to keep updated clip link information correct.
    /// Note that a clip may be mapped onto an empty list. That indicates
    /// the clip has been removed, without replacing it with new clips.
    ReplacementMap mReplacements;

    /// \see mReplacements
    /// \see expandReplacements
    /// In this map, all entries are 'expanded' until no more replaced clips
    /// are part of any replacement anymore.
    ReplacementMap mExpandedReplacements;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Update mExpandedReplacements
    /// Expand/Recurse, to ensure that the algorithm also works when clips (during the edit)
    /// are replaced with other clips that, in turn, are replaced with yet other clips.
    ///
    /// This works as follows:
    /// (note: 'left' are all clips that are mapped onto other clips,
    ///        'right' are all clips that are a replacement clip).
    ///
    /// As long as there are 'right' clips that are also present 'left',
    /// replace these 'right clips' with their replacements (thus the result
    /// of using them as 'left' keys) in the mappings.
    void expand();

    /// Helper method for expandReplacements. This method expands all the clips in
    /// original to contain all 'final' clips, after consecutively having applied
    /// a list of clip replacements.
    /// \param original list of clips to be expanded
    /// \return list containing all fully expanded clips
    model::IClips expand(const model::IClips& original);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const LinkReplacementMap& obj);
};

}}} // namespace

#endif
