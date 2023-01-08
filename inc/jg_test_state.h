#pragma once

#include <ostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <initializer_list>

namespace jg {
namespace test_state {

namespace detail {

/// Default validation policy for a `strong_type` instance. The default behavior is to do no validation,
/// which is why the body of `validate(const T& value)` is empty. If invariants or semantics must hold at
/// construction, then create a policy that asserts or throws an exception in `validate(const T& value)`
/// if they don't hold for `value`. This default empty policy implementation will be optimized away by all
/// compilers in any optimized build.
struct strong_type_no_validation final
{
    template <typename T>
    static void validate(const T&) {}
};

/// A trivial "strong type" to prevent parameters that semantically aren't test state values from being
/// resolved as such due to the `value` class "auto resolve" template constructor being implicit.
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

    prefix_string prefix;
    formatted_string formatted;
};

inline std::ostream& operator<<(std::ostream& stream, const output& output)
{
    return stream << output.formatted.underlying;
}

output& operator+=(output& output, const property& property);
output& operator+=(output& output, const value& value);

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
template <typename T>
void output_value(std::ostream& stream, T* value);
template <typename T>
void output_value(std::ostream& stream, const T* value);
void output_value(std::ostream& stream, std::nullptr_t);
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
    for (auto it = first_property; it != last_property; ++it) {
        if (it != first_property)
            list += ", ";
        list += it->formatted.underlying;
    }
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
    for (TIterator it = first_value; it != last_value; ++it) {
        if (it != first_value)
            list += ", ";
        list += value{*it}.formatted.underlying;
    }
    return value{formatted_string{detail::square_bracket(list)}};
}

template <typename TRange>
value array(const TRange& values)
{
    return array(std::begin(values), std::end(values));
}

inline property::property(const std::string& name, const value& value)
{
    formatted.underlying += detail::quote(name);
    formatted.underlying += ": ";
    formatted.underlying += value.formatted.underlying;
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

inline output& operator+=(output& output, const property& property)
{
    output.formatted.underlying += !output.formatted.underlying.empty() ? "\n" : "";
    output.formatted.underlying += output.prefix.underlying;
    output.formatted.underlying += property.formatted.underlying;
    return output;
}

inline output& operator+=(output& output, const value& value)
{
    output.formatted.underlying += !output.formatted.underlying.empty() ? "\n" : "";
    output.formatted.underlying += output.prefix.underlying;
    output.formatted.underlying += value.formatted.underlying;
    return output;
}

namespace detail {

inline std::string surround(const std::string& text, const std::string& left, const std::string& right, const std::string& fill)
{
    std::string surrounded{left};
    if (!text.empty()) {
        surrounded += fill;
        surrounded += text;
        surrounded += fill;
    }
    surrounded += right;
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

template <typename T>
inline void output_value(std::ostream& stream, T* value)
{
    output_value(stream, const_cast<const T*>(value));
}

template <typename T>
inline void output_value(std::ostream& stream, const T* value)
{
    if (value)
        stream << "0x"
               << std::hex
               << std::setw(sizeof(T*) * 2) // two hex chars per byte
               << std::setfill('0')
               << reinterpret_cast<std::uintptr_t>(value);
    else
        stream << "null";
}

inline void output_value(std::ostream& stream, std::nullptr_t)
{
    stream << "null";
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
