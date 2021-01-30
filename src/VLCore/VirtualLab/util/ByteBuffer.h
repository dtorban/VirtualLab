#ifndef VIRTUALLAB_UTIL_BYTE_BUFFER_H_
#define VIRTUALLAB_UTIL_BYTE_BUFFER_H_

#include <iostream>

namespace vl {

class ByteBufferWriter {
public:
    ByteBufferWriter(int size = 512) : size(size), pos(0) {
        bytes = new unsigned char[size];
    }

    virtual ~ByteBufferWriter() {
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
        int strSize = str.size();
        addData(strSize);
        addData((const unsigned char*)str.c_str(), str.size());
    }

    int getSize() const { return pos; }
    const unsigned char* getBytes() const { return bytes; }

private:
    unsigned char* bytes;
    int size;
    int pos;
};

class ByteBufferReader {
public:
    ByteBufferReader(const unsigned char* bytes) : bytes(bytes), pos(0) {}

    virtual ~ByteBufferReader() {}

    template<typename T>
    void readData(T& val) {
        readData((unsigned char*)&val, sizeof(val));
    }

    void readData(unsigned char* data, int len) {
        memcpy(data, &bytes[pos], len);
        pos += len;
    }

    void readString(std::string& str) {
        int dataSize;
        readData(dataSize);
        unsigned char *buf = new unsigned char[dataSize+1];
        readData(buf, dataSize);
        buf[dataSize] = '\0';
        str = std::string(reinterpret_cast<char*>(buf));
        delete[] buf;
        //addData((const unsigned char*)str.c_str(), str.size());
    }

    int getSize() const { return pos; }
    const unsigned char* getBytes() const { return bytes; }

private:
    const unsigned char* bytes;
    int pos;
};


}

#endif