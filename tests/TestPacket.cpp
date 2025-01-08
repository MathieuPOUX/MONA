#include "Mona/Mona.h"
#include "Mona/Memory/Packet.h"
#include <deque>

using namespace std;
using namespace Mona;

void PacketATA(const Packet& packet, bool buffered) {
	CHECK(memcmp(packet.data(), "ata", 3) == 0 && packet.size() == 3 && packet.buffer().operator bool()==buffered);
}

void PacketDAT(const Packet& packet, bool buffered) {
	CHECK(memcmp(packet.data(), "dat", 3) == 0 && packet.size() == 3 && packet.buffer().operator bool()==buffered);
}

void PacketT(const Packet& packet, bool buffered) {
	CHECK(memcmp(packet.data(), "t", 1) == 0 && packet.size() == 1 && packet.buffer().operator bool()==buffered);
}

int main() {
  
	// Test Unbuffered
	{
		Packet packet;

		CHECK(!packet && !packet.buffer() && !packet.data() && !packet.size());
		CHECK(memcmp(packet.set(EXPC("data")).data(), "data", 4) == 0 && packet.size() == 4 && !packet.buffer());

		PacketATA(Packet(packet,packet.data()+1,packet.size()-1),false);
		
		PacketDAT(Packet(packet,packet.data(),3),false);

		packet += 1;
		CHECK(memcmp(packet.data(), "ata", 3) == 0 && packet.size() == 3 && !packet.buffer());

		packet -= 1;
		CHECK(memcmp(packet.data(), "at", 2) == 0 && packet.size() == 2 && !packet.buffer());

		PacketT(Packet(packet, packet.data() + 1, 1 ), false);

		packet.reset();
		CHECK(!packet.data() && packet.size() == 0 && !packet.buffer());
	}

	// Test Buffered
	{
		Shared<string> pBuffer(SET, EXPC("data"));

		Packet packet;
		CHECK(!packet && !packet.buffer() && !packet.data() && !packet.size() && pBuffer && pBuffer->data() && pBuffer->size());

		CHECK(memcmp(packet.set(pBuffer).data(), "data", 4) == 0 && packet.size() == 4 && packet && packet.buffer());

		PacketATA(Packet(packet,packet.data()+1,packet.size()-1),true);
		
		PacketDAT(Packet(packet,packet.data(),3),true);
		
		packet += 1;
		CHECK(memcmp(packet.data(), "ata", 3) == 0 && packet.size() == 3 && packet.buffer());

		packet -= 1;
		CHECK(memcmp(packet.data(), "at", 2) == 0 && packet.size() == 2 && packet.buffer());

		PacketT(Packet(packet, packet.data() + 1, 1 ), true);
		
		packet.reset();
		CHECK(!packet.data() && packet.size() == 0 && !packet.buffer());
	}

	// Test Bufferize
	{
		deque<Packet> packets;
		Packet packet(EXPC("data"));
		
		// test unbuffered data
		CHECK(!packet.buffer());
		const char* data(packet.data());
		packets.emplace_back(move(packet));
		CHECK(packet.buffer() && &packet.buffer() != &packets.back().buffer() && data != packet.data());

		// test buffered data
		Shared<string> pBuffer(SET, packet.data(), packet.size());
		CHECK(packet.set(pBuffer).buffer());
		data = packet.data();
		packets.emplace_back(move(packet));
		CHECK(packet.buffer() && &packet.buffer() != &packets.back().buffer() && data == packet.data());

		packet = nullptr;
		CHECK(!packet.buffer());
	}


	return 0;
}
