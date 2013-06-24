#include "HelperTimelineTrim.h"

#include "HelperTimeline.h"
#include "HelperWindow.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

namespace test {

void TrimLeft(model::IClipPtr clip, pixel length, bool shift, bool endtrim)
{
    wxPoint from = LeftCenter(clip);
    wxPoint to = from;
    to.x += length;
    Trim(from,to,shift,endtrim);
}

void TrimRight(model::IClipPtr clip, pixel length, bool shift, bool endtrim)
{
    wxPoint from = RightCenter(clip);
    wxPoint to = from;
    to.x += length;
    Trim(from,to,shift,endtrim);
}

void BeginTrim(wxPoint from, bool shift)
{
    Move(from);
    waitForIdle();
    if (shift)
    {
        ShiftDown();
        waitForIdle();
    }
    LeftDown();
    waitForIdle();
}

void EndTrim(bool shift)
{
    LeftUp();
    waitForIdle();
    if (shift)
    {
        ShiftUp();
        waitForIdle();
    }
}

void Trim(wxPoint from, wxPoint to, bool shift, bool endtrim)
{
    BeginTrim(from,shift);
    Move(to);
    waitForIdle();
    if (endtrim)
    {
        EndTrim(shift);
    }
}

void ShiftTrim(wxPoint from, wxPoint to)
{
    Trim(from,to,true,true);
}

} // namespace