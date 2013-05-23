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
#include <cstring>

namespace nbt
{
    TagIntArray::TagIntArray(const std::string &name, int* value, size_t size)
        : Tag(name)
        , _values(value)
        , _size(size)
    {
    }

    TagIntArray::TagIntArray(const TagIntArray &t)
        : Tag(t.getName())
        , _values(0)
        , _size(t._size)
    {
        _values = new int[_size];
        memcpy(_values, t._values, _size);
    }

    TagIntArray::~TagIntArray()
    {
        delete _values;
    }

    int* TagIntArray::getValues() const
    {
        return _values;
    }

    void TagIntArray::setValues(int *values, unsigned int newSize)
    {
        _values = values;
        _size = newSize;
    }

    unsigned int TagIntArray::getSize() const
    {
        return _size;
    }

    uint8_t TagIntArray::getType() const
    {
        return TAG_BYTE_ARRAY;
    }


    ByteArray TagIntArray::toByteArray() const
    {
        ByteArray ret = Tag::toByteArray();

        ret.push_back(htobe32(_size));

        for (size_t k = 0; k < _size; ++k)
        {
            ret.push_back(htobe32(_values[k]));
        }

        return ret;
    }


    std::string TagIntArray::toString() const
    {
        std::stringstream ret;
        
        ret << "TAG_Int_Array";
        
        if (!_name.empty())
            ret << "(\"" << _name << "\")";
        
        ret << ": ";

        ret << _size << " ints";

        //comment this out if you wanna see the raw data
        //for (size_t i = 0; i < _value.size(); ++i)
        //{
        //    ret << "0x";
        //    ret.width(2);
        //    ret.fill('0');
        //    ret << std::hex << (int)_value[i] << " ";
        //}

        return ret.str();
    }


    Tag* TagIntArray::clone() const
    {
        return new TagIntArray(*this);
    }
}
