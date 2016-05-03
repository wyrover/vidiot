// Copyright 2014-2016 Eric Raijmakers.
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

#include "TimelineDataObject.h"

#include "AClipEdit.h"
#include "IClip.h"
#include "LinkReplacementMap.h"
#include "Properties.h"
#include "StatusBar.h"

namespace gui { namespace timeline {

const wxString TimelineDataObject::sFormat = wxString("application/vidiot/clips");

TimelineDataObject::TimelineDataObject()
    : wxTextDataObject()
    , mFrameRate(0,1)
    , mAudioSampleRate(0)
{
    SetFormat(wxDataFormat(sFormat));
}

TimelineDataObject::TimelineDataObject(model::SequencePtr sequence)
    : wxTextDataObject()
    , mFrameRate(model::Properties::get().getFrameRate())
    , mAudioSampleRate(model::Properties::get().getAudioSampleRate())
{
    SetFormat(wxDataFormat(sFormat));

    cmd::LinkReplacementMap replacementMap;

    auto addDrop = [&replacementMap](cmd::Drops& drops, model::TrackPtr track, model::IClips& clips)
    {
        ASSERT(!clips.empty());
        cmd::Drop d;
        d.position = clips.front()->getLeftPts();
        d.clips = make_cloned(clips);
        ASSERT_EQUALS(clips.size(), d.clips.size());
        model::IClips::iterator itOriginal = clips.begin();
        model::IClips::iterator itReplacement = d.clips.begin();
        while (itOriginal != clips.end())
        {
            ASSERT(itReplacement != d.clips.end());
            model::IClips replacements;
            replacements.emplace_back(*itReplacement);
            replacementMap.add(*itOriginal,replacements);
            ++itOriginal;
            ++itReplacement;
        }
        d.track = track;
        drops.emplace_back(d);
        clips.clear();
    };

    auto getDropsInTrack = [&addDrop](cmd::Drops& drops, model::TrackPtr track)
    {
        model::TrackPtr tempTrack = boost::make_shared<model::Track>();
        tempTrack->setIndex(track->getIndex());

        // Also include all transitions for which all 'involved' clips are selected.
        std::set<model::IClipPtr> transitions;
        for (model::IClipPtr clip : track->getClips())
        {
            model::TransitionPtr transition{ boost::dynamic_pointer_cast<model::Transition>(clip) };
            if (transition &&
                (!transition->getLeft() || transition->getPrev()->getSelected()) &&
                (!transition->getRight() || transition->getNext()->getSelected()))
            {
                transitions.insert(transition);
            }
        }

        model::IClips clips;
        for (model::IClipPtr clip : track->getClips())
        {
            if (clip->getSelected() || transitions.count(clip) != 0)
            {
                clips.emplace_back(clip);
            }
            else if (!clips.empty())
            {
                addDrop(drops, tempTrack, clips);
            }
        }
        if (!clips.empty())
        {
            addDrop(drops, tempTrack, clips);
        }
    };

    for (model::TrackPtr track : sequence->getVideoTracks())
    {
        getDropsInTrack(mDropsVideo, track);
    }
    for (model::TrackPtr track : sequence->getAudioTracks())
    {
        getDropsInTrack(mDropsAudio, track);
    }

    // Restore links between (cloned) clips.
    replacementMap.replace();

    SetText(serialize());
}

TimelineDataObject::~TimelineDataObject()
{
}

//////////////////////////////////////////////////////////////////////////
// FROM wxTextDataObject
//////////////////////////////////////////////////////////////////////////

void TimelineDataObject::SetText(const wxString& text)
{
    wxTextDataObject::SetText(text);
    deserialize(GetText());
}

wxDataFormat TimelineDataObject::GetPreferredFormat(Direction dir) const
{
    return wxDataFormat(sFormat);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool TimelineDataObject::checkIfOkForPasteOrDrop() const
{
	if ((mFrameRate != model::Properties::get().getFrameRate()) ||
		(mAudioSampleRate != model::Properties::get().getAudioSampleRate()))
	{
		VAR_ERROR(mFrameRate)(mAudioSampleRate);
		StatusBar::get().timedInfoText(_("Data in clipboard has incompatible properties."));
		return false;
    }
	if (mDropsVideo.empty() && mDropsAudio.empty())
	{
		StatusBar::get().timedInfoText(_("Nothing to be pasted."));
		return false;
	}
	return true;
}

cmd::Drops TimelineDataObject::getDrops(const model::SequencePtr& sequence, pts sequenceOrigin) const
{
	ASSERT_MORE_THAN_EQUALS_ZERO(sequenceOrigin);

	// Clips are 'copied' using their normal position in the sequence.
	// However, for dropping, the leftmost pasted clip should be dropped at position '0'.
    pts clipsOrigin = std::numeric_limits<pts>::max();
    std::for_each(mDropsVideo.begin(), mDropsVideo.end(), [&clipsOrigin](const cmd::Drop& d) { clipsOrigin = std::min(clipsOrigin, d.position); });
    std::for_each(mDropsAudio.begin(), mDropsAudio.end(), [&clipsOrigin](const cmd::Drop& d) { clipsOrigin = std::min(clipsOrigin, d.position); });

	ASSERT_MORE_THAN_EQUALS_ZERO(clipsOrigin);

	int nVideoTracks = sequence->getVideoTracks().size();
	int nAudioTracks = sequence->getAudioTracks().size();

	cmd::Drops result;
	for (cmd::Drop drop : mDropsVideo)
	{
		if (drop.track->getIndex() >= nVideoTracks)
		{
			StatusBar::get().timedInfoText(_("Cannot paste: not enough video tracks."));
			return cmd::Drops();
		}
		drop.track = sequence->getVideoTrack(drop.track->getIndex());
		drop.position += (-clipsOrigin) + sequenceOrigin;
		result.emplace_back(drop);
	}
	for (cmd::Drop drop : mDropsAudio)
	{
		if (drop.track->getIndex() >= nAudioTracks)
		{
			StatusBar::get().timedInfoText(_("Cannot paste: not enough audio tracks."));
			return cmd::Drops();
		}
		drop.track = sequence->getAudioTrack(drop.track->getIndex());
		drop.position += (-clipsOrigin) + sequenceOrigin;
		result.emplace_back(drop);
	}

	return result;
}

bool TimelineDataObject::storeInClipboard()
{
    if (mDropsVideo.empty() && mDropsAudio.empty())
    {
        StatusBar::get().timedInfoText(_("Nothing to be stored in clipboard."));
    }
    else
    {
        if (wxTheClipboard->Open())
        {
            wxTheClipboard->SetData(this);
            wxTheClipboard->Close();
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

const std::string sXmlName("clips");

void TimelineDataObject::deserialize(wxString from)
{
    std::istringstream store(from.ToStdString());
    try
    {
        boost::archive::xml_iarchive ar(store);
        ar & boost::serialization::make_nvp(sXmlName.c_str(), *this);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

wxString TimelineDataObject::serialize() const
{
    std::ostringstream store;
    try
    {
        boost::archive::xml_oarchive ar(store);
        ar & boost::serialization::make_nvp(sXmlName.c_str(), *this);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
    return store.str();
}


template<class Archive>
void TimelineDataObject::serialize(Archive & ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(mFrameRate);
    ar & BOOST_SERIALIZATION_NVP(mAudioSampleRate);
	ar & BOOST_SERIALIZATION_NVP(mDropsVideo);
	ar & BOOST_SERIALIZATION_NVP(mDropsAudio);
}

template void TimelineDataObject::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TimelineDataObject::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
