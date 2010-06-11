#include "FifoPacket.h"

extern "C" {
#include "stdint.h"
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
