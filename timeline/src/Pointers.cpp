#include "Pointers.h"
#include <wx/image.h>
#include <wx/bitmap.h>
#include "GuiTimeLine.h"
#include "GuiTimeLineClip.h"
#include "cursor_move_cut.xpm"
#include "cursor_trim_begin.xpm"
#include "cursor_trim_end.xpm"

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Pointers::Pointers(GuiTimeLine& timeline)
:   mTimeline(timeline)
{
    wxImage image;
    image = wxBitmap(cursor_move_cut_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorMoveCut = wxCursor(image);

    image = wxBitmap(cursor_trim_begin_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorTrimBegin = wxCursor(image);

    image = wxBitmap(cursor_trim_end_xpm).ConvertToImage();
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
    image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
    mCursorTrimEnd = wxCursor(image);

    mTimeline.SetCursor(mCursorMoveCut); /** todo default cursor */
}

Pointers::~Pointers()
{
}

bool Pointers::isOnBeginOfClip(wxPoint virtualposition)
{
    boost::tuple<GuiTimeLineClipPtr,int> clip = mTimeline.findClip(virtualposition);
    if (!clip.get<0>())
    {
        return false;
    }
    int dist_begin = virtualposition.x - clip.get<1>();
    if ((dist_begin > 1) && (dist_begin < 4))
    {
        mTimeline.SetCursor(mCursorTrimBegin);
    }
    return (dist_begin > 1) && (dist_begin < 4);

}

bool Pointers::isOnEndOfClip(wxPoint virtualposition)
{
    boost::tuple<GuiTimeLineClipPtr,int> clip = mTimeline.findClip(virtualposition);
    if (!clip.get<0>())
    {
        return false;
    }
    int dist_end = clip.get<1>() + clip.get<0>()->getBitmap().GetWidth() - virtualposition.x;
    if ((dist_end > 1) && (dist_end < 4))
    {
        mTimeline.SetCursor(mCursorTrimEnd);
    }
    return (dist_end > 1) && (dist_end < 4);
}

bool Pointers::isBetweenClips(wxPoint virtualposition)
{
    boost::tuple<GuiTimeLineClipPtr,int> clip = mTimeline.findClip(virtualposition);
    if (!clip.get<0>())
    {
        return false;
    }
    int dist_begin = virtualposition.x - clip.get<1>();
    int dist_end = clip.get<1>() + clip.get<0>()->getBitmap().GetWidth() - virtualposition.x;
    if ((dist_begin <= 1) || (dist_end <= 1))
    {
        mTimeline.SetCursor(mCursorMoveCut);
    }
    return (dist_begin <= 1) || (dist_end <= 1);
}
