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
// LOGGING
//////////////////////////////////////////////////////////////////////////

//static
wxString Avcodec::getDefaultLogLevel()
{
    return (*sLogLevels.begin()).first;
}

//static
std::list<wxString> Avcodec::getLogLevels()
{
    std::list<wxString> result;
    BOOST_FOREACH( auto value, sLogLevels )
    {
        result.push_back(value.first);
    }
    return result;
}

void Avcodec::configureLog()
{
    BOOST_FOREACH( auto value, sLogLevels )
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

std::string Avcodec::getErrorMessage(int errorcode)
{
    static const int errbuf_size = 256;
    char errbuf[errbuf_size];
    int errorDecodeResult = av_strerror(errorcode, errbuf, errbuf_size);
    VAR_ERROR(errorDecodeResult);
    return str( boost::format("'%1%'") % errbuf );
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

    std::ostringstream o;
    if (ptr)
    {
        o   << "["
            << (*(AVClass**)ptr)->item_name(ptr)
            << ";"
            << (*(AVClass**)ptr)->class_name
            << "]";
    }
    else
    {
        o << "";
    }
    Log().get("AVCODEC") << o.str() << " [" << sFixedBuffer << "]";
}