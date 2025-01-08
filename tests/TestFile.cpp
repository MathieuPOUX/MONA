#include "Mona/Mona.h"
#include "Mona/Disk/FileReader.h"
#include "Mona/Disk/FileWriter.h"
#include "Mona/Threading/Handler.h"

using namespace std;
using namespace Mona;

struct MainHandler : Handler {
	MainHandler() : Handler(_signal) {}
	bool join(uint32_t count) {
		uint32_t done(0);
		while ((done += Handler::flush()) < count) {
			if (!_signal.wait(14000))
				return false;
		};
		return count == (done += Handler::flush(true));
	}

private:
	void flush() {}
	Signal _signal;
};
static ThreadPool	_ThreadPool;


int main() {

	// File

	Exception ex;
	const char* name("temp.mona");

	{
		File file(name, File::MODE_WRITE);
		CHECK(file.write(ex, EXPC("Salut")) && !ex);
		CHECK(file.written() == 5 && file.size(true) == 5);
	}

	{
		CHECK(File(name, File::MODE_APPEND).write(ex, EXPC("Salut")) && !ex);
		File file(name, File::MODE_READ);
		CHECK(file.size() == 10);
		char data[10];
		CHECK(file.read(ex, data, 20) == 10 && file.readen() == 10 && !ex && memcmp(data, EXPC("SalutSalut")) == 0);
		CHECK(!file.write(ex, data, sizeof(data)) && ex && ex.cast<Ex::Permission>());
		ex = nullptr;
	}

	{
		File file(name, File::MODE_WRITE);
		CHECK(file.write(ex, EXPC("Salut")) && !ex);
		CHECK(file.written() == 5 && file.size(true) == 5);
		char data[10];
		CHECK(file.read(ex, data, sizeof(data)) == -1 && ex && ex.cast<Ex::Permission>());
		ex = nullptr;
	}
  
	// FileReader
	{
		MainHandler handler;
		IOFile		io(handler, _ThreadPool);

		const char* name("temp.mona");
		Exception ex;

		FileReader reader(io);
		reader.onError = [](const Exception& ex) {
			FATAL_ERROR("FileReader, ", ex);
		};
		reader.onReaden = [&](Shared<string>& pBuffer, bool end) {
			if (pBuffer->size() > 3) {
				CHECK(pBuffer->size() == 5 && memcmp(pBuffer->data(), EXPC("Salut")) == 0 && end);
				reader.close();
				reader.open(name).read(3);
			} else if (pBuffer->size() == 3) {
				CHECK(memcmp(pBuffer->data(), EXPC("Sal")) == 0 && !end);
				reader.read(3);
			} else {
				CHECK(pBuffer->size() == 2 && memcmp(pBuffer->data(), EXPC("ut")) == 0 && end);
				reader.close();
			}
		};
		reader.open(name).read();
		CHECK(handler.join(3));
	}

	// FileWriter
	{
		MainHandler handler;
		IOFile		io(handler, _ThreadPool);
		const char* name("temp.mona");
		Exception ex;
		Packet salut(EXPC("Salut"));

		FileWriter writer(io);
		writer.onError = [](const Exception& ex) {
			FATAL_ERROR("FileWriter, ", ex);
		};
		bool onFlush = false;
		writer.onFlush = [&onFlush](bool deletion) {
			onFlush = true;
			CHECK(!deletion);
		};
		writer.open(name).write(salut);
		io.join();
		CHECK(onFlush); // onFlush!
		CHECK(writer->written() == 5 && writer->size(true) == 5);
		writer.write(salut);
		io.join();
		CHECK(writer->written() == 10 && writer->size(true) == 10);
		writer.close();
		
		onFlush = false;
		writer.open(name, true).write(salut);
		io.join();
		CHECK(onFlush); // onFlush!
		CHECK(writer->written()==5 && writer->size(true) == 15);
		writer.close();

		CHECK(handler.join(2)); // 2 writing step = 2 onFlush!
		CHECK(FileSystem::Delete(ex, name) && !ex);
	}

	return 0;
}
