#ifndef PRECOMPILED_H
#define PRECOMPILED_H

extern "C" {
#pragma warning(disable:4244)
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#pragma warning(default:4244)
};

#include <algorithm>
#include <boost/archive/archive_exception.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/assert.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>
#include <boost/bind.hpp>
#include <boost/config.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/limits.hpp>
#include <boost/make_shared.hpp>
#include <boost/mpl/list.hpp>
#include <boost/noncopyable.hpp>
#include <boost/rational.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp> // This must be included AFTER including all archives!
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <limits.h>
#include <map>
#include <math.h>
#include <ostream>
#include <set>
#include <sstream>
#include <stdio.h>
#include <string>
#include <utility>
#include <wx/app.h>
#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/bookctrl.h>
#include <wx/brush.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/clipbrd.h>
#include <wx/cmdproc.h>
#include <wx/collpane.h>
#include <wx/colour.h>
#include <wx/confbase.h>
#include <wx/config.h> // This ensures that in other parts of the code only #include "Config.h" is required
#include <wx/control.h>
#include <wx/cursor.h>
#include <wx/dataobj.h>
#include <wx/dataview.h>
#include <wx/datetime.h>
#include <wx/dc.h>
#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/dcgraph.h>
#include <wx/dcmemory.h>
#include <wx/debug.h>
#include <wx/debugrpt.h>
#include <wx/defs.h>
#include <wx/dialog.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/dnd.h>
#include <wx/dnd.h>
#include <wx/docview.h>
#include <wx/event.h>
#include <wx/evtloop.h>
#include <wx/fileconf.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/font.h>
#include <wx/frame.h>
#include <wx/FSWatcher.h>
#include <wx/gauge.h>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/graphics.h>
#include <wx/headercol.h>
#include <wx/headerctrl.h>
#include <wx/intl.h>
#include <wx/listbook.h>
#include <wx/menu.h>
#include <wx/minifram.h>
#include <wx/msgdlg.h>
#include <wx/msgout.h> // For NIY
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/pen.h>
#include <wx/propdlg.h>
#include <wx/propgrid/propgrid.h>
#include <wx/radiobox.h>
#include <wx/region.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>
#include <wx/tglbtn.h>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/tooltip.h>
#include <wx/utils.h>
#include <wx/window.h>
#include <wx/dataview.h>

#include "Pointers.h"

#endif // PRECOMPILED_H