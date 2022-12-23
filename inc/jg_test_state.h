#pragma once

#include <ostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <initializer_list>

namespace jg {
namespace test_state {

class value;
class property;

struct prefix
{
    std::string value;
    operator std::string() const;
    static prefix google_test();
};

class output
{
public:
    output() = default;

    explicit output(prefix prefix);
    output(const property& property);
    output(const value& value);

    output(prefix prefix, const property& property);
    output(prefix prefix, const value& value);

    output& operator+=(const property& property);
    output& operator+=(const value& value);

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
    template <typename T> value(const T& value);
    template <typename T> value(std::initializer_list<T> values);

    static value formatted(std::string string);

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

namespace detail {

void surround(std::ostream& stream, const char* text, const char* left, const char* right);
void surround(std::ostream& stream, const std::string& text, const char* left, const char* right);

void quote(std::ostream& stream, const char* text);
void quote(std::ostream& stream, const std::string& text);

template <typename T, typename std::enable_if<!std::is_base_of<std::string, T>::value && !std::is_same<void*, T>::value && !std::is_same<const void*, T>::value, T>::type* = nullptr>
void value(std::ostream& stream, const T& value);

template <typename T, typename std::enable_if<std::is_base_of<std::string, T>::value, T>::type* = nullptr>
void value(std::ostream& stream, const T& value);

template <typename T, typename std::enable_if<std::is_same<void*, T>::value || std::is_same<const void*, T>::value, T>::type* = nullptr>
void value(std::ostream& stream, const T& value);

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
    for (const auto& value : values) {
        std::ostringstream stream;
        append_comma(stream);
        detail::value(stream, value);
        m_formatted += stream.str();
    }

    std::ostringstream stream;
    detail::surround(stream, m_formatted, "[", "]");
    m_formatted = stream.str();
}

inline value value::formatted(std::string string)
{
    value value;
    value.m_formatted = string;
    return value;
}

inline value object(std::initializer_list<property> properties)
{
    detail::append_after_first_call append_comma{", "};
    std::string formatted;
    for (const auto& property : properties) {
        std::ostringstream stream;
        append_comma(stream) << property;
        formatted += stream.str();
    }

    std::ostringstream stream;
    detail::surround(stream, formatted, "{", "}");
    formatted = stream.str();

    return value::formatted(formatted);
}

inline value array(std::initializer_list<value> values)
{
    detail::append_after_first_call append_comma{", "};
    std::string formatted;
    for (const auto& value : values) {
        std::ostringstream stream;
        append_comma(stream) << value;
        formatted += stream.str();
    }

    std::ostringstream stream;
    detail::surround(stream, formatted, "[", "]");
    formatted = stream.str();

    return value::formatted(formatted);
}

template <typename TIterator>
value array(TIterator first_value, TIterator last_value)
{
    detail::append_after_first_call append_comma{", "};
    std::string formatted;
    for (TIterator it = first_value; it != last_value; ++it) {
        std::ostringstream stream;
        append_comma(stream);
        detail::value(stream, *it);
        formatted += stream.str();
    }

    std::ostringstream stream;
    detail::surround(stream, formatted, "[", "]");
    formatted = stream.str();

    return value::formatted(formatted);
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
    std::string formatted_values;
    for (const auto& value : values) {
        std::ostringstream stream;
        append_comma(stream) << value;
        formatted_values += stream.str();
    }

    std::ostringstream stream;
    detail::quote(stream, name);
    stream << ": ";
    detail::surround(stream, formatted_values, "[", "]");
    m_formatted = stream.str();
}

inline prefix::operator std::string() const
{
    return value;
}

inline prefix prefix::google_test()
{
    return {"[    STATE ] "};
}

inline output::output(prefix prefix)
    : m_prefix{prefix}
{}

inline output::output(const value& value)
{
    *this += value;
}

inline output::output(const property& property)
{
    *this += property;
}

inline output::output(prefix prefix, const property& property)
    : m_prefix{prefix}
{
    *this += property;
}

inline output::output(prefix prefix, const value& value)
    : m_prefix{prefix}
{
    *this += value;
}

inline output& output::operator+=(const property& property)
{
    std::ostringstream stream;
    stream << (!m_formatted.empty() ? "\n" : "") << m_prefix;
    stream << property;
    m_formatted += stream.str();
    return *this;
}

inline output& output::operator+=(const value& value)
{
    std::ostringstream stream;
    stream << (!m_formatted.empty() ? "\n" : "") << m_prefix;
    stream << value;
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

template <typename T, typename std::enable_if<!std::is_base_of<std::string, T>::value && !std::is_same<void*, T>::value && !std::is_same<const void*, T>::value, T>::type*>
void value(std::ostream& stream, const T& value)
{
    stream << value;
}

template <typename T, typename std::enable_if<std::is_base_of<std::string, T>::value, T>::type*>
void value(std::ostream& stream, const T& value)
{
    quote(stream, value);
}

template <typename T, typename std::enable_if<std::is_same<void*, T>::value || std::is_same<const void*, T>::value, T>::type*>
void value(std::ostream& stream, const T& value)
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
