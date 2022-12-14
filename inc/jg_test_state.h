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

class output
{
public:
    output() = default;
    output(const property& p);
    output(const value& v);

    static output with_prefix(std::string prefix);
    static output with_prefix(std::string prefix, const property& p);
    static output with_prefix(std::string prefix, const value& v);

    static output with_google_test_prefix();
    static output with_google_test_prefix(const property& p);
    static output with_google_test_prefix(const value& v);

    output& operator+=(const property& p);
    output& operator+=(const value& v);

    friend std::ostream& operator<<(std::ostream& stream, const output& s)
    {
        return stream << s.formatted;
    }

private:
    std::string prefix;
    std::string formatted;
};

class value
{
public:
    template <typename T> value(const T& value);
    template <typename T> value(std::initializer_list<T> value);

    static value preformatted(std::string formatted);

    friend std::ostream& operator<<(std::ostream& stream, const value& v)
    {
        return stream << v.formatted;
    }

private:
    value() = default;

    std::string formatted;
};

value object(std::initializer_list<property> props);
value array(std::initializer_list<value> values);
template <typename It> value array(It first, It last);
template <typename T> value array(const T& container);

class property
{
public:
    property(const char* name, const value& value);
    property(const char* name, std::initializer_list<value> values);

    friend std::ostream& operator<<(std::ostream& stream, const property& p)
    {
        return stream << p.formatted;
    }

private:
    std::string formatted;
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

} // namespace detail

template <typename T>
value::value(const T& v)
{
    static_assert(!std::is_base_of<property, T>::value, "A 'value' cannot be constructed from a 'property'");

    std::ostringstream value_stream;
    detail::value(value_stream, v);
    formatted = value_stream.str();
}

template <typename T>
value::value(std::initializer_list<T> v)
{
    static_assert(!std::is_base_of<property, T>::value, "A 'value' cannot be constructed from 'initializer_list<property>'");

    for (const auto& val : v) {
        std::ostringstream stream;
        stream << (!formatted.empty() ? ", " : "");
        detail::value(stream, val);
        formatted += stream.str();
    }

    std::ostringstream stream;
    detail::surround(stream, formatted, "[", "]");
    formatted = stream.str();
}

inline value value::preformatted(std::string formatted)
{
    value v;
    v.formatted = formatted;
    return v;
}

inline value object(std::initializer_list<property> props)
{
    std::string formatted;
    for (const auto& p : props) {
        std::ostringstream stream;
        stream << (!formatted.empty() ? ", " : "");
        stream << p;
        formatted += stream.str();
    }

    std::ostringstream stream;
    detail::surround(stream, formatted, "{", "}");
    formatted = stream.str();

    return value::preformatted(formatted);
}

inline value array(std::initializer_list<value> values)
{
    std::string formatted;
    for (const auto& val : values) {
        std::ostringstream stream;
        stream << (!formatted.empty() ? ", " : "");
        stream << val;
        formatted += stream.str();
    }

    std::ostringstream stream;
    detail::surround(stream, formatted, "[", "]");
    formatted = stream.str();

    return value::preformatted(formatted);
}

template <typename It>
value array(It first, It last)
{
    std::string formatted;
    for (It it = first; it != last; ++it) {
        std::ostringstream stream;
        stream << (!formatted.empty() ? ", " : "");
        detail::value(stream, *it);
        formatted += stream.str();
    }

    std::ostringstream stream;
    detail::surround(stream, formatted, "[", "]");
    formatted = stream.str();

    return value::preformatted(formatted);
}

template <typename T>
value array(const T& container)
{
    return array(container.begin(), container.end());
}

inline property::property(const char* name, const value& val)
{
    std::ostringstream stream;
    detail::quote(stream, name);
    stream << ": " << val;
    formatted = stream.str();
}

inline property::property(const char* name, std::initializer_list<value> vals)
{
    std::ostringstream stream;
    detail::quote(stream, name);
    stream << ": ";

    std::string formatted_values;
    for (const auto& val : vals) {
        std::ostringstream value_stream;
        value_stream << (!formatted_values.empty() ? ", " : "") << val;
        formatted_values += value_stream.str();
    }

    detail::surround(stream, formatted_values, "[", "]");
    formatted = stream.str();
}

inline output::output(const value& v)
{
    *this += v;
}

inline output::output(const property& p)
{
    *this += p;
}

inline output output::with_prefix(std::string prefix)
{
    output state;
    state.prefix = prefix;
    return state;
}

inline output output::with_prefix(std::string prefix, const property& p)
{
    output state;
    state.prefix = prefix;
    state += p;
    return state;
}

inline output output::with_prefix(std::string prefix, const value& v)
{
    output state;
    state.prefix = prefix;
    state += v;
    return state;
}

inline output output::with_google_test_prefix()
{
    return with_prefix("[    STATE ] ");
}

inline output output::with_google_test_prefix(const property& p)
{
    return with_prefix("[    STATE ] ", p);
}

inline output output::with_google_test_prefix(const value& v)
{
    return with_prefix("[    STATE ] ", v);
}

inline output& output::operator+=(const property& p)
{
    std::ostringstream stream;
    stream << (!formatted.empty() ? "\n" : "") << prefix;
    stream << p;
    formatted += stream.str();
    return *this;
}

inline output& output::operator+=(const value& v)
{
    std::ostringstream stream;
    stream << (!formatted.empty() ? "\n" : "") << prefix;
    stream << v;
    formatted += stream.str();
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

} // namespace detail
} // namespace test_state
} // namespace jg
