#pragma once

#include <ostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <initializer_list>

namespace jg {
namespace test_state {

class prefix;
class formatted;
class value;
class property;

class output
{
public:
    output() = default;

    output(prefix prefix);
    output(value value);
    output(property property);

    output(prefix prefix, value value);
    output(prefix prefix, property property);

    output& operator+=(value value);
    output& operator+=(property property);

    friend std::ostream& operator<<(std::ostream& stream, const output& output)
    {
        return stream << output.m_formatted;
    }

private:
    std::string m_prefix;
    std::string m_formatted;
};

class value
{
public:
    value(formatted formatted);
    template <typename T> value(const T& value);
    template <typename T> value(std::initializer_list<T> values);

    friend std::ostream& operator<<(std::ostream& stream, const value& value)
    {
        return stream << value.m_formatted;
    }

private:
    value() = default;

    std::string m_formatted;
};

value object(std::initializer_list<property> properties);
value array(std::initializer_list<value> values);
template <typename TIterator> value array(TIterator first_value, TIterator last_value);
template <typename TContainer> value array(const TContainer& values);

class property
{
public:
    property(const char* name, const value& value);
    property(const char* name, std::initializer_list<value> values);

    friend std::ostream& operator<<(std::ostream& stream, const property& property)
    {
        return stream << property.m_formatted;
    }

private:
    std::string m_formatted;
};

class prefix
{
public:
    prefix() = default;
    explicit prefix(std::string value);
    operator std::string() const;
    static prefix google_test();

private:
    std::string m_value;
};

class formatted
{
public:
    formatted() = default;
    explicit formatted(std::string value);
    operator std::string() const;

private:
    std::string m_value;
};

namespace detail {

void surround(std::ostream& stream, const char* text, const char* left, const char* right);
void surround(std::ostream& stream, const std::string& text, const char* left, const char* right);

void quote(std::ostream& stream, const char* text);
void quote(std::ostream& stream, const std::string& text);

template <typename T>
using is_string = std::is_base_of<std::string, T>;

template <typename T>
using is_voidptr = std::is_same<void*, T>;

template <typename T>
using is_const_voidptr = std::is_same<const void*, T>;

template <typename T, typename std::enable_if<!is_string<T>::value && !is_voidptr<T>::value && !is_const_voidptr<T>::value, T>::type* = nullptr>
void value(std::ostream& stream, const T& value);

void value(std::ostream& stream, const std::string& value);
void value(std::ostream& stream, const void* value);
void value(std::ostream& stream, const char* value);
void value(std::ostream& stream, bool value);

class append_after_first_call
{
public:
    append_after_first_call(std::string string);
    std::ostream& operator()(std::ostream& stream);

private:
    bool m_after_first{false};
    std::string m_string;
};

} // namespace detail

inline value::value(formatted formatted)
    : m_formatted{formatted}
{}

template <typename T>
value::value(const T& value)
{
    static_assert(!std::is_base_of<property, T>::value, "A 'value' cannot be constructed from a 'property'");

    std::ostringstream stream;
    detail::value(stream, value);
    m_formatted = stream.str();
}

template <typename T>
value::value(std::initializer_list<T> values)
{
    static_assert(!std::is_base_of<property, T>::value, "A 'value' cannot be constructed from 'initializer_list<property>'");

    detail::append_after_first_call append_comma{", "};
    std::ostringstream value_stream;
    for (const auto &value : values) {
        append_comma(value_stream);
        detail::value(value_stream, value);
    }

    std::ostringstream stream;
    detail::surround(stream, value_stream.str(), "[", "]");
    m_formatted = stream.str();
}

inline value object(std::initializer_list<property> properties)
{
    detail::append_after_first_call append_comma{", "};
    std::ostringstream property_stream;
    for (const auto& property : properties)
        append_comma(property_stream) << property;

    std::ostringstream stream;
    detail::surround(stream, property_stream.str(), "{", "}");

    return value(formatted(stream.str()));
}

inline value array(std::initializer_list<value> values)
{
    detail::append_after_first_call append_comma{", "};
    std::ostringstream value_stream;
    for (const auto& value : values)
        append_comma(value_stream) << value;

    std::ostringstream stream;
    detail::surround(stream, value_stream.str(), "[", "]");

    return value(formatted(stream.str()));
}

template <typename TIterator>
value array(TIterator first_value, TIterator last_value)
{
    detail::append_after_first_call append_comma{", "};
    std::ostringstream value_stream;
    for (TIterator it = first_value; it != last_value; ++it) {
        append_comma(value_stream);
        detail::value(value_stream, *it);
    }

    std::ostringstream stream;
    detail::surround(stream, value_stream.str(), "[", "]");

    return value(formatted(stream.str()));
}

template <typename TContainer>
value array(const TContainer& values)
{
    return array(values.begin(), values.end());
}

inline property::property(const char* name, const value& value)
{
    std::ostringstream stream;
    detail::quote(stream, name);
    stream << ": " << value;
    m_formatted = stream.str();
}

inline property::property(const char* name, std::initializer_list<value> values)
{
    detail::append_after_first_call append_comma{", "};
    std::ostringstream property_stream;
    for (const auto& value : values)
        append_comma(property_stream) << value;

    std::ostringstream stream;
    detail::quote(stream, name);
    stream << ": ";
    detail::surround(stream, property_stream.str(), "[", "]");
    m_formatted = stream.str();
}

inline prefix::prefix(std::string value)
    : m_value{std::move(value)}
{}

inline prefix::operator std::string() const
{
    return m_value;
}

inline prefix prefix::google_test()
{
    return prefix{"[    STATE ] "};
}

inline formatted::formatted(std::string value)
    : m_value{std::move(value)}
{}

inline formatted::operator std::string() const
{
    return m_value;
}

inline output::output(prefix prefix)
    : m_prefix{prefix}
{}

inline output::output(value value)
{
    *this += value;
}

inline output::output(property property)
{
    *this += property;
}

inline output::output(prefix prefix, property property)
    : m_prefix{prefix}
{
    *this += property;
}

inline output::output(prefix prefix, value value)
    : m_prefix{prefix}
{
    *this += value;
}

inline output& output::operator+=(property property)
{
    std::ostringstream stream;
    if (!m_formatted.empty())
        stream << "\n";
    stream << m_prefix << property;
    m_formatted += stream.str();
    return *this;
}

inline output& output::operator+=(value value)
{
    std::ostringstream stream;
    if (!m_formatted.empty())
        stream << "\n";
    stream << m_prefix << value;
    m_formatted += stream.str();
    return *this;
}

namespace detail {

inline void surround(std::ostream& stream, const char* text, const char* left, const char* right)
{
    stream << left;
    if (text[0] != '\0')
        stream << ' ' << text << ' ';
    stream << right;
}

inline void surround(std::ostream& stream, const std::string& text, const char* left, const char* right)
{
    surround(stream, text.c_str(), left, right);
}

inline void quote(std::ostream& stream, const char* text)
{
    stream << '\"' << text << '\"';
}

inline void quote(std::ostream& stream, const std::string& text)
{
    return quote(stream, text.c_str());
}

template <typename T, typename std::enable_if<!is_string<T>::value && !is_voidptr<T>::value && !std::is_same<const void*, T>::value, T>::type*>
void value(std::ostream& stream, const T& value)
{
    stream << value;
}

void value(std::ostream& stream, const std::string& value)
{
    quote(stream, value);
}

inline void value(std::ostream& stream, const void* value)
{
    stream << "0x"
           << std::hex
           << std::setw(sizeof(void*) * 2)
           << std::setfill('0')
           << reinterpret_cast<std::uintptr_t>(value);
}

inline void value(std::ostream& stream, const char* value)
{
    quote(stream, value);
}

inline void value(std::ostream& stream, bool value)
{
    stream << std::boolalpha << value;
}

inline append_after_first_call::append_after_first_call(std::string string)
    : m_string{std::move(string)}
{}

inline std::ostream& append_after_first_call::operator()(std::ostream& stream)
{
    if (m_after_first)
        stream << m_string;
    else
        m_after_first = true;
    return stream;
}

} // namespace detail
} // namespace test_state
} // namespace jg
