// Copyright 2013,2014 Eric Raijmakers.
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

#include "OutputFormats.h"

#include "AudioCodecs.h"
#include "OutputFormat.h"

#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "VideoCodecs.h"

namespace model { namespace render {

// static
OutputFormatList OutputFormats::sOutputFormats;

// static
void OutputFormats::add(const OutputFormat& format)
{
    sOutputFormats.push_back(boost::make_shared<OutputFormat>(format));
}

// static
void OutputFormats::initialize()
{
    sOutputFormats.clear();

    //add(OutputFormat().setName("avi"));

    LOG_INFO;
    for (AVOutputFormat* format = av_oformat_next(0); format != 0; format = av_oformat_next(format))
    {
        VAR_INFO(format);
        bool unknowncodec = false;
        AudioCodecPtr audiocodec;
        if (format->audio_codec != CODEC_ID_NONE)
        {
            audiocodec = AudioCodecs::find(format->audio_codec);
            if (!audiocodec) { unknowncodec = true; }
        }
        VideoCodecPtr videocodec;
        if (format->video_codec != CODEC_ID_NONE)
        {
            videocodec = VideoCodecs::find(format->video_codec);
            if (!videocodec) { unknowncodec = true; }
        }
        if ((audiocodec && videocodec) && (!unknowncodec))
        {
            wxStringTokenizer tokenizer(format->extensions, ",");
            std::list<wxString> extensions;
            while ( tokenizer.HasMoreTokens() )
            {
                extensions.push_back(tokenizer.GetNextToken());
            }
            if (!extensions.empty())
            {
                add(OutputFormat(wxString(format->name),wxString(format->long_name),extensions,format->audio_codec,format->video_codec));
            }

        }
    }
}

// static
OutputFormatList OutputFormats::getList()
{
    return make_cloned<OutputFormat>(sOutputFormats);
}

// static
std::list<wxString> OutputFormats::getNames()
{
    std::list<wxString> result;
    for ( OutputFormatPtr format : sOutputFormats )
    {
        result.push_back(format->getLongName());
    }
    return result;
}

// static
OutputFormatPtr OutputFormats::getByName(const wxString& name)
{
    for ( OutputFormatPtr format : sOutputFormats )
    {
        if (name.IsSameAs(format->getLongName()))
        {
            return make_cloned<OutputFormat>(format);
        }
    }
    return OutputFormatPtr();
}

// static
OutputFormatPtr OutputFormats::getByExtension(const wxString& extension)
{
    for ( OutputFormatPtr format : sOutputFormats )
    {
        for ( wxString formatextension : format->getExtensions() )
        {
            if (extension.IsSameAs(formatextension))
            {
                return make_cloned<OutputFormat>(format);
            }
        }
    }
    return OutputFormatPtr();
}

// static
OutputFormatPtr OutputFormats::getDefault()
{
    ASSERT_MORE_THAN_ZERO(sOutputFormats.size());
    return make_cloned<OutputFormat>(*(sOutputFormats.begin()));
}

}} //namespace