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

#ifndef HELPER_RENDER_H
#define HELPER_RENDER_H

#include "Test.h"

namespace test
{

  std::pair< RandomTempDirPtr, wxFileName > OpenRenderDialogAndSetFilename();
  std::pair< RandomTempDirPtr, wxFileName > RenderTimeline(int lengthInS = 1);
  void PlaybackRenderedTimeline(const wxFileName& path, pixel start = 2, milliseconds t = 600);
  void RenderAndPlaybackCurrentTimeline(int renderedlengthInS = 2, pixel playbackStart = 2, milliseconds playbackLength = 600);

}

#endif