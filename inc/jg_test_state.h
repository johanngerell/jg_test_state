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

class surrounded
{
public:
    surrounded(const std::string& text, const std::string& left, const std::string& right)
        : m_text{&text}
        , m_left{&left}
        , m_right{&right}
    {}

    friend std::ostream& operator<<(std::ostream& stream, const surrounded& s)
    {
        stream << *s.m_left;
        if (!s.m_text->empty())
            stream << ' ' << *s.m_text << ' ';
        stream << *s.m_right;
        return stream;
    }

private:
    const std::string* m_text;
    const std::string* m_left;
    const std::string* m_right;
};

inline surrounded surround(const std::string& text, const std::string& left, const std::string& right);

class quoted
{
public:
    quoted(const std::string& text)
        : m_text{&text}
    {}

    friend std::ostream& operator<<(std::ostream& stream, const quoted& q)
    {
        stream << '\"' << *q.m_text << '\"';
        return stream;
    }

private:
    const std::string* m_text;
};

inline quoted quote(const std::string& text);

template <typename T>
void value(std::ostream& stream, const T& value);
void value(std::ostream& stream, std::string value);
void value(std::ostream& stream, void* value);
void value(std::ostream& stream, const void* value);
void value(std::ostream& stream, char* value);
void value(std::ostream& stream, const char* value);
void value(std::ostream& stream, bool value);

class append_after_first_call final
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
std::string value_to_string(const T& value)
{
    std::ostringstream stream;
    detail::value(stream, value);
    return stream.str();
}

template <typename T>
value::value(const T& value)
{
    static_assert(!std::is_same<property, T>::value, "A 'value' cannot be constructed from a 'property'");
    m_formatted = value_to_string(value);
}

template <typename T>
value::value(std::initializer_list<T> array_values)
{
    static_assert(!std::is_same<property, T>::value, "A 'value' cannot be constructed from a 'property'");
    detail::append_after_first_call append_comma{", "};
    std::ostringstream value_stream;
    for (const auto &value : array_values) {
        append_comma(value_stream);
        detail::value(value_stream, value);
    }
    std::ostringstream stream;
    stream << detail::surround(value_stream.str(), "[", "]");
    m_formatted = stream.str();
}

inline value object(std::initializer_list<property> properties)
{
    detail::append_after_first_call append_comma{", "};
    std::ostringstream property_stream;
    for (const auto& property : properties)
        append_comma(property_stream) << property;
    std::ostringstream stream;
    stream << detail::surround(property_stream.str(), "{", "}");
    return value(formatted(stream.str()));
}

inline value array(std::initializer_list<value> values)
{
    detail::append_after_first_call append_comma{", "};
    std::ostringstream value_stream;
    for (const auto& value : values)
        append_comma(value_stream) << value;
    std::ostringstream stream;
    stream << detail::surround(value_stream.str(), "[", "]");
    return value(formatted(stream.str()));
}

template <typename TIterator>
value array(TIterator first_value, TIterator last_value)
{
    detail::append_after_first_call append_comma{", "};
    std::ostringstream value_stream;
    for (TIterator it = first_value; it != last_value; ++it)
        append_comma(value_stream) << value(*it);
    std::ostringstream stream;
    stream << detail::surround(value_stream.str(), "[", "]");
    return value(formatted(stream.str()));
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
    detail::append_after_first_call append_comma{", "};
    std::ostringstream property_stream;
    for (const auto& value : values)
        append_comma(property_stream) << value;

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

inline surrounded surround(const std::string& text, const std::string& left, const std::string& right)
{
    return {text, left, right};
}

inline quoted quote(const std::string& text)
{
    return {text};
}

template <typename T>
void value(std::ostream& stream, const T& value)
{
    stream << value;
}

inline void value(std::ostream& stream, std::string value)
{
    stream << quote(value);
}

inline void value(std::ostream& stream, void* value_)
{
    value(stream, const_cast<const void*>(value_));
}

inline void value(std::ostream& stream, const void* value)
{
    stream << "0x"
           << std::hex
           << std::setw(sizeof(void*) * 2)
           << std::setfill('0')
           << reinterpret_cast<std::uintptr_t>(value);
}

inline void value(std::ostream& stream, char* value)
{
    stream << quote(value);
}

inline void value(std::ostream& stream, const char* value)
{
    stream << quote(value);
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
