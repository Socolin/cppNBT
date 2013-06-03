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
    TagCompound::TagCompound(const std::string &name)
        : Tag(name)
    {
    }


    TagCompound::TagCompound(const TagCompound &t) : Tag(t.getName())
    {
        for (auto tagItr : t._value)
        {
            insert(tagItr.second->clone());
        }
   }


    TagCompound::~TagCompound()
    {
        for (auto tagItr : _value)
            delete tagItr.second;
        _value.clear();
    }


    const std::map<std::string, Tag *>& TagCompound::getValue() const
    {
        return _value;
    }


    void TagCompound::setValue(const std::list<Tag *> value)
    {
        std::list<Tag *>::const_iterator i;
        for (i = value.begin(); i != value.end(); ++i)
            insert(**i);
    }

    void TagCompound::insert(const Tag &tag)
    {
        _value[tag.getName()] = tag.clone();
    }

    void TagCompound::insert(Tag *tag)
    {
        _value[tag->getName()] = tag;
    }

    void TagCompound::remove(const std::string &name)
    {
        _value.erase(name);
    }

    std::vector<std::string> TagCompound::getKeys() const
    {
        std::vector<std::string> ret;

        for (auto tagItr : _value)
            ret.push_back(tagItr.first);

        return ret;
    }

    std::vector<Tag *> TagCompound::getValues() const
    {
        std::vector<Tag *> ret;

        for (auto tagItr : _value)
            ret.push_back(tagItr.second);

        return ret;
    }


    Tag *TagCompound::getValueAt(const std::string &key) const
    {
        auto tagItr = _value.find(key);
        return tagItr->second;
    }


    uint8_t TagCompound::getType() const
    {
        return TAG_COMPOUND;
    }


    ByteArray TagCompound::toByteArray() const
    {
        ByteArray ret = Tag::toByteArray();

        for (auto tagItr : _value)
        {
            ByteArray tmp = tagItr.second->toByteArray();

            for (size_t i = 0; i < tmp.size(); ++i)
                ret.push_back(tmp[i]);
        }

        ret.push_back((char)TAG_END);

        return ret;
    }


    std::string TagCompound::toString() const
    {
        std::stringstream ret;
        ret << "TAG_Compound";

        if (!_name.empty())
            ret << "(\"" << _name << "\")";

        ret << ": " << _value.size() << " entries" << std::endl
            << "{" << std::endl;

        for (auto tagItr : _value)
        {
            ret << "  " 
                << string_replace(tagItr.second->toString(), "\n", "\n  ")
                << std::endl;
        }

        ret << "}";

        return ret.str();
    }

    Tag *TagCompound::clone() const
    {
        TagCompound *ret = new TagCompound(_name);

        for (auto tagItr : _value)
        {
            ret->insert(tagItr.second->clone());
        }

        return ret;
    }

    int TagCompound::getInt(const std::string& key) const
    {
        nbt::TagInt* tag = getValueAt<nbt::TagInt>(key);
        if (tag)
        {
            return tag->getValue();
        }
        return 0;
    }

    short TagCompound::getShort(const std::string& key) const
    {
        nbt::TagShort* tag = getValueAt<nbt::TagShort>(key);
        if (tag)
        {
            return tag->getValue();
        }
        return 0;
    }

    char TagCompound::getByte(const std::string& key) const
    {
        nbt::TagByte* tag = getValueAt<nbt::TagByte>(key);
        if (tag)
        {
            return tag->getValue() != 0;
        }
        return 0;
    }

    bool TagCompound::getBool(const std::string& key) const
    {
        nbt::TagByte* tag = getValueAt<nbt::TagByte>(key);
        if (tag)
        {
            return tag->getValue();
        }
        return false;
    }

    float TagCompound::getFloat(const std::string& key) const
    {
        nbt::TagFloat* tag = getValueAt<nbt::TagFloat>(key);
        if (tag)
        {
            return tag->getValue();
        }
        return 0;
    }

    double TagCompound::getDouble(const std::string& key) const
    {
        nbt::TagDouble* tag = getValueAt<nbt::TagDouble>(key);
        if (tag)
        {
            return tag->getValue();
        }
        return 0;
    }

    const std::string& TagCompound::getString(const std::string& key) const
    {
        nbt::TagString* tag = getValueAt<nbt::TagString>(key);
        if (tag)
        {
            return tag->getValue();
        }
        return "";
    }

    bool TagCompound::hasKey(const std::string& key) const
    {
        return _value.find(key) != _value.end();
    }
}
