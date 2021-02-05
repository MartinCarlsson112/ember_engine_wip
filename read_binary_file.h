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
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
}