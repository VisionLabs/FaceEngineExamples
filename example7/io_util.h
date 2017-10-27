//
// Created by s.cherepanov on 2/20/17.
//

#ifndef FACEENGINE_IO_UTIL_H
#define FACEENGINE_IO_UTIL_H

#include <fsdk/FaceEngine.h>

#include <vector>
#include <fstream>

struct VectorArchive: fsdk::IArchive
{
	std::vector<uint8_t>& dataOut;
	size_t index = 0;

	bool write(const void* data, size_t size) override {
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		dataOut.insert(dataOut.end(), p, p+size);
		return true;
	}

	bool read(void* data, size_t size) override {
		assert(size <= dataOut.size()-index);
		memcpy(data, (void*)&dataOut[0+index], size);
		index += size;
		return true;
	}

	void setSizeHint(size_t /*hint*/) override {}

	VectorArchive(std::vector<uint8_t>& inout):
		dataOut(inout)
	{}
};

inline std::vector<uint8_t> readFile(const std::string& path) {
	std::ifstream file(path, std::ios::binary);
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<uint8_t> out(size);
	file.read((char*)&out[0], size);
	return out;
}

inline bool writeFile(const std::string& path, const std::vector<uint8_t>& data) {
	std::ofstream file(path, std::ios::binary);
	return !!file.write((char*)&data[0], data.size());
}

#endif //FACEENGINE_IO_UTIL_H
