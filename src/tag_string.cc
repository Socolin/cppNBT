/*
 * Copyright (C) 2011 Lukas Niederbremer
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

namespace nbt
{
    TagString::TagString(const std::string &name, const std::string &value) 
        : Tag(name), _value(value)
    {
    }


    TagString::TagString(const TagString &t)
        : Tag(t.getName()), _value(t.getValue())
    {
    }


    std::string TagString::getValue() const
    {
        return _value;
    }


    void TagString::setValue(const std::string &value)
    {
        _value = value;
    }


    uint8_t TagString::getType() const
    {
        return TAG_STRING;
    }


    ByteArray TagString::toByteArray() const
    {
        ByteArray ret = Tag::toByteArray();

        int16_t len = _value.length();
        uint8_t *split = reinterpret_cast<uint8_t *>(&len);

        for (int i = 1; i >= 0; --i)
            ret.push_back(split[i]);

        for (int i = 0; i < len; ++i)
            ret.push_back(_value[i]);

        return ret;
    }


    std::string TagString::toString() const
    {
        std::stringstream ret;

        ret << "TAG_String";

        if (!_name.empty())
            ret << "(\"" << _name << "\")";
        
        ret << ": " << _value;

        return ret.str();
    }

    Tag *TagString::clone() const
    {
        return new TagString(_name, _value);
    }
}
