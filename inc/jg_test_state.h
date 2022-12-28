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

class output final
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

class value final
{
public:
    value(formatted formatted);
    template <typename T> value(const T& value);
    template <typename T> value(std::initializer_list<T> array_values);

    friend std::ostream& operator<<(std::ostream& stream, const value& value)
    {
        return stream << value.m_formatted;
    }

private:
    friend output;
    friend property;

    value() = default;

    std::string m_formatted;
};

value array(std::initializer_list<value> values);
template <typename TIterator> value array(TIterator first_value, TIterator last_value);
template <typename TRange> value array(const TRange& values);

value object(std::initializer_list<property> properties);

class property final
{
public:
    property(std::string name, value value);
    property(std::string name, std::initializer_list<value> values);

    friend std::ostream& operator<<(std::ostream& stream, const property& property)
    {
        return stream << property.m_formatted;
    }

private:
    friend output;

    std::string m_formatted;
};

class prefix final
{
public:
    prefix() = default;
    explicit prefix(std::string value);
    std::string get() const;

private:
    std::string m_value;
};

prefix google_test_prefix();

class formatted final
{
public:
    formatted() = default;
    explicit formatted(std::string value);
    std::string get() const;

private:
    std::string m_value;
};

namespace detail {

std::string surround(const std::string& text, const std::string& left, const std::string& right);
std::string quote(const std::string& text);

template <typename T>
void output_value(std::ostream& stream, const T& value);
void output_value(std::ostream& stream, std::string value);
void output_value(std::ostream& stream, void* value);
void output_value(std::ostream& stream, const void* value);
void output_value(std::ostream& stream, char* value);
void output_value(std::ostream& stream, const char* value);
void output_value(std::ostream& stream, bool value);

/// Function object that outputs a string to a stream when called, unless it's the first time it's called.
/// Use this for separating output items during iteration, like "1, apple, 3, false".
class output_after_first_call final
{
public:
    std::ostream& operator()(std::ostream& stream, const std::string& string);

private:
    bool m_after_first{false};
};

} // namespace detail

inline value::value(formatted formatted)
    : m_formatted{formatted.get()}
{}

template <typename T>
value::value(const T& value)
{
    static_assert(!std::is_same<property, T>::value, "A 'value' cannot be constructed from a 'property'");
    std::ostringstream stream;
    detail::output_value(stream, value);
    m_formatted = stream.str();
}

template <typename T>
value::value(std::initializer_list<T> array_values)
{
    static_assert(!std::is_same<property, T>::value, "A 'value' cannot be constructed from a 'property'");
    detail::output_after_first_call output_after_first_call;
    std::ostringstream value_stream;
    for (const auto &value : array_values) {
        output_after_first_call(value_stream, ", ");
        detail::output_value(value_stream, value);
    }
    m_formatted = detail::surround(value_stream.str(), "[", "]");
}

inline value object(std::initializer_list<property> properties)
{
    detail::output_after_first_call output_after_first_call;
    std::ostringstream property_stream;
    for (const auto& property : properties)
        output_after_first_call(property_stream, ", ") << property;
    return value(formatted(detail::surround(property_stream.str(), "{", "}")));
}

inline value array(std::initializer_list<value> values)
{
    detail::output_after_first_call output_after_first_call;
    std::ostringstream value_stream;
    for (const auto& value : values)
        output_after_first_call(value_stream, ", ") << value;
    return value(formatted(detail::surround(value_stream.str(), "[", "]")));
}

template <typename TIterator>
value array(TIterator first_value, TIterator last_value)
{
    detail::output_after_first_call output_after_first_call;
    std::ostringstream value_stream;
    for (TIterator it = first_value; it != last_value; ++it)
        output_after_first_call(value_stream, ", ") << value(*it);
    return value(formatted(detail::surround(value_stream.str(), "[", "]")));
}

template <typename TRange>
value array(const TRange& values)
{
    return array(std::begin(values), std::end(values));
}

inline property::property(std::string name, value value)
{
    m_formatted.reserve(name.length() + 2 + 2 + value.m_formatted.length());
    m_formatted.append(detail::quote(name)).append(": ").append(value.m_formatted);
}

inline property::property(std::string name, std::initializer_list<value> values)
{
    detail::output_after_first_call output_after_first_call;
    std::ostringstream value_stream;
    for (const auto& value : values)
        output_after_first_call(value_stream, ", ") << value;
    *this = property(name, value(formatted(detail::surround(value_stream.str(), "[", "]"))));
}

inline prefix::prefix(std::string value)
    : m_value{std::move(value)}
{}

inline std::string prefix::get() const
{
    return m_value;
}

inline prefix google_test_prefix()
{
    return prefix{"[    STATE ] "};
}

inline formatted::formatted(std::string value)
    : m_value{std::move(value)}
{}

inline std::string formatted::get() const
{
    return m_value;
}

inline output::output(prefix prefix)
    : m_prefix{prefix.get()}
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
    : m_prefix{prefix.get()}
{
    *this += property;
}

inline output::output(prefix prefix, value value)
    : m_prefix{prefix.get()}
{
    *this += value;
}

inline output& output::operator+=(property property)
{
    m_formatted.reserve(m_formatted.length() + (m_formatted.empty() ? 0 : 1) + m_prefix.length() + property.m_formatted.length());
    if (!m_formatted.empty())
        m_formatted.append(1, '\n');
    m_formatted.append(m_prefix).append(property.m_formatted);
    return *this;
}

inline output& output::operator+=(value value)
{
    m_formatted.reserve(m_formatted.length() + (m_formatted.empty() ? 0 : 1) + m_prefix.length() + value.m_formatted.length());
    if (!m_formatted.empty())
        m_formatted.append(1, '\n');
    m_formatted.append(m_prefix).append(value.m_formatted);
    return *this;
}

namespace detail {

inline std::string surround(const std::string& text, const std::string& left, const std::string& right)
{
    std::string text_;
    text_.reserve(text.length() + (text.empty() ? 0 : 2) + left.length() + right.length());
    text_.append(left);
    if (!text.empty())
        text_.append(1, ' ').append(text).append(1, ' ');
    text_.append(right);
    return text_;
}

inline std::string quote(const std::string& text)
{
    std::string text_;
    text_.reserve(text.length() + 2);
    text_.append(1, '\"').append(text).append(1, '\"');
    return text_;
}

template <typename T>
void output_value(std::ostream& stream, const T& value)
{
    stream << value;
}

inline void output_value(std::ostream& stream, std::string value)
{
    stream << quote(value);
}

inline void output_value(std::ostream& stream, void* value_)
{
    output_value(stream, const_cast<const void*>(value_));
}

inline void output_value(std::ostream& stream, const void* value)
{
    stream << "0x"
           << std::hex
           << std::setw(sizeof(void*) * 2)
           << std::setfill('0')
           << reinterpret_cast<std::uintptr_t>(value);
}

inline void output_value(std::ostream& stream, char* value)
{
    stream << quote(value);
}

inline void output_value(std::ostream& stream, const char* value)
{
    stream << quote(value);
}

inline void output_value(std::ostream& stream, bool value)
{
    stream << std::boolalpha << value;
}

inline std::ostream& output_after_first_call::operator()(std::ostream& stream, const std::string& string)
{
    if (m_after_first)
        stream << string;
    else
        m_after_first = true;
    return stream;
}

} // namespace detail
} // namespace test_state
} // namespace jg
