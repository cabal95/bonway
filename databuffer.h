#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>


namespace mDNS {

class DataBuffer
{
private:
    uint8_t	*m_data;
    size_t	m_capacity, m_size;
    off_t	m_offset;

protected:
    void increaseIfNeeded(size_t size);

public:
    DataBuffer();
    DataBuffer(size_t capacity);
    DataBuffer(const void *data, size_t size);
    DataBuffer(const DataBuffer &rhs);
    ~DataBuffer();

    size_t getCapacity() const;
    size_t getSize() const;
    off_t getOffset() const;
    size_t getAvailable() const;
    void setSize(size_t size);
    void seek(off_t offset, int whence = SEEK_CUR);

    void putInt8(uint8_t value);
    void putInt16(uint16_t value);
    void putInt32(uint32_t value);
    void putBytes(const void *data, size_t size);
    void putData(const DataBuffer &data);

    uint8_t readInt8();
    uint16_t readInt16();
    uint32_t readInt32();
    DataBuffer readData(size_t size);
    const void *readBytes(size_t size);

    uint8_t peekInt8(off_t offset = 0, int whence = SEEK_CUR) const;
    uint16_t peekInt16(off_t offset = 0, int whence = SEEK_CUR) const;
    uint32_t peekInt32(off_t offset = 0, int whence = SEEK_CUR) const;
    DataBuffer peekData(size_t size, off_t offset = 0, int whence = SEEK_CUR) const;
    const void *peekBytes(size_t size, off_t offset = 0, int whence = SEEK_CUR) const;

    const void *rawBytes() const;

    DataBuffer &operator=(const DataBuffer &rhs);
};

} /* namespace mDNS */

#endif /* __DATABUFFER_H__ */

