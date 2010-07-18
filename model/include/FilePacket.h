#ifndef FIFO_PACKET_H
#define FIFO_PACKET_H

#include "UtilFifo.h"

struct AVPacket;

class Packet : boost::noncopyable
{
public:
	Packet(AVPacket* packet);
	virtual ~Packet();
	AVPacket* getPacket();
    int getSizeInBytes();
private:
	AVPacket* mPacket;
};

typedef boost::shared_ptr<Packet> PacketPtr;
typedef Fifo<PacketPtr> FifoPacket;

#endif // FIFO_PACKET_H