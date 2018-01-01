#include <iostream>
#include <fstream>
#include <array>
#include <cstdint>

constexpr std::size_t FRAME_SIZE = 256;
constexpr std::size_t PAGE_SIZE  = FRAME_SIZE; 
constexpr std::size_t PAGE_TABLE_SIZE = 256;
constexpr std::size_t TLB_SIZE        = 16; 

class entry
{
	std::uint16_t _virtual;
	std::uint16_t _physical;
};

typedef std::array<std::uint8_t, FRAME_SIZE> frame; 

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Error: argc not 3\n";
		return 1;
	}
	std::ifstream main_memory_src(argv[1], std::ios::binary);
	std::array<frame, 256> main_memory;
	if (not main_memory_src.is_open())
	{
		std::cerr << "File not opened\n";
		return 2;
	}

	int pos = main_memory_src.tellg();
	for (auto &frame: main_memory)
	{
		main_memory_src.read(frame.data(), pos + FRAME_SIZE);
		pos += FRAME_SIZE;
	}

	std::array<entry, PAGE_TABLE_SIZE> page_table;
	std::array<entry, TLB_SIZE> TLB;
	
	return 0;
}
