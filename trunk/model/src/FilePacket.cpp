#include "FilePacket.h"

extern "C" {
#ifdef _MSC_VER
#include "stdint.h"
#endif // _MSC_VER
#include <avcodec.h>
};

Packet::Packet(AVPacket* packet)
:	mPacket(0)
{
    mPacket = new AVPacket(*packet);
}

Packet::~Packet()
{
	if (mPacket->data)
	{
		av_free_packet(mPacket);
	}
    delete mPacket;
}

AVPacket* Packet::getPacket()
{
	return mPacket;
}

int Packet::getSizeInBytes()
{
    return mPacket->size;
}
