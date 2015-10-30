// Copyright 2013-2015 Eric Raijmakers.
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

#ifndef PRECOMPILED_H
#define PRECOMPILED_H

// Required for ensuring that boost asserts are handled via UtilLog
#define BOOST_ENABLE_ASSERT_HANDLER

// Mechanism:
// If more than one unity file depends on a ffmpeg/boost/wx header - place here
// If all files depending on the header are part of the same 'unity' (thus, same folder)
// then add to these (cpp) files themselves. Note: if it is a header file that includes
// the ffmpeg/boost/wx header, be aware that the header file is also included elsewhere.

extern "C" {
#pragma warning(disable:4244)
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#pragma warning(default:4244)
};

#ifdef _MSC_VER
#define __STDC_LIMIT_MACROS
#include "inttypes.h"
#else
#include <inttypes.h>
#endif

#include <algorithm>
#include <atomic>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/exception/all.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/limits.hpp>
#include <boost/make_shared.hpp>
#include <boost/mpl/list.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/rational.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp> // This must be included AFTER including all archives!
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/units/detail/utility.hpp>
#include <boost/utility.hpp>

#include <wx/app.h>
#include <wx/aui/aui.h>
#include <wx/bookctrl.h>
#include <wx/brush.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/clipbrd.h>
#include <wx/clrpicker.h>
#include <wx/cmdproc.h>
#include <wx/colour.h>
#include <wx/combobox.h>
#include <wx/control.h>
#include <wx/dataobj.h>
#include <wx/dataview.h>
#include <wx/datetime.h>
#include <wx/dc.h>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcgraph.h>
#include <wx/dcmemory.h>
#include <wx/debug.h>
#include <wx/defs.h>
#include <wx/dialog.h>
#include <wx/dir.h>
#include <wx/dnd.h>
#include <wx/docview.h>
#include <wx/event.h>
#include <wx/evtloop.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/font.h>
#include <wx/frame.h>
#include <wx/fswatcher.h>
#include <wx/gauge.h>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/graphics.h>
#include <wx/headercol.h>
#include <wx/html/htmlwin.h>
#include <wx/headerctrl.h>
#include <wx/iconbndl.h>
#include <wx/intl.h>
#include <wx/listbox.h>
#include <wx/menu.h>
#include <wx/mousestate.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/pen.h>
#include <wx/progdlg.h>
#include <wx/propdlg.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/regex.h>
#include <wx/region.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/utils.h>
#include <wx/valnum.h>
#include <wx/window.h>
#include <wx/wizard.h>

#include "Pointers.h"

#endif // PRECOMPILED_H
