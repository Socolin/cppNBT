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

    void NbtBuffer::read(uint8_t *compressedBuffer, unsigned int length)
    {
        uLongf BUFF_SIZE = length * 128;
        uint8_t *inflatedBuffer = new uint8_t[BUFF_SIZE];
        memset(inflatedBuffer, 0, BUFF_SIZE);

        int result = uncompress(inflatedBuffer, &BUFF_SIZE, compressedBuffer, length);
        while (result == Z_BUF_ERROR)
        {
            delete[] inflatedBuffer;
            BUFF_SIZE = BUFF_SIZE * 2;
            inflatedBuffer = new uint8_t[BUFF_SIZE];
            result = uncompress(inflatedBuffer, &BUFF_SIZE, compressedBuffer, length);

        }
        if (result != Z_STREAM_END && result!= Z_OK)
        {
            delete[] inflatedBuffer;
            return;
        }

        bufferSize = BUFF_SIZE;

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

        delete[] inflatedBuffer;
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
        if (bufferPos + len <= bufferSize)
        {
            memcpy(buf, _buffer + bufferPos, len);
            bufferPos += len;

        }
        else
        {
            std::cerr << "ERREUR !" << std::endl;
        }
    }

    NbtMembFn NbtBuffer::getReader(uint8_t type)
    {
        switch (type)
        {
            case TAG_BYTE:       return &NbtBuffer::readByte;
            case TAG_SHORT:      return &NbtBuffer::readShort;
            case TAG_INT:        return &NbtBuffer::readInt;
            case TAG_LONG:       return &NbtBuffer::readLong;
            case TAG_FLOAT:      return &NbtBuffer::readFloat;
            case TAG_DOUBLE:     return &NbtBuffer::readDouble;
            case TAG_BYTE_ARRAY: return &NbtBuffer::readByteArray;
            case TAG_STRING:     return &NbtBuffer::readString;
            case TAG_LIST:       return &NbtBuffer::readList;
            case TAG_COMPOUND:   return &NbtBuffer::readCompound;
            case TAG_INT_ARRAY:  return &NbtBuffer::readIntArray;
            default: return NULL; // Also bogus
        }
    }

    Tag *NbtBuffer::readTag()
    {
        uint8_t type = 0;

        readBuffer(&type, 1);
        if (type == TAG_END)
            return new TagEnd();

        Tag *nameTag = readString();
        TagString *nameTagStr = dynamic_cast<TagString *>(nameTag);

        NbtMembFn reader = getReader(type);
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

        if (!is_big_endian())
            flipBytes<int16_t>(val);

        return new TagShort("", val);
    }

    Tag *NbtBuffer::readInt()
    {
        int32_t val;
        readBuffer(&val, 4);

        if (!is_big_endian())
            flipBytes<int32_t>(val);

        return new TagInt("", val);
    }

    Tag *NbtBuffer::readLong()
    {
        int64_t val;
        readBuffer(&val, 8);

        if (!is_big_endian())
            flipBytes<int64_t>(val);

        return new TagLong("", val);
    }

    Tag *NbtBuffer::readFloat()
    {
        float val;
        readBuffer(&val, 4);

        if (!is_big_endian())
            flipBytes<float>(val);

        return new TagFloat("", val);
    }

    Tag *NbtBuffer::readDouble()
    {
        double val;
        readBuffer(&val, 8);

        if (!is_big_endian())
            flipBytes<double>(val);

        return new TagDouble("", val);
    }

    Tag *NbtBuffer::readByteArray()
    {
        int32_t len;
        readBuffer(&len, 4);

        if (!is_big_endian())
            flipBytes<int32_t>(len);

        unsigned char *byteArray = new unsigned char[len];

        readBuffer(byteArray, len);

        return new TagByteArray("", byteArray, len);
    }

    Tag *NbtBuffer::readIntArray()
    {
        int32_t len;
        readBuffer(&len, 4);

        if (!is_big_endian())
            flipBytes<int32_t>(len);

        IntArray ia;
        for (int i = 0; i < len; ++i)
        {
            int32_t val;
            readBuffer(&val, 4);

            ia.push_back(val);
        }

        return new TagIntArray("", ia);
    }

    Tag *NbtBuffer::readString()
    {
        int16_t len;
        readBuffer(&len, 2);

        if (!is_big_endian())
            flipBytes<int16_t>(len);

        std::string str;
        for (int i = 0; i < len; ++i)
        {
            // TODO: Read blocks
            uint8_t ch;
            readBuffer(&ch, 1);
            str.push_back(ch);
        }

        return new TagString("", str);
    }

    Tag *NbtBuffer::readList()
    {
        int8_t childType;
        int32_t len;

        readBuffer(&childType, 1);
        readBuffer(&len, 4);

        TagList *ret = new TagList(childType, "");

        if (!is_big_endian())
            flipBytes<int32_t>(len);

        NbtMembFn reader = getReader(childType);
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
