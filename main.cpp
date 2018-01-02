#include <iostream>
#include <fstream>
#include <array>
#include <cstdint>
#include <algorithm>

constexpr std::size_t FRAME_SIZE = 256;
constexpr std::size_t PAGE_SIZE  = FRAME_SIZE;
constexpr std::size_t PAGE_TABLE_SIZE = 256;
constexpr std::size_t TLB_SIZE        = 16;
constexpr std::uint16_t MAX_UINT16    = static_cast<std::uint16_t>(-1);

typedef   char _byte_t;
typedef   std::array<_byte_t, FRAME_SIZE> frame;
typedef   std::uint16_t address_t;

struct entry
{
	std::uint16_t _virtual  = -1;
	std::uint16_t _physical = -1;
	int _access_time_stamp  = -1;
	frame* _phy_memory = nullptr;
	frame& operator()()
	{
		if (_phy_memory)
			return *_phy_memory;
		throw std::runtime_error("Access nullptr in entry");
	}
};

struct swap_table
{
	std::array<frame, 256> _swap;
    swap_table(const char* file_path)
	{
		if (std::ifstream main_memory_src{file_path, std::ios::binary})
		{
			for (auto &f: _swap)
				main_memory_src.read(f.data(), FRAME_SIZE);
		}
		else
			std::cerr << "Error: " << file_path << " not open\n";
	}
	frame& operator[] (int id)
	{
		return _swap[id];
	}
};

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Error: argc not 3\n";
		return 1;
	}
	std::array<frame, 256> main_memory;
	std::array<entry, PAGE_TABLE_SIZE> page_table;
	std::array<entry, TLB_SIZE> TLB;
    swap_table swap(argv[1]);
	int address_size(0), page_fault_count(0), TLB_hit_count(0);
	std::ifstream addr_src{argv[2], std::ios::in};
	addr_src >> address_size;
	for (int time(0); time < address_size; time++)
	{
		address_t address;
		addr_src >> address;
		std::uint8_t page = address >> 8, offset = address & 0x00FF;
		std::uint16_t frame = -1;
		// TLB first
		for (auto iter(TLB.begin()); iter != TLB.end(); ++iter)
		{
			if (iter->_virtual == page) // hit
			{
				iter->_access_time_stamp = time;
				// update page table
				for (auto it(page_table.begin()); it != page_table.end(); ++it)
					if (it->_virtual == page)
						it->_access_time_stamp = time;
				frame = iter->_physical;
				TLB_hit_count++;
				break;
			}
		}

		// not found in TLB... next: page table
		if (frame == MAX_UINT16)
		{
			for (auto iter(page_table.begin()); iter != page_table.end(); ++iter)
			{
				if (iter->_virtual == page) // hit
				{
					iter->_access_time_stamp = time;
					std::sort(TLB.begin(), TLB.end(), [](entry const &A, entry const &B) {
						return A._access_time_stamp < B._access_time_stamp;
					});
					// update TLB
					TLB[0] = *iter;
					frame = iter->_physical;
					break;
				}
			}
		}

		// not found in TLB, page table... **page fault** next: swap space
		if (frame == MAX_UINT16)
		{
			page_fault_count++;
			for (auto iter_frame(page_table.begin()); iter_frame != page_table.end(); ++iter_frame)
			{
				if (iter_frame->_phy_memory == nullptr)
				{
					iter_frame->_phy_memory = &(swap[page]);
					iter_frame->_virtual    = page;
					iter_frame->_physical   = iter_frame - page_table.begin();
					iter_frame->_access_time_stamp = time;
					std::sort(TLB.begin(), TLB.end(), [](entry const &A, entry const &B) {
						return A._access_time_stamp < B._access_time_stamp;
					});
					// update TLB
					TLB[0] = *iter_frame;
					frame  = iter_frame->_physical;
					break;
				}
			}
		}

		std::cout << ((frame << 8) + offset) << " " << static_cast<int>(page_table[frame]()[offset]) << "\n";
	}
	std::cout << "TLB hits: " << TLB_hit_count << "\n";
	std::cout << "Page Faults: " << page_fault_count << "\n";
	return 0;
}
