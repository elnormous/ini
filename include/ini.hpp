// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef INI_HPP
#define INI_HPP

#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ini
{
    class ParseError final: public std::logic_error
    {
    public:
        using std::logic_error::logic_error;
    };

    class RangeError final: public std::range_error
    {
    public:
        using std::range_error::range_error;
    };

    using Values = std::map<std::string, std::string, std::less<>>;

    class Section final
    {
    public:
        Section() = default;

        explicit Section(const std::string& initName):
            name{initName}
        {
        }

        auto begin() noexcept
        {
            return values.begin();
        }

        auto end() noexcept
        {
            return values.end();
        }

        auto begin() const noexcept
        {
            return values.begin();
        }

        auto end() const noexcept
        {
            return values.end();
        }

        const std::string& getName() const noexcept { return name; }
        void setName(const std::string& newName) { name = newName; }

        const Values& getValues() const noexcept { return values; }

        bool hasValue(const std::string_view key) const
        {
            return values.find(key) != values.end();
        }

        std::string& operator[](const std::string_view key)
        {
            if (const auto valueIterator = values.find(key); valueIterator != values.end())
                return valueIterator->second;
            else
            {
                const auto& [newIterator, success] = values.insert(std::make_pair(key, std::string{}));
                (void)success;
                return newIterator->second;
            }
        }

        const std::string& operator[](const std::string_view key) const
        {
            if (const auto valueIterator = values.find(key); valueIterator != values.end())
                return valueIterator->second;
            else
                throw RangeError{"Value does not exist"};
        }

        const std::string& getValue(const std::string_view key, const std::string& defaultValue = {}) const
        {
            if (const auto valueIterator = values.find(key); valueIterator != values.end())
                return valueIterator->second;

            return defaultValue;
        }

        void setValue(const std::string_view key, const std::string& value)
        {
            values.insert(std::make_pair(key, value));
        }

        void deleteValue(const std::string_view key)
        {
            if (const auto valueIterator = values.find(key); valueIterator != values.end())
                values.erase(valueIterator);
        }

        std::size_t getSize() const noexcept
        {
            return values.size();
        }

    private:
        std::string name;
        Values values;
    };

    class Data final
    {
    public:
        using Sections = std::map<std::string, Section, std::less<>>;

        Data() = default;

        const Sections& getSections() const noexcept { return sections; }

        auto begin() noexcept
        {
            return sections.begin();
        }

        auto end() noexcept
        {
            return sections.end();
        }

        auto begin() const noexcept
        {
            return sections.begin();
        }

        auto end() const noexcept
        {
            return sections.end();
        }

        bool hasSection(const std::string_view name) const
        {
            return sections.find(name) != sections.end();
        }

        Section& operator[](const std::string_view name)
        {
            if (const auto sectionIterator = sections.find(name); sectionIterator != sections.end())
                return sectionIterator->second;
            else
            {
                const auto& [newIterator, success] = sections.insert(std::make_pair(name, Section{}));
                (void)success;
                return newIterator->second;
            }
        }

        const Section& operator[](const std::string_view name) const
        {
            if (const auto sectionIterator = sections.find(name); sectionIterator != sections.end())
                return sectionIterator->second;
            else
                throw RangeError{"Section does not exist"};
        }

        void eraseSection(const std::string_view name)
        {
            if (const auto sectionIterator = sections.find(name); sectionIterator != sections.end())
                sections.erase(sectionIterator);
        }

        std::size_t getSize() const noexcept
        {
            return sections.size();
        }

    private:
        Sections sections;
    };

    inline namespace detail
    {
        constexpr std::array<std::uint8_t, 3> utf8ByteOrderMark = {0xEF, 0xBB, 0xBF};
    }

    template <class Iterator>
    Data parse(Iterator begin, Iterator end)
    {
        class Parser final
        {
        public:
            static Data parse(Iterator begin, Iterator end)
            {
                Data result;

                std::string section;

                for (auto iterator = hasByteOrderMark(begin, end) ? begin + 3 : begin; iterator != end;)
                {
                    if (isWhitespace(static_cast<char>(*iterator)) ||
                        static_cast<char>(*iterator) == '\n' ||
                        static_cast<char>(*iterator) == '\r') // line starts with a whitespace
                        ++iterator; // skip the white space
                    else if (static_cast<char>(*iterator) == '[') // section
                    {
                        ++iterator; // skip the left bracket

                        bool parsedSection = false;

                        for (;;)
                        {
                            if (iterator == end ||
                                static_cast<char>(*iterator) == '\n' ||
                                static_cast<char>(*iterator) == '\r')
                            {
                                if (!parsedSection)
                                    throw ParseError{"Unexpected end of section"};

                                ++iterator; // skip the newline
                                break;
                            }
                            else if (static_cast<char>(*iterator) == ';')
                            {
                                ++iterator; // skip the semicolon

                                if (!parsedSection)
                                    throw ParseError{"Unexpected comment"};

                                while (iterator != end)
                                {
                                    if (static_cast<char>(*iterator) == '\n' ||
                                        static_cast<char>(*iterator) == '\r')
                                    {
                                        ++iterator; // skip the newline
                                        break;
                                    }

                                    ++iterator;
                                }
                                break;
                            }
                            else if (static_cast<char>(*iterator) == ']')
                                parsedSection = true;
                            else if (static_cast<char>(*iterator) != ' ' &&
                                     static_cast<char>(*iterator) != '\t')
                            {
                                if (parsedSection)
                                    throw ParseError{"Unexpected character after section"};
                            }

                            if (!parsedSection)
                                section.push_back(static_cast<char>(*iterator));

                            ++iterator;
                        }

                        trim(section);

                        if (section.empty())
                            throw ParseError{"Invalid section name"};

                        result[section] = Section{};
                    }
                    else if (static_cast<char>(*iterator) == ';') // comment
                    {
                        while (++iterator != end)
                        {
                            if (static_cast<char>(*iterator) == '\r' ||
                                static_cast<char>(*iterator) == '\n')
                            {
                                ++iterator; // skip the newline
                                break;
                            }
                        }
                    }
                    else // key, value pair
                    {
                        std::string key;
                        std::string value;
                        bool parsedKey = false;

                        while (iterator != end)
                        {
                            if (static_cast<char>(*iterator) == '\r' ||
                                static_cast<char>(*iterator) == '\n')
                            {
                                ++iterator; // skip the newline
                                break;
                            }
                            else if (static_cast<char>(*iterator) == '=')
                            {
                                if (!parsedKey)
                                    parsedKey = true;
                                else
                                    throw ParseError{"Unexpected character"};
                            }
                            else if (static_cast<char>(*iterator) == ';')
                            {
                                ++iterator; // skip the semicolon

                                while (iterator != end)
                                {
                                    if (static_cast<char>(*iterator) == '\r' ||
                                        static_cast<char>(*iterator) == '\n')
                                    {
                                        ++iterator; // skip the newline
                                        break;
                                    }

                                    ++iterator;
                                }
                                break;
                            }
                            else
                            {
                                if (!parsedKey)
                                    key.push_back(static_cast<char>(*iterator));
                                else
                                    value.push_back(static_cast<char>(*iterator));
                            }

                            ++iterator;
                        }

                        if (key.empty())
                            throw ParseError{"Invalid key name"};

                        trim(key);
                        trim(value);

                        result[section][key] = value;
                    }
                }

                return result;
            }

        private:
            static bool hasByteOrderMark(Iterator begin, Iterator end) noexcept
            {
                for (const auto b : utf8ByteOrderMark)
                    if (begin == end || static_cast<std::uint8_t>(*begin) != b)
                        return false;
                    else
                        ++begin;
                return true;
            }

            static constexpr bool isWhitespace(const char c) noexcept
            {
                return c == ' ' || c == '\t';
            }

            static std::string& leftTrim(std::string& s)
            {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                                [](char c) noexcept {return !isWhitespace(c);}));
                return s;
            }

            static std::string& rightTrim(std::string& s)
            {
                s.erase(std::find_if(s.rbegin(), s.rend(),
                                     [](char c) noexcept {return !isWhitespace(c);}).base(), s.end());
                return s;
            }

            static std::string& trim(std::string& s)
            {
                return leftTrim(rightTrim(s));
            }
        };

        return Parser::parse(begin, end);
    }

    inline Data parse(const char* data)
    {
        auto end = data;
        while (*end) ++end;
        return parse(data, end);
    }

    template <class T>
    Data parse(const T& data)
    {
        return parse(std::begin(data), std::end(data));
    }

    inline std::string encode(const Data& data, bool byteOrderMark = false)
    {
        std::string result;

        if (byteOrderMark) result.assign(utf8ByteOrderMark.begin(),
                                         utf8ByteOrderMark.end());

        for (const auto& [name, section] : data)
        {
            if (!name.empty())
            {
                result.push_back('[');
                result.insert(result.end(), name.begin(), name.end());
                result.push_back(']');
                result.push_back('\n');
            }

            for (const auto& [key, value] : section)
            {
                result.insert(result.end(), key.begin(), key.end());
                result.push_back('=');
                result.insert(result.end(), value.begin(), value.end());
                result.push_back('\n');
            }
        }

        return result;
    }
} // namespace ini

#endif // INI_HPP
