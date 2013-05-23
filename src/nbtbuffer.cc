/*
 * Copyright (C) 2012 Scott Atkins
 *
 * This file is part of cppNBT
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "cppnbt.h"

#include <cassert>
#include <iostream>
#include <cstring>

namespace nbt
{
    typedef Tag *(NbtBuffer::*NbtMembFn)(); // Again...

    const NbtMembFn NbtBuffer::readerFunctions[] =
    {
            NULL,
            &NbtBuffer::readByte,
            &NbtBuffer::readShort,
            &NbtBuffer::readInt,
            &NbtBuffer::readLong,
            &NbtBuffer::readFloat,
            &NbtBuffer::readDouble,
            &NbtBuffer::readByteArray,
            &NbtBuffer::readString,
            &NbtBuffer::readList,
            &NbtBuffer::readCompound,
            &NbtBuffer::readIntArray,
    };

    NbtBuffer::NbtBuffer()
        : _root(NULL), _buffer(NULL), bufferSize(0), bufferPos(0)
    {

    }

    NbtBuffer::NbtBuffer(uint8_t *compressedBuffer, unsigned int length)
        : _root(NULL), _buffer(NULL), bufferSize(0), bufferPos(0)
    {
        read(compressedBuffer, length);
    }

    NbtBuffer::~NbtBuffer()
    {
        delete _root;
    }

    #define BASE_BUFFER_SIZE 32768
    void NbtBuffer::read(uint8_t *compressedBuffer, unsigned int length)
    {
        static thread_local uint8_t *inflatedBuffer = new uint8_t[BASE_BUFFER_SIZE];
        static thread_local size_t inflatedBufferSize = BASE_BUFFER_SIZE;
        uLongf uncompressedSize = inflatedBufferSize;

        int result = uncompress(inflatedBuffer, &uncompressedSize, compressedBuffer, length);
        while (result == Z_BUF_ERROR)
        {
            inflatedBufferSize *= 2;
            delete[] inflatedBuffer;
            inflatedBuffer = new uint8_t[inflatedBufferSize];
            uncompressedSize = inflatedBufferSize;

            result = uncompress(inflatedBuffer, &uncompressedSize, compressedBuffer, length);
        }
        if (result != Z_STREAM_END && result!= Z_OK)
        {
            return;
        }

        bufferSize = uncompressedSize;

        //setup the buffer stream
        _buffer = inflatedBuffer;

        if (_root)
        {
            delete _root;
            _root = NULL;
        }

        //read root
        _root = readTag();

        //all done reading, clean-up
        _buffer = NULL;

    }

    Tag *NbtBuffer::getRoot() const
    {
        return _root;
    }

    void NbtBuffer::setRoot(const Tag &r)
    {
        delete _root;
        _root = r.clone();
    }

    void NbtBuffer::readBuffer(void* buf, unsigned len)
    {
        assert(bufferPos + len <= bufferSize);
        memcpy(buf, _buffer + bufferPos, len);
        bufferPos += len;
    }

    Tag *NbtBuffer::readTag()
    {
        uint8_t type = 0;

        readBuffer(&type, 1);
        if (type == TAG_END)
            return new TagEnd();

        Tag *nameTag = readString();
        TagString *nameTagStr = dynamic_cast<TagString *>(nameTag);

        NbtMembFn reader = readerFunctions[type];
        if (reader == nullptr)
        {
            delete nameTag;
            return nullptr;
        }
        Tag *res = (this->*reader)();
        res->setName(nameTagStr->getValue());

        delete nameTag;
        return res;
    }


    Tag *NbtBuffer::readByte()
    {
        uint8_t byte;
        readBuffer(&byte, 1);

        return new TagByte("", byte);
    }

    Tag *NbtBuffer::readShort()
    {
        int16_t val;
        readBuffer(&val, 2);

        val = be16toh(val);

        return new TagShort("", val);
    }

    Tag *NbtBuffer::readInt()
    {
        int32_t val;
        readBuffer(&val, 4);

        val = be32toh(val);

        return new TagInt("", val);
    }

    Tag *NbtBuffer::readLong()
    {
        int64_t val;
        readBuffer(&val, 8);

        val = be64toh(val);

        return new TagLong("", val);
    }

    Tag *NbtBuffer::readFloat()
    {
        union endianConvert
        {
            int i;
            float f;
        } val;

        readBuffer(&val, 4);

        val.i = be32toh(val.i);

        return new TagFloat("", val.f);
    }

    Tag *NbtBuffer::readDouble()
    {
        union endianConvert
        {
            long l;
            double d;
        } val;
        readBuffer(&val, 8);

        val.l = be64toh(val.l);

        return new TagDouble("", val.d);
    }

    Tag *NbtBuffer::readByteArray()
    {
        int32_t len;
        readBuffer(&len, 4);

        len = be32toh(len);

        unsigned char *byteArray = new unsigned char[len];

        readBuffer(byteArray, len);

        return new TagByteArray("", byteArray, len);
    }

    Tag *NbtBuffer::readIntArray()
    {
        int32_t len;
        readBuffer(&len, 4);

        len = be32toh(len);

        int* intArray = new int[len];

        readBuffer(intArray, len * sizeof(int));

        for (int i = 0; i < len; ++i)
        {
            intArray[i] = be32toh(intArray[i]);
        }

        return new TagIntArray("", intArray, len);
    }

    Tag *NbtBuffer::readString()
    {
        int16_t len;
        readBuffer(&len, 2);

        len = be16toh(len);

        std::string str;
        str.reserve(len);

        str.append(reinterpret_cast<char*>(_buffer + bufferPos),len);
        bufferPos += len;

        return new TagString("", str);
    }

    Tag *NbtBuffer::readList()
    {
        int8_t childType;
        int32_t len;

        readBuffer(&childType, 1);
        readBuffer(&len, 4);

        TagList *ret = new TagList(childType, "");

        len = be32toh(len);

        NbtMembFn reader = readerFunctions[childType];
        for (int i = 0; i < len; ++i)
        {
            Tag *child = (this->*reader)();
            ret->append(child);
        }

        return ret;
    }

    Tag *NbtBuffer::readCompound()
    {
        Tag *child = NULL;
        TagCompound *ret = new TagCompound("");

        while (((child = readTag()) != NULL))
        {
            if (child->getType() == TAG_END)
            {
                delete child;
                break;
            }

            ret->insert(child);
        }

        return ret;
    }
}
