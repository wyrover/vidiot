#include "UtilInitAvcodec.h"

#include "Config.h"
#include "UtilEnum.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"

boost::bimap<int, wxString> Avcodec::mapAvcodecLevels = boost::assign::list_of<boost::bimap<int, wxString>::relation >
            (AV_LOG_QUIET,     _("None"))
            (AV_LOG_FATAL,     _("Fatal"))
            (AV_LOG_ERROR,     _("Error"))
            (AV_LOG_WARNING,   _("Warning"))
            (AV_LOG_INFO,      _("Info"))
            (AV_LOG_VERBOSE,   _("Verbose"));

//////////////////////////////////////////////////////////////////////////
// MEMBERS
//////////////////////////////////////////////////////////////////////////

const int Avcodec::sMaxLogSize = 500;
char* Avcodec::sFixedBuffer = 0;
int Avcodec::sLevel = AV_LOG_FATAL;
wxString Avcodec::sMostRecentLogLine(_("Increase the avcodec logging level in the .ini file to get detailed information."));

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void Avcodec::init()
{
    sFixedBuffer = new char[sMaxLogSize];
    av_register_all();
}

void Avcodec::exit()
{
    delete[] sFixedBuffer;
    sFixedBuffer = 0;
}

//////////////////////////////////////////////////////////////////////////
// LOCKING
//////////////////////////////////////////////////////////////////////////

boost::mutex Avcodec::sMutex;

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

//static
wxString Avcodec::getDefaultLogLevel()
{
    return mapAvcodecLevels.left.find(AV_LOG_ERROR)->second;
}

//static
std::list<wxString> Avcodec::getLogLevels()
{
    std::list<wxString> result;

    BOOST_FOREACH( auto value, mapAvcodecLevels.right )
    {
        result.push_back(value.first);
    }
    return result;
}

void Avcodec::configureLog()
{
    BOOST_FOREACH( auto value, mapAvcodecLevels.right )
    {
        if (value.first.IsSameAs(Config::ReadString(Config::sPathLogLevelAvcodec)))
        {
            sLevel = value.second;
            break;
        }
    }
    av_log_set_level(sLevel); // Only required for default avcodec log method
    av_log_set_callback(Avcodec::log);
}

wxString Avcodec::getMostRecentLogLine()
{
    return sMostRecentLogLine;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Avcodec::log(void *ptr, int level, const char * msg, va_list ap)
{
    if (level > sLevel) return;
    int len = vsnprintf(sFixedBuffer, sMaxLogSize, msg, ap);
    if ( len > 0 && sFixedBuffer[len-1] == '\n' )
    {
        // Strip new line in logged line
        sFixedBuffer[len-1] = '.';
    }

    std::ostringstream osComponent;
    if (ptr)
    {
        osComponent
            << "["
            << (*(AVClass**)ptr)->item_name(ptr)
            << ";"
            << (*(AVClass**)ptr)->class_name
            << "]";
    }

    sMostRecentLogLine = sFixedBuffer;
    Log().get("AVC    ") << osComponent.str() << " [" << sFixedBuffer << "]";
}