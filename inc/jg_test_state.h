#pragma once

#include <ostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <initializer_list>

namespace jg {
namespace test_state {

namespace detail {

/// Default validation policy for a `strong_type` instance. The default behavior is to do nothing.
/// If invariants or semantics must hold at construction, then create a policy that asserts or throws
/// an exception in `validate(const T& value)` if they don't hold for `value`.
struct strong_type_no_validation final
{
    template <typename T>
    static void validate(const T&) {}
};

/// A trivial "strong type" to prevent parameters that semantically aren't values from being resolved as
/// such due to the value class "auto resolve" template constructor being implicit.
/// @tparam T Underlying type of this strong type.
/// @tparam Tag Tag type that distinguishes different strong types with the same underlying type.
/// @tparam Validator Validation policy for this strong type. By default, no validation occurs.
template <typename T, typename Tag, typename Validator = strong_type_no_validation>
struct strong_type final
{
    strong_type() = default;
    explicit strong_type(T value)
        : underlying{std::move(value)}
    {
        Validator::validate(underlying);
    }

    T underlying{};
};

} // namespace detail

using prefix_string = detail::strong_type<std::string, struct prefix_tag>;
using formatted_string = detail::strong_type<std::string, struct formatted_tag>;

prefix_string google_test_prefix();

struct value;
struct property;

struct output final
{
    output() = default;
    output(const value& value);
    output(const property& property);
    output(prefix_string prefix);
    output(prefix_string prefix, const value& value);
    output(prefix_string prefix, const property& property);

    output& operator+=(const value& value);
    output& operator+=(const property& property);

    prefix_string prefix;
    formatted_string formatted;
};

inline std::ostream& operator<<(std::ostream& stream, const output& output)
{
    return stream << output.formatted.underlying;
}

struct value final
{
    explicit value(formatted_string formatted);
    template <typename T> value(const T& value);

    formatted_string formatted;
};

inline std::ostream& operator<<(std::ostream& stream, const value& value)
{
    return stream << value.formatted.underlying;
}

value array(std::initializer_list<value> values);
template <typename TIterator> value array(TIterator first_value, TIterator last_value);
template <typename TRange> value array(const TRange& values);

value object(property property);
value object(std::initializer_list<property> properties);
template <typename TIterator> value object(TIterator first_property, TIterator last_property);
template <typename TRange> value object(const TRange& properties);

struct property final
{
    property(const std::string& name, const value& value);

    formatted_string formatted;
};

inline std::ostream& operator<<(std::ostream& stream, const property& property)
{
    return stream << property.formatted.underlying;
}

namespace detail {

std::string curly_bracket(const std::string& text);
std::string square_bracket(const std::string& text);
std::string quote(const std::string& text);

template <typename T>
void output_value(std::ostream& stream, const T& value);
void output_value(std::ostream& stream, const std::string& value);
void output_value(std::ostream& stream, void* value);
void output_value(std::ostream& stream, const void* value);
void output_value(std::ostream& stream, char* value);
void output_value(std::ostream& stream, const char* value);
void output_value(std::ostream& stream, bool value);

} // namespace detail

inline value::value(formatted_string formatted)
    : formatted{std::move(formatted)}
{}

template <typename T>
value::value(const T& value)
{
    static_assert(!std::is_same<property, T>::value, "A 'value' cannot be constructed from a 'property'");
    static_assert(!std::is_same<prefix_string, T>::value, "A 'value' cannot be constructed from a 'prefix_string'");
    static_assert(!std::is_same<formatted_string, T>::value, "A 'value' cannot be constructed from a 'formatted_string'");
    std::ostringstream stream;
    detail::output_value(stream, value);
    formatted.underlying = stream.str();
}

inline value object(property property)
{
    return value{formatted_string{detail::curly_bracket(property.formatted.underlying)}};
}

inline value object(std::initializer_list<property> properties)
{
    return object(properties.begin(), properties.end());
}

template <typename TIterator>
value object(TIterator first_property, TIterator last_property)
{
    std::string list;
    for (auto it = first_property; it != last_property; ++it)
        list.append(it != first_property ? ", " : "").append(it->formatted.underlying);
    return value{formatted_string{detail::curly_bracket(list)}};
}

template <typename TRange>
value object(const TRange& properties)
{
    return object(std::begin(properties), std::end(properties));
}

inline value array(std::initializer_list<value> values)
{
    return array(values.begin(), values.end());
}

template <typename TIterator>
value array(TIterator first_value, TIterator last_value)
{
    std::string list;
    for (TIterator it = first_value; it != last_value; ++it)
        list.append(it != first_value ? ", " : "").append(value{*it}.formatted.underlying);
    return value{formatted_string{detail::square_bracket(list)}};
}

template <typename TRange>
value array(const TRange& values)
{
    return array(std::begin(values), std::end(values));
}

inline property::property(const std::string& name, const value& value)
{
    formatted.underlying.reserve(name.length() + 2 /* the ": " */ + 2 /* the quotes */ + value.formatted.underlying.length());
    formatted.underlying.append(detail::quote(name)).append(": ").append(value.formatted.underlying);
}

inline prefix_string google_test_prefix()
{
    return prefix_string{"[    STATE ] "};
}

inline output::output(prefix_string prefix)
    : prefix{std::move(prefix)}
{}

inline output::output(const value& value)
{
    *this += value;
}

inline output::output(const property& property)
{
    *this += property;
}

inline output::output(prefix_string prefix, const property& property)
    : prefix{std::move(prefix)}
{
    *this += property;
}

inline output::output(prefix_string prefix, const value& value)
    : prefix{std::move(prefix)}
{
    *this += value;
}

inline output& output::operator+=(const property& property)
{
    formatted.underlying.reserve(formatted.underlying.length() +
                                 (formatted.underlying.empty() ? 0 : 1) /* the '\n' */ +
                                 prefix.underlying.length() +
                                 property.formatted.underlying.length());
    if (!formatted.underlying.empty())
        formatted.underlying.append(1, '\n');
    formatted.underlying.append(prefix.underlying).append(property.formatted.underlying);
    return *this;
}

inline output& output::operator+=(const value& value)
{
    formatted.underlying.reserve(formatted.underlying.length() +
                                 (formatted.underlying.empty() ? 0 : 1) /* the '\n' */ +
                                 prefix.underlying.length() +
                                 value.formatted.underlying.length());
    if (!formatted.underlying.empty())
        formatted.underlying.append(1, '\n');
    formatted.underlying.append(prefix.underlying).append(value.formatted.underlying);
    return *this;
}

namespace detail {

inline std::string surround(const std::string& text, const std::string& left, const std::string& right, const std::string& fill)
{
    std::string surrounded;
    surrounded.reserve(text.length() + (text.empty() ? 0 : (2 * fill.length())) + left.length() + right.length());
    surrounded.append(left);
    if (!text.empty())
        surrounded.append(fill).append(text).append(fill);
    surrounded.append(right);
    return surrounded;
}

inline std::string curly_bracket(const std::string& text)
{
    return surround(text, "{", "}", " ");
}

inline std::string square_bracket(const std::string& text)
{
    return surround(text, "[", "]", " ");
}

inline std::string quote(const std::string& text)
{
    return surround(text, "\"", "\"", "");
}

template <typename T>
void output_value(std::ostream& stream, const T& value)
{
    stream << value;
}

inline void output_value(std::ostream& stream, const std::string& value)
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
           << std::setw(sizeof(void*) * 2) // two hex chars per byte
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

} // namespace detail
} // namespace test_state
} // namespace jg
