#ifndef VIRTUALLAB_UTIL_BYTE_BUFFER_H_
#define VIRTUALLAB_UTIL_BYTE_BUFFER_H_

#include <iostream>

namespace vl {

class ByteBuffer {
public:
    ByteBuffer(int size = 512) : size(size), pos(0) {
        bytes = new unsigned char[size];
    }

    ~ByteBuffer() {
        delete[] bytes;
    }

    template<typename T>
    void addData(const T& val) {
        addData((unsigned char*)&val, sizeof(val));
    }

    void addData(const unsigned char* data, int len) {
        if (pos + len > size) {
            unsigned char* newBytes = new  unsigned char[size*2];
            memcpy(newBytes, bytes, size);
            delete[] bytes;
            bytes = newBytes;
            size *= 2;
        }
        memcpy(&bytes[pos], data, size);
        pos += len;
    }

    void addString(const std::string& str) {
        addData((const unsigned char*)str.c_str(), str.size());
    }

    int getSize() const { return pos; }
    const unsigned char* getBytes() const { return bytes; }

private:
    unsigned char* bytes;
    int size;
    int pos;
};

}

#endif