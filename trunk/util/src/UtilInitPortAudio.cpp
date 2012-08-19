#include "UtilInitPortAudio.h"

#include <pa_debugprint.h>
#include <portaudio.h>

#include "UtilLog.h"

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void PortAudio::init()
{
    PaUtil_SetDebugPrintFunction(log);
    PaError err = Pa_Initialize();
    ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));
}

void PortAudio::exit()
{
    PaError err = Pa_Terminate();
    ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void PortAudio::log(const char * log)
{
    std::string s(log);
    std::string::iterator it = s.end() - 1;
    if (*it == '\n')
    {
         s.erase(it);
    }
    LOG_DEBUG << s;
}