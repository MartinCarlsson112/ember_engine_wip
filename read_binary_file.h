#pragma once
#include <fstream>
#include <vector>

inline std::vector<char> read_binary_file(const std::string& filename) 
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		//todo: log this failure
		return std::vector<char>();
	}
	else
	{
		size_t file_size = (size_t)file.tellg();
		std::vector<char> buffer(file_size);
		file.seekg(0);
		file.read(buffer.data(), file_size);
		file.close();
		return buffer;
	}
}