#include <string.h>
#include "databuffer.h"


using namespace std;
namespace mDNS {


DataBuffer::DataBuffer()
{
    m_capacity = 256;
    m_data = (uint8_t *)malloc(m_capacity);
    m_size = 0;
    m_offset = 0;
}


DataBuffer::DataBuffer(size_t capacity)
{
    m_capacity = capacity;
    m_data = (uint8_t *)malloc(m_capacity);
    m_size = 0;
    m_offset = 0;
}


DataBuffer::DataBuffer(const void *data, size_t size)
{
    m_capacity = size;
    m_data = (uint8_t *)malloc(m_capacity);
    m_size = size;
    m_offset = 0;

    memcpy(m_data, data, size);
}


DataBuffer::DataBuffer(const DataBuffer &rhs)
{
    m_capacity = rhs.m_capacity;
    m_size = rhs.m_size;
    m_offset = rhs.m_offset;
    m_data = (uint8_t *)malloc(m_capacity);
    memcpy(m_data, rhs.m_data, m_size);
}


DataBuffer::~DataBuffer()
{
    if (m_data != NULL)
	free(m_data);
}


void DataBuffer::increaseIfNeeded(size_t size)
{
    size_t cap = m_capacity;


    while ((m_size + size) > cap) {
	cap += 256;
    }

    if (cap > m_capacity) {
	m_data = (uint8_t *)realloc(m_data, cap);
	m_capacity = cap;
    }

    m_size += size;
}


size_t DataBuffer::getCapacity() const
{
    return m_capacity;
}


size_t DataBuffer::getSize() const
{
    return m_size;
}


off_t DataBuffer::getOffset() const
{
    return m_offset;
}


size_t DataBuffer::getAvailable() const
{
    return (m_size - m_offset);
}


void DataBuffer::setSize(size_t size)
{
    if (size > m_size)
	increaseIfNeeded(size - m_size);
    else
	m_size = size;
}


void DataBuffer::seek(off_t offset, int whence)
{
    if (whence == SEEK_SET)
	m_offset = offset;
    else if (whence == SEEK_CUR)
	m_offset += offset;
    else if (whence == SEEK_END)
	m_offset = (m_size + offset);

    if (m_offset < 0)
	m_offset = 0;

    if (m_offset > (off_t)m_size)
	increaseIfNeeded(m_offset - m_size);
}


void DataBuffer::putInt8(uint8_t value)
{
    increaseIfNeeded(sizeof(int8_t));

    m_data[m_offset++] = value;
}


void DataBuffer::putInt16(uint16_t value)
{
    increaseIfNeeded(sizeof(int16_t));

    memcpy(m_data + m_offset, &value, sizeof(int16_t));
    m_offset += sizeof(int16_t);
}


void DataBuffer::putInt32(uint32_t value)
{
    increaseIfNeeded(sizeof(int32_t));

    memcpy(m_data + m_offset, &value, sizeof(int32_t));
    m_offset += sizeof(int32_t);
}


void DataBuffer::putBytes(const void *data, size_t size)
{
    increaseIfNeeded(size);

    memcpy(m_data + m_offset, data, size);
    m_offset += size;
}


void DataBuffer::putData(const DataBuffer &data)
{
    putBytes(data.peekBytes(data.getAvailable()), data.getAvailable());
}


uint8_t DataBuffer::readInt8()
{
    uint8_t	value = 0;


    if ((m_offset + sizeof(uint8_t)) <= m_size) {
	value = m_data[m_offset];
	m_offset += sizeof(uint8_t);
    }

    return value;
}


uint16_t DataBuffer::readInt16()
{
    uint16_t	value = 0;


    if ((m_offset + sizeof(uint16_t)) <= m_size) {
	memcpy(&value, m_data + m_offset, sizeof(uint16_t));
	m_offset += sizeof(uint16_t);
    }

    return value;
}


uint32_t DataBuffer::readInt32()
{
    uint32_t	value = 0;


    if ((m_offset + sizeof(uint32_t)) <= m_size) {
	memcpy(&value, m_data + m_offset, sizeof(uint32_t));
	m_offset += sizeof(uint32_t);
    }

    return value;
}


DataBuffer DataBuffer::readData(size_t size)
{
    if ((m_offset + size) <= m_size) {
	m_offset += size;
	return DataBuffer(m_data + m_offset - size, size);
    }

    return DataBuffer();
}


const void *DataBuffer::readBytes(size_t size)
{
    if ((m_offset + size) <= m_size) {
	m_offset += size;
	return (m_data + m_offset - size);
    }

    return NULL;
}


uint8_t DataBuffer::peekInt8(off_t offset, int whence) const
{
    uint8_t	value = 0;


    if (whence == SEEK_CUR)
	offset += m_offset;
    else if (whence == SEEK_END)
	offset += m_size;

    if (offset >= 0 && (offset + sizeof(uint8_t)) <= m_size) {
	value = m_data[offset];
    }

    return value;
}


uint16_t DataBuffer::peekInt16(off_t offset, int whence) const
{
    uint16_t	value = 0;


    if (whence == SEEK_CUR)
	offset += m_offset;
    else if (whence == SEEK_END)
	offset += m_size;

    if (offset >= 0 && (offset + sizeof(uint16_t)) <= m_size) {
	memcpy(&value, m_data + offset, sizeof(uint16_t));
    }

    return value;
}


uint32_t DataBuffer::peekInt32(off_t offset, int whence) const
{
    uint32_t	value = 0;


    if (whence == SEEK_CUR)
	offset += m_offset;
    else if (whence == SEEK_END)
	offset += m_size;

    if (offset >= 0 && (offset + sizeof(uint32_t)) <= m_size) {
	memcpy(&value, m_data + offset, sizeof(uint32_t));
    }

    return value;
}


DataBuffer DataBuffer::peekData(size_t size, off_t offset, int whence) const
{
    if (whence == SEEK_CUR)
	offset += m_offset;
    else if (whence == SEEK_END)
	offset += m_size;

    if (offset >= 0 && (offset + size) <= m_size) {
	return DataBuffer(m_data + offset, size);
    }

    return DataBuffer();
}


const void *DataBuffer::peekBytes(size_t size, off_t offset, int whence) const
{
    if (whence == SEEK_CUR)
	offset += m_offset;
    else if (whence == SEEK_END)
	offset += m_size;

    if (offset >= 0 && (offset + size) <= m_size) {
	return (m_data + offset);
    }

    return NULL;
}


const void *DataBuffer::rawBytes() const
{
    return m_data;
}


DataBuffer &DataBuffer::operator=(const DataBuffer &rhs)
{
    if (m_data != NULL)
	free(m_data);

    m_capacity = rhs.m_capacity;
    m_size = rhs.m_size;
    m_offset = rhs.m_offset;
    m_data = (uint8_t *)malloc(m_capacity);
    memcpy(m_data, rhs.m_data, m_size);

    return *this;
}


} /* namespace mDNS */

