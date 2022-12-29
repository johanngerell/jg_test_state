#pragma once

#include <ostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <initializer_list>

namespace jg {
namespace test_state {

namespace detail {

/// A strong type used for avoiding that certain string-based parameters -- that aren't semantically
/// values -- are template-resolved as values.
template <typename T, typename Tag>
struct strong_type final
{
    explicit strong_type(T value)
        : underlying{std::move(value)}
    {}

    friend std::ostream& operator<<(std::ostream& stream, const strong_type& value)
    {
        return stream << value.underlying;
    }

    T underlying{};
};

} // namespace detail

using prefix = detail::strong_type<std::string, struct prefix_tag>;
prefix google_test_prefix();

using formatted = detail::strong_type<std::string, struct formatted_tag>;

class value;
class property;

class output final
{
public:
    output() = default;

    output(prefix prefix);
    output(const value& value);
    output(const property& property);

    output(prefix prefix, const value& value);
    output(prefix prefix, const property& property);

    output& operator+=(const value& value);
    output& operator+=(const property& property);

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
    explicit value(formatted formatted);
    template <typename T> value(const T& value);

    friend std::ostream& operator<<(std::ostream& stream, const value& value)
    {
        return stream << value.m_formatted;
    }

private:
    friend output;
    friend property;

    std::string m_formatted;
};

value array(std::initializer_list<value> values);
template <typename T> value array(std::initializer_list<T> values);
template <typename TIterator> value array(TIterator first_value, TIterator last_value);
template <typename TRange> value array(const TRange& values);

value object(std::initializer_list<property> properties);

class property final
{
public:
    property(const std::string& name, const value& value);

    friend std::ostream& operator<<(std::ostream& stream, const property& property)
    {
        return stream << property.m_formatted;
    }

private:
    friend output;

    std::string m_formatted;
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
    : m_formatted{std::move(formatted.underlying)}
{}

template <typename T>
value::value(const T& value)
{
    static_assert(!std::is_same<property, T>::value, "A 'value' cannot be constructed from a 'property'");
    std::ostringstream stream;
    detail::output_value(stream, value);
    m_formatted = stream.str();
}

inline value object(std::initializer_list<property> properties)
{
    detail::output_after_first_call output_after_first_call;
    std::ostringstream stream;
    for (const auto& property : properties)
        output_after_first_call(stream, ", ") << property;
    return value{formatted(detail::surround(stream.str(), "{", "}"))};
}

inline value array(std::initializer_list<value> values)
{
    return array(values.begin(), values.end());
}

template <typename T>
value array(std::initializer_list<T> values)
{
    return array(values.begin(), values.end());
}

template <typename TIterator>
value array(TIterator first_value, TIterator last_value)
{
    detail::output_after_first_call output_after_first_call;
    std::ostringstream stream;
    for (TIterator it = first_value; it != last_value; ++it)
        output_after_first_call(stream, ", ") << value(*it);
    return value{formatted(detail::surround(stream.str(), "[", "]"))};
}

template <typename TRange>
value array(const TRange& values)
{
    return array(std::begin(values), std::end(values));
}

inline property::property(const std::string& name, const value& value)
{
    m_formatted.reserve(name.length() + 2 + 2 + value.m_formatted.length());
    m_formatted.append(detail::quote(name)).append(": ").append(value.m_formatted);
}

inline prefix google_test_prefix()
{
    return prefix{"[    STATE ] "};
}

inline output::output(prefix prefix)
    : m_prefix{std::move(prefix.underlying)}
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
    : m_prefix{std::move(prefix.underlying)}
{
    *this += property;
}

inline output::output(prefix prefix, const value& value)
    : m_prefix{std::move(prefix.underlying)}
{
    *this += value;
}

inline output& output::operator+=(const property& property)
{
    m_formatted.reserve(m_formatted.length() + (m_formatted.empty() ? 0 : 1) + m_prefix.length() + property.m_formatted.length());
    if (!m_formatted.empty())
        m_formatted.append(1, '\n');
    m_formatted.append(m_prefix).append(property.m_formatted);
    return *this;
}

inline output& output::operator+=(const value& value)
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

inline void output_value(std::ostream& stream, void* value)
{
    output_value(stream, const_cast<const void*>(value));
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
