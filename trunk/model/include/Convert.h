#ifndef CONVERT_H
#define CONVERT_H

namespace model {

class Convert
{
public:

    static int timeToPts(int time);         ///< @param time time duration in milliseconds 
    static int ptsToTime(int pts);          ///< @return time duration in milliseconds
    static int ptsToMicroseconds(int pts);  ///< @return time duration in microseconds.
    static int microsecondsToPts(int us);   ///< @param us time duration in microseconds 
};

} // namespace

#endif // CONVERT_H
