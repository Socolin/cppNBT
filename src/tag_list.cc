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
    TagList::TagList(const uint8_t &type,
                     const std::string &name,
                     const std::vector<Tag *> &value)
        : Tag(name)
    {
        _childType = type;

        std::vector<Tag *>::const_iterator i;

        for (i = value.begin(); i != value.end(); ++i)
            append(**i);
    }


    TagList::TagList(const TagList &t) : Tag(t.getName())
    {
        _childType = t.getChildType();

        std::vector<Tag *>::const_iterator i;
        std::vector<Tag *> value = t.getValue();

        for (i = value.begin(); i != value.end(); ++i)
            append(**i);
    }


    TagList::~TagList()
    {
        std::vector<Tag *>::iterator it;

        for (it = _value.begin(); it != _value.end(); ++it)
            delete *it;
    }


    std::vector<Tag *> TagList::getValue() const
    {
        return _value;
    }


    void TagList::setValue(const std::vector<Tag *> &value)
    {
        std::vector<Tag *>::const_iterator i;
        for (i = value.begin(); i != value.end(); ++i)
            append(**i);
    }


    uint8_t TagList::getChildType() const
    {
        return _childType;
    }


    void TagList::setChildType(const uint8_t &value)
    {
        _childType = value;
        clear();
    }


    void TagList::append(const Tag &value)
    {
        if (value.getType() == _childType)
            _value.push_back(value.clone());

    }


    void TagList::append(Tag *value)
    {
        if (value->getType() == _childType)
            _value.push_back(value);

    }


    void TagList::removeFirst()
    {
        if (_value.size() > 0)
        {
            delete *(_value.begin());
            _value.erase(_value.begin());
        }
    }


    void TagList::removeLast()
    {
        if (_value.size() > 0)
        {
            delete *(_value.end() - 1);
            _value.erase(_value.end() - 1);
        }
    }


    void TagList::remove(Tag *tag)
    {
        std::vector<Tag *>::iterator i;
        for (i = _value.begin(); i < _value.end(); ++i)
        {
            if (*i == tag)
            {
                delete *i;
                _value.erase(i);

                break;
            }
        }
    }


    void TagList::remove(size_t i)
    {
        if (_value.size() > 0 && i < _value.size())
        {
            std::vector<Tag *>::iterator it = _value.begin() + i;
            delete *it;
            _value.erase(it);
        }
    }


    void TagList::clear()
    {
        std::vector<Tag *>::iterator i;
        for (i = _value.begin(); i < _value.end(); ++i)
        {
            delete *i;
            _value.erase(i);
        }
    }


    Tag *TagList::at(size_t i) const
    {
        if (_value.size() > 0)
            return _value.at(i);
        return nullptr;
    }


    Tag *TagList::back() const
    {
        if (_value.size() > 0)
            return _value.back();
        return nullptr;
    }


    Tag *TagList::front() const
    {
        if (_value.size() > 0)
            return _value.front();
        return nullptr;
    }


    size_t TagList::size() const
    {
        return _value.size();
    }


    uint8_t TagList::getType() const
    {
        return TAG_LIST;
    }

    ByteArray TagList::toByteArray() const
    {
        ByteArray ret = Tag::toByteArray();

        ret.push_back(getChildType());

        int32_t len = _value.size();
        uint8_t *split = reinterpret_cast<uint8_t *>(&len);

        for (int i = 3; i >= 0; --i)
            ret.push_back(split[i]);

        std::vector<Tag *>::const_iterator i;
        for (i = _value.begin(); i != _value.end(); ++i)
        {
            ByteArray tmp = (*i)->toByteArray();
            uint8_t len_parts[2];

            len_parts[0] = tmp.at(2);
            len_parts[1] = tmp.at(1);

            int16_t *len = reinterpret_cast<int16_t*>(len_parts);

            tmp.erase(tmp.begin(), tmp.begin() + 3 + *len);

            for (size_t j = 0; j < tmp.size(); ++j)
                ret.push_back(tmp[j]);
        }

        return ret;
    }


    std::string TagList::toString() const
    {
        std::stringstream ret;
        
        ret << "TAG_List";

        if (!_name.empty())
            ret << "(\"" << _name << "\")";

        ret << ": " << _value.size() << " entries of type " 
                    << getTypeName(_childType) << std::endl 
                    << "{" << std::endl;

        for (size_t i = 0; i < _value.size(); ++i)
            ret << "  "
                << string_replace(_value[i]->toString(), "\n", "\n  ")
                << std::endl;

        ret << "}";

        return ret.str();
    }

    Tag *TagList::clone() const
    {
        TagList *ret = new TagList(_childType, _name);

        std::vector<Tag *>::const_iterator t;
        for (t = _value.begin(); t != _value.end(); ++t)
            ret->append(**t);

        return ret;
    }

    Tag *TagList::operator[](size_t index) const
    {
        return at(index);
    }

    TagList &TagList::operator<<(const Tag &tag)
    {
        append(tag);

        return *this;
    }
}
