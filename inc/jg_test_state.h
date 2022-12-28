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
    std::string m_formatted;
};

class prefix final
{
public:
    prefix() = default;
    explicit prefix(std::string value);
    operator std::string() const;
    static prefix google_test();

private:
    std::string m_value;
};

class formatted final
{
public:
    formatted() = default;
    explicit formatted(std::string value);
    operator std::string() const;

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
    output_after_first_call(std::string string);
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
    static_assert(!std::is_same<property, T>::value, "A 'value' cannot be constructed from a 'property'");
    std::ostringstream stream;
    detail::output_value(stream, value);
    m_formatted = stream.str();
}

template <typename T>
value::value(std::initializer_list<T> array_values)
{
    static_assert(!std::is_same<property, T>::value, "A 'value' cannot be constructed from a 'property'");
    detail::output_after_first_call output_comma{", "};
    std::ostringstream value_stream;
    for (const auto &value : array_values) {
        output_comma(value_stream);
        detail::output_value(value_stream, value);
    }
    m_formatted = detail::surround(value_stream.str(), "[", "]");
}

inline value object(std::initializer_list<property> properties)
{
    detail::output_after_first_call output_comma{", "};
    std::ostringstream property_stream;
    for (const auto& property : properties)
        output_comma(property_stream) << property;
    return value(formatted(detail::surround(property_stream.str(), "{", "}")));
}

inline value array(std::initializer_list<value> values)
{
    detail::output_after_first_call output_comma{", "};
    std::ostringstream value_stream;
    for (const auto& value : values)
        output_comma(value_stream) << value;
    return value(formatted(detail::surround(value_stream.str(), "[", "]")));
}

template <typename TIterator>
value array(TIterator first_value, TIterator last_value)
{
    detail::output_after_first_call output_comma{", "};
    std::ostringstream value_stream;
    for (TIterator it = first_value; it != last_value; ++it)
        output_comma(value_stream) << value(*it);
    return value(formatted(detail::surround(value_stream.str(), "[", "]")));
}

template <typename TRange>
value array(const TRange& values)
{
    return array(std::begin(values), std::end(values));
}

inline property::property(std::string name, value value)
{
    std::ostringstream stream;
    stream << detail::quote(name) << ": " << value;
    m_formatted = stream.str();
}

inline property::property(std::string name, std::initializer_list<value> values)
{
    detail::output_after_first_call output_comma{", "};
    std::ostringstream property_stream;
    for (const auto& value : values)
        output_comma(property_stream) << value;

    std::ostringstream stream;
    stream << detail::quote(name) << ": " << detail::surround(property_stream.str(), "[", "]");
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

inline output_after_first_call::output_after_first_call(std::string string)
    : m_string{std::move(string)}
{}

inline std::ostream& output_after_first_call::operator()(std::ostream& stream)
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
