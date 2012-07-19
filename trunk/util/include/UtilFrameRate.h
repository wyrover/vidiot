#ifndef FRAMERATE_H
#define FRAMERATE_H

typedef boost::rational<int> FrameRate;

namespace framerate {

const FrameRate s24p = FrameRate(1000, 24 * 1001);
const FrameRate s25p = FrameRate(   1, 25);
const FrameRate s30p = FrameRate(1000, 30 * 1001);

std::vector<FrameRate> getSupported();

wxString toString(FrameRate framerate);

FrameRate fromString(wxString framerate);

} // namespace

#endif // FRAMERATE_H