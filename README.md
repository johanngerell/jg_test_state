# The jg::test_state library

A simple C++ header library that makes it easy to record/collect runtime state in a key/value/property bag for debugging purposes (for example in tests) and output it nicely formatted in a JSON-ish way when tests run or just when they fail.

The major benefits of using `jg::test_state` compared to manually formatting some runtime state for output (to a `std::ostream` or `FILE*` instance like `std::cout` or a `stdout`) are that `jg::test_state`

  - makes it easy to output structured data nicely formatted,
  - yields reusable objects that can be streamed repeatedly.

The latter is good in test cases with many test assertions or expectations:

```c++
using namespace jg::test_state;
output state_if_failed = ...;
...
EXPECT_TRUE(condition) << state_if_failed;
EXPECT_NE(ptr, nullptr) << state_if_failed;
EXPECT_EQ(value, "foobar") << state_if_failed;
```

The major differences between `jg::test_state` and regular JSON output utility libraries is that

  - No intermediate JSON object model tree is being built. Instead, values are immediately streamed out to an internal string storage as they are being added to the output.
  - Each addition to the output can be seen as a separate JSON document in the sense that it can be arbitarily complex, from a simple value to a nested JSON-like tree.

## Overview

Test state output is heavily focused on a formatting style matching that of JSON, which is focused on the concept of a *value*.

The JSON specification limits what a *value* can be comprised of, but `jg::test_state` doesn't have that limitation. If state data can be output to a `std::ostream`-derived instance -- like `std::cout` -- then it's a value in the eyes of `jg::test_state`.

This is the very simple yet highly recursive JSON-like description of the `jg::test_state` output:

  - A *value* is anything that can be output to a `std::ostream`-derived instance:

    ```json
    value
    ```
  - A *property* is comprised of a name and a value:

    ```json
    "name": value
    ```
  - An *object* is a value that is comprised of zero or more properties:

    ```json
    { "name1": value1, "name2": value2, ... }
    ```
  - An *array* is a value that is comprised of zero or more values:

    ```json
    [ value1, value2, ... ]
    ```
In the general case, a value is output according to the stream output operator `operator<<(std::ostream&,...)` for its underlying type. A few special cases get additional treatment though:

  - A string is output enclosed in double-quotes (these are considered strings: `std::string`, `const char*` and `char*`). This is the same as for JSON, but it's not what the stream output operator does by default.
  - A boolean is output as `true` or `false`.
  - A non-null pointer (not `const char*` and `char*`, as they are considered strings) is output as a 64-bit zero-padded "0x"-prefixed hexadecimal number, and a null pointer (`nullptr` or 0) is output as `null`.

This means that a user-defined type can be recorded as test state as long as the user has defined the stream output operator for it.

## Usage

There are three classes to use:

  - `jg::test_state::value`
  - `jg::test_state::property`
  - `jg::test_state::output`

And two helper functions (with overloads) that create specific *kinds* of `jg::test_state::value` instances -- like JSON:

  - `jg::test_state::object(...)`
  - `jg::test_state::array(...)`

The workhorse here is `jg::test_state::value`, or just `value` going forward. It outputs the actual state data to its underlying string storage. In contrast to a regular JSON value, the state data can be of any type that can be output to `std::ostream`, and `value` formats the output according to the state data type. For instance, strings are enclosed in double quotes, and boolean data becomes `true` or `false`. State data of user-defined types are output using `operator<<(std::ostream&, ...)`, which must be supplied by the user.

The `jg::test_state::property`, or just `property` going forward, is just a data pair with a name and a `value`, where the name is output enclosed in double quotes and `: ` separates the name and the `value` -- like JSON.

To output structured or sequence data in a JSON-ish format, the functions `jg::test_state::object(...)` and `jg::test_state::array(...)`, or `object(...)` and `array(...)` going forward, are used. The output of `object(...)` is a comma-separated list of *properties* enclosed in `{ ... }`, and the output of `array(...)` is a comma-separated list of *values* enclosed in `[ ... ]`. Array values are not necessarily of a homogenous type -- like JSON.

Finally, `jg::test_state::output`, or just `output` going forward, facilitates grouping and formatting state data as simple values, properties, objects, arrays, etc. Each addition of a `value` or a `property` via the `output` constructor or `output::operator+=` is formatted on a separate line with an optional prefix to fit the general output from test frameworks, etc. An added value can be arbitrarily complex by virtue of nesting `object` and `array`. 

### Adding simple values

Typical usage:

```cpp
using namespace jg::test_state;

output state{prefix_string{"prefix: "}};
state += 4711;
state += "foo";
state += std::isnan(0.0);
state += reinterpret_cast<void*>(&std::cout);

std::cout << state;
```

Output:

    prefix: 4711
    prefix: "foo"
    prefix: false
    prefix: 0x00000000deadbeef

The `output` constructor takes an optional prefix parameter ("prefix: " in the example above -- by default an empty string) that starts each output line. This is useful with test frameworks that output lines with a specific and consistent line start format.

For instance, Google Test uses "[. . . . .] " with labels between the brackets, like "[    OK    ] ". Using `output` with Google Test in a way that aligns state output with the rest of the test output is as easy as instantiating `output` with `google_test_prefix()`:

```cpp
using namespace jg::test_state;

// In a test case with a succeeding EXPECT_...
output state{google_test_prefix()};
state += 4711;
state += "foo";

std::cout << state;

EXPECT_TRUE(condition);
```

Output:

    [ RUN      ] TestName.TestCaseName
    [    STATE ] 4711
    [    STATE ] "foo"
    [       OK ] TestName.TestCaseName (12 ms)

Only outputting state when an EXPECT_... fails:

```cpp
using namespace jg::test_state;

// In a test case with a failing EXPECT_...
output state{google_test_prefix};
state += 4711;
state += "foo";

EXPECT_TRUE(condition) << state;
EXPECT_EG(expected, actual) << state; // fails
```

Output:

    [ RUN      ] TestName.TestCaseName
    [    STATE ] 4711
    [    STATE ] "foo"
       <... additional expected vs actual output>
    [  FAILED  ] TestName.TestCaseName (12 ms)

Or as an object without an intermediate `output` instance:

```cpp
using namespace jg::test_state;

EXPECT_TRUE(condition) << output(google_test_prefix(),
    object
    ({
        { "int", 4711 },
        { "string", "foo" }
    })
);
```

Output:

    [ RUN      ] TestName.TestCaseName
    [    STATE ] { "int": 4711, "string": "foo" }
       <... additional expected vs actual output>
    [  FAILED  ] TestName.TestCaseName (12 ms)

Or as an array without an intermediate `output` instance:

```cpp
using namespace jg::test_state;

EXPECT_TRUE(condition) << output(google_test_prefix(),
    array({ 4711, "foo" })
);
```

Output:

    [ RUN      ] TestName.TestCaseName
    [    STATE ] [ 4711, "foo" ]
       <... additional expected vs actual output>
    [  FAILED  ] TestName.TestCaseName (12 ms)

There are two possible operations on `output`:

  - Adding a value (the examples above).
  - Adding a property.

Each of those operations adds a separate output line.

### Adding properties

Typical usage:

```cpp
using namespace jg::test_state;

output state;
state += {"number" , 4711};
state += {"text"   , "foo"};
state += {"boolean", std::isnan(0.0)};
state += {"pointer", (void*)&std::cout};

std::cout << state;
```

Output:

    "number": 4711
    "text": "foo"
    "boolean": false
    "pointer": 0x00000000deadbeef

### Adding user-defined data

As an example, the typical output of state data of user-defined position and velocity types can look like this:

```c++
moving_particle particle;
particle.position = {1, 2};
particle.velocity = {3, 4};

using namespace jg::test_state;

output state;
state += { "position", object({{ "x", particle.position.x }, { "y", particle.position.y }})};
state += { "velocity", object({{ "vx", particle.velocity.x }, { "vy", particle.velocity.y }})};

std::cout << state;
```

Output:

    "position": { "x": 1, "y": 2 }
    "velocity": { "vx": 3, "vy": 4 }

By using `object(...)` with the `output` constructor, that same state can be output on a single line:

```c++
using namespace jg::test_state;

output state({ "particle",
    object({
        { "position", object({{ "x", particle.position.x }, { "y", particle.position.y }}) },
        { "velocity", object({{ "vx", particle.velocity.x }, { "vy", particle.velocity.y }}) }
    })
});

std::cout << state;
```

Output:

    "particle": { "position": { "x": 1, "y": 2 }, "velocity": { "vx": 3, "vy": 4 } }

#### User-defined data details

To add user-defined data of type `foo` to `output`, the function `operator<<(std::ostream&, const foo&)` must be implemented.

For example, let's say a user has a `point2d` structure and they want to output state data of that type to find issues with a unit test that operates on it:

```cpp
struct point2d
{
    int x;
    int y;
};
```

This type is unknown to the compiler and to `value`, so it cannot be automatically output. By implementing the stream output operator for it, the user can decide how to format its output when it's added to an `output` instance:

```cpp
std::ostream& operator<<(std::ostream& stream, const point2d& point)
{
    return stream << "(" << point.x << "," << point.y << ")"; // output as (x,y)
}
```

A `point2d` can now be added to `output`:

```cpp
using namespace jg::test_state;

const point2d point = get_that_point();

output state;
state += {"that_point" , point};

std::cout << state;
```

Output:

    "that_point": (57,311)

### Adding objects

An object is ideal to output structured state data -- think of a conventional data `struct` with or without nested `struct` members -- that doesn't have its own stream output operator (see [Adding user-defined data](#adding-user-defined-data)).

For example, let's say we have a moving particle (or, more like a dot on a pixel grid, since we use `int` and not floating point values), described by its position and velocity:

```cpp
struct vector2d
{
    int x;
    int y;
};

struct moving_particle
{
    vector2d position;
    vector2d velocity;
};

moving_particle particle;
```

We want to output its current state during testing to understand why our move logic malfunctions:

```cpp
using namespace jg::test_state;

output state;
state += { "position", object({
    { "x", particle.position.x },
    { "y", particle.position.y }
})};
state += { "velocity", object({
    { "vx", particle.velocity.x },
    { "vy", particle.velocity.y }
})};

std::cout << state;
```

Output on two lines since two `+=` was used:

    "position": { "x": 1, "y": 2 }
    "velocity": { "vx": 3, "vy": 4 }

If we want to output the position and velocity as one object on one line, we would do this instead:

```cpp
using namespace jg::test_state;

output state;
state +=
{ "particle", object({
    { "position", object({
        { "x", particle.position.x },
        { "y", particle.position.y }
    })},
    { "velocity", object({
        { "vx", particle.velocity.x },
        { "vy", particle.velocity.y }
    })}
})};

std::cout << state;
```

Or if we have no need for the explicit `output` instance:

```cpp
using namespace jg::test_state;

std::cout << output
{ "particle", object({
    { "position", object({
        { "x", particle.position.x },
        { "y", particle.position.y }
    })},
    { "velocity", object({
        { "vx", particle.velocity.x },
        { "vy", particle.velocity.y }
    })}
})};
```

The output is on one line since one `+=` was used, but linebreaks are added below for exposition purposes:

    "particle":
    {
        "position": { "x": 1, "y": 2 },
        "velocity": { "vx": 3, "vy": 4 }
    }

### Adding arrays

An *array* is ideal to output ranges, views, or collections of *homogeneous* (one type) data, for example `std::vector` and anything else with `begin()` and `end()` functions that access their iterators. However, collections of *heterogeneous* (different types) data are fully supported too (as in JSON).

For example, let's say we have the NATO Phonetic Alphabet stored in a vector of strings, and we need to output it during testing:

```cpp
const std::vector<std::string> nato { "alpha", "bravo", "charlie" };

using namespace jg::test_state;

output state;
state += array(nato);
state += array(nato.begin(), nato.end());
state += array({ "alpha", "bravo", "charlie" });

std::cout << state;
```

Output:

    [ "alpha", "bravo", "charlie" ]
    [ "alpha", "bravo", "charlie" ]
    [ "alpha", "bravo", "charlie" ]

Plain C++ arrays can be output with `std::begin` and `std::end`:

```cpp
const size_t sizes[3] { 1, 2, 3 };

using namespace jg::test_state;

output state;
state += array(std::begin(sizes), std::end(sizes));

std::cout << state;
```

Output:

    [ 1, 2, 3 ]

If the plain C++ array is only known by pointer and element count, then do this:

```cpp
const size_t sizes[3] { 1, 2, 3 };

using namespace jg::test_state;

output state;
state += array(sizes, sizes + 3);

std::cout << state;
```

Output:

    [ 1, 2, 3 ]

In the same way as with objects, the `value` of a `property` can be an array:

```cpp
const size_t sizes[3] { 1, 2, 3 };

using namespace jg::test_state;

output state;
state += { "sizes", array(sizes, sizes + 3) };

std::cout << state;
```

Output:

    "sizes": [ 1, 2, 3 ]

## JSON divergences

  - Pointer values are output as hexadecimal values prefixed with "0x", but JSON doesn't support numbers in hexadecimal format.
  - Floating point values are output according to the locale used by the built-in stream output for them, which means that the formatting will not be JSON compliant if the used locale formats the floating point decimals as a comma instead of a period.
  - If a user-defined stream output operator is used for some state data, the output is only JSON compliant if the user made it so.
