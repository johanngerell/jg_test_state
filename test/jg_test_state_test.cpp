#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <jg_test_state.h>

using namespace jg::test_state;

template <typename T>
static std::string to_string(const T& value)
{
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

struct vector2d
{
    int x;
    int y;
};

static std::ostream& operator<<(std::ostream& stream, const vector2d& v)
{
    return stream << "(" << v.x << "," << v.y << ")";
}

struct moving_particle
{
    vector2d position;
    vector2d velocity;
};

static std::ostream& operator<<(std::ostream& stream, const moving_particle& p)
{
    return stream << "pos" << p.position << "," << "vel" << p.velocity;
}

static void test_ctors_simple_value()
{
    {
        output state{vector2d{1,2}};
        assert(to_string(state) == "(1,2)");
    }

    {
        output state{4711};
        assert(to_string(state) == "4711");
    }

    {
        output state{true};
        assert(to_string(state) == "true");
    }

    {
        output state{false};
        assert(to_string(state) == "false");
    }

    {
        output state{"foo"};
        assert(to_string(state) == "\"foo\"");
    }

    {
        output state{std::string{"bar"}};
        assert(to_string(state) == "\"bar\"");
    }

    {
        const float pi = 3.1415926f;
        output state{pi};
        assert(to_string(state).substr(0, 5) == "3.141");
    }

    {
        const double pi = 3.1415926;
        output state{pi};
        assert(to_string(state).substr(0, 5) == "3.141");
    }

    {
        uintptr_t deadbeef = 0x00000000DEADBEEF;
        output state{reinterpret_cast<void*>(deadbeef)};
        assert(to_string(state) == "0x00000000deadbeef");
    }

    {
        const uintptr_t deadbeef = 0x00000000DEADBEEF;
        output state{reinterpret_cast<const void*>(deadbeef)};
        assert(to_string(state) == "0x00000000deadbeef");
    }

    {
        const std::string foo;
        output state{&foo};
        assert(to_string(state).substr(0, 2) == "0x");
        assert(to_string(state).length() == 18);
    }

    {
        const std::string* foo = nullptr;
        output state{foo};
        assert(to_string(state) == "null");
    }
}

static void test_static_ctors_simple_value()
{
    {
        output state(prefix_string{"prefix: "}, vector2d{1,2});
        assert(to_string(state) == "prefix: (1,2)");
    }

    {
        output state(prefix_string{"prefix: "}, 4711);
        assert(to_string(state) == "prefix: 4711");
    }

    {
        output state(prefix_string{"prefix: "}, true);
        assert(to_string(state) == "prefix: true");
    }

    {
        output state(prefix_string{"prefix: "}, false);
        assert(to_string(state) == "prefix: false");
    }

    {
        output state(prefix_string{"prefix: "}, "foo");
        assert(to_string(state) == "prefix: \"foo\"");
    }

    {
        output state(prefix_string{"prefix: "}, std::string{"bar"});
        assert(to_string(state) == "prefix: \"bar\"");
    }

    {
        const float pi = 3.1415926f;
        output state(prefix_string{"prefix: "}, pi);
        assert(to_string(state).substr(0, 13) == "prefix: 3.141");
    }

    {
        const double pi = 3.1415926;
        output state(prefix_string{"prefix: "}, pi);
        assert(to_string(state).substr(0, 13) == "prefix: 3.141");
    }

    {
        uintptr_t deadbeef = 0x00000000DEADBEEF;
        output state(prefix_string{"prefix: "}, reinterpret_cast<void*>(deadbeef));
        assert(to_string(state) == "prefix: 0x00000000deadbeef");
    }

    {
        const uintptr_t deadbeef = 0x00000000DEADBEEF;
        output state(prefix_string{"prefix: "}, reinterpret_cast<const void*>(deadbeef));
        assert(to_string(state) == "prefix: 0x00000000deadbeef");
    }
}

static void test_ctors_complex_value()
{
    {
        output state = array({
            vector2d{1,2},
            vector2d{3,4}
        });
        assert(to_string(state) == "[ (1,2), (3,4) ]");
    }

    {
        output state = object({
            {"p1", vector2d{1,2}},
            {"p2", vector2d{3,4}}
        });
        assert(to_string(state) == R"({ "p1": (1,2), "p2": (3,4) })");
    }

    {
        output state {
            array({
                vector2d{1,2},
                vector2d{3,4}
            })
        };
        assert(to_string(state) == "[ (1,2), (3,4) ]");
    }

    {
        output state {
            object({
                {"p1", vector2d{1,2}},
                {"p2", vector2d{3,4}}
            })
        };
        assert(to_string(state) == R"({ "p1": (1,2), "p2": (3,4) })");
    }

    {
        output state = array({
            true,
            1,
            false,
            "foo"
        });
        assert(to_string(state) == "[ true, 1, false, \"foo\" ]");
    }

    {
        output state = object({
            {"true", true},
            {"1", 1},
            {"false", false},
            {"foo", "foo"}
        });
        assert(to_string(state) == R"({ "true": true, "1": 1, "false": false, "foo": "foo" })");
    }

    {
        output state {
            array({
                true,
                1,
                false,
                "foo"
            })
        };
        assert(to_string(state) == "[ true, 1, false, \"foo\" ]");
    }

    {
        output state {
            object({
                {"true", true},
                {"1", 1},
                {"false", false},
                {"foo", "foo"}
            })
        };
        assert(to_string(state) == R"({ "true": true, "1": 1, "false": false, "foo": "foo" })");
    }

    {
        output state {
            array
            ({
                4711,
                array
                ({
                    4711,
                    object
                    ({
                        { "4711", 4711 },
                        { "4712", 4712 }
                    }),
                    array
                    ({
                        4711, 4712, 4713
                    })
                })
            })
        };
        assert(to_string(state) == R"([ 4711, [ 4711, { "4711": 4711, "4712": 4712 }, [ 4711, 4712, 4713 ] ] ])");
    }
}

static void test_ctors_complex_property()
{
    {
        output state =
            property {
                "points",
                array({
                    vector2d{1,2},
                    vector2d{3,4}
                })
            };
        assert(to_string(state) == "\"points\": [ (1,2), (3,4) ]");
    }

    {
        output state {
            {
                "points",
                array({
                    vector2d{1,2},
                    vector2d{3,4}
                })
            }
        };
        assert(to_string(state) == "\"points\": [ (1,2), (3,4) ]");
    }

    {
        output state {
            {
                "points", object
                ({
                    { "pt1", vector2d{1,2} },
                    { "pt2", vector2d{3,4} }
                })
            }
        };
        assert(to_string(state) == R"("points": { "pt1": (1,2), "pt2": (3,4) })");
    }
}

static void test_ctors_simple_property()
{
    {
        output state{{"name", vector2d{1,2}}};
        assert(to_string(state) == "\"name\": (1,2)");
    }

    {
        output state{{"name", 4711}};
        assert(to_string(state) == "\"name\": 4711");
    }

    {
        output state{{"name", true}};
        assert(to_string(state) == "\"name\": true");
    }

    {
        output state{{"name", false}};
        assert(to_string(state) == "\"name\": false");
    }

    {
        output state{property{"name", "foo"}};
        assert(to_string(state) == "\"name\": \"foo\"");
    }

    {
        output state{{"name", std::string{"bar"}}};
        assert(to_string(state) == "\"name\": \"bar\"");
    }

    {
        const float pi = 3.1415926f;
        output state{{"name", pi}};
        assert(to_string(state).substr(0, 13) == "\"name\": 3.141");
    }

    {
        const double pi = 3.1415926;
        output state{{"name", pi}};
        assert(to_string(state).substr(0, 13) == "\"name\": 3.141");
    }

    {
        uintptr_t deadbeef = 0x00000000DEADBEEF;
        output state{{"name", reinterpret_cast<void*>(deadbeef)}};
        assert(to_string(state) == "\"name\": 0x00000000deadbeef");
    }

    {
        const uintptr_t deadbeef = 0x00000000DEADBEEF;
        output state{{"name", reinterpret_cast<const void*>(deadbeef)}};
        assert(to_string(state) == "\"name\": 0x00000000deadbeef");
    }
}

static void test_prefix()
{
    {
        output state;
        state += 1;
        state += 2;
        state += 3;

        assert(to_string(state) == "1\n2\n3");
    }

    {
        output state{prefix_string{"prefix: "}};
        state += 1;
        state += 2;
        state += 3;

        assert(to_string(state) == "prefix: 1\nprefix: 2\nprefix: 3");
    }

    {
        output state(prefix_string{"prefix: "});
        state += 1;
        state += 2;
        state += 3;

        assert(to_string(state) == "prefix: 1\nprefix: 2\nprefix: 3");
    }

    {
        output state(prefix_string{"prefix: "}, 1);
        assert(to_string(state) == "prefix: 1");
    }

    {
        output state(prefix_string{"prefix: "}, {"one", 1});
        assert(to_string(state) == "prefix: \"one\": 1");
    }

    {
        output state(prefix_string{"prefix: "}, object({{"one", 1}}));
        assert(to_string(state) == "prefix: { \"one\": 1 }");
    }

    {
        output state(prefix_string{"prefix: "}, array({1, 2}));
        assert(to_string(state) == "prefix: [ 1, 2 ]");
    }

    {
        output state(prefix_string{"prefix: "}, array({"two", 2}));
        assert(to_string(state) == "prefix: [ \"two\", 2 ]");
    }

    #define PREFIX "[    STATE ] "

    {
        output state(google_test_prefix());
        state += 1;
        state += 2;
        state += 3;

        assert(to_string(state) == \
            PREFIX "1\n" \
            PREFIX "2\n" \
            PREFIX "3");
    }

    //

    {
        output state(google_test_prefix(), 1);
        assert(to_string(state) == PREFIX "1");
    }

    {
        output state(google_test_prefix(), {"one", 1});
        assert(to_string(state) == PREFIX "\"one\": 1");
    }

    {
        output state(google_test_prefix(), object({{"one", 1}}));
        assert(to_string(state) == PREFIX "{ \"one\": 1 }");
    }

    {
        output state(google_test_prefix(), array({1, 2}));
        assert(to_string(state) == PREFIX "[ 1, 2 ]");
    }

    {
        output state(google_test_prefix(), array({"two", 2}));
        assert(to_string(state) == PREFIX "[ \"two\", 2 ]");
    }

    #undef PREFIX
}

static void test_user_defined_output()
{
    {
        moving_particle particle;
        particle.position = {1, 2};
        particle.velocity = {3, 4};

        output state;
        state += particle;

        assert(to_string(state) == R"(pos(1,2),vel(3,4))");
    }

    {
        moving_particle particle;
        particle.position = {1, 2};
        particle.velocity = {3, 4};

        output state;
        state += { "particle", particle };

        assert(to_string(state) == R"("particle": pos(1,2),vel(3,4))");
    }
}

static void test_user_defined_type()
{
    {
        moving_particle particle;
        particle.position = {1, 2};
        particle.velocity = {3, 4};

        output state = object({{"position", object({{"x", particle.position.x}, {"y", particle.position.y}})},
                               {"velocity", object({{"vx", particle.velocity.x}, {"vy", particle.velocity.y}})}});

        assert(to_string(state) == R"({ "position": { "x": 1, "y": 2 }, "velocity": { "vx": 3, "vy": 4 } })"); 
    }

    {
        moving_particle particle;
        particle.position = {1, 2};
        particle.velocity = {3, 4};

        output state ({ "particle", object({{"position", object({{"x", particle.position.x}, {"y", particle.position.y}})},
                               {"velocity", object({{"vx", particle.velocity.x}, {"vy", particle.velocity.y}})}})});

        assert(to_string(state) == R"("particle": { "position": { "x": 1, "y": 2 }, "velocity": { "vx": 3, "vy": 4 } })"); 
    }

    {
        moving_particle particle;
        particle.position = {1, 2};
        particle.velocity = {3, 4};

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

        assert(to_string(state) == R"("particle": { "position": { "x": 1, "y": 2 }, "velocity": { "vx": 3, "vy": 4 } })");
    }

    {
        moving_particle particle;
        particle.position = {1, 2};
        particle.velocity = {3, 4};

        output state;
        state += { "position", object({
            { "x", particle.position.x },
            { "y", particle.position.y }
        })};
        state += { "velocity", object({
            { "vx", particle.velocity.x },
            { "vy", particle.velocity.y }
        })};

        assert(to_string(state) == R"("position": { "x": 1, "y": 2 }
"velocity": { "vx": 3, "vy": 4 })");
    }
}

static void test_object()
{
    {
        output state;
        state += object({"number", 4711});

        assert(to_string(state) == R"({ "number": 4711 })");
    }

    {
        output state;
        state += object({{"number", 4711}});

        assert(to_string(state) == R"({ "number": 4711 })");
    }

    {
        output state;
        state += object({{"number", 4711}, {"string", "foo"}});

        assert(to_string(state) == R"({ "number": 4711, "string": "foo" })");
    }

    const property nato_array[] {
        {"alpha", 1},
        {"bravo", 2},
        {"charlie", 3}
    };
    const std::vector<property> nato_vector { std::begin(nato_array), std::end(nato_array) };

    {
        output state;
        state += object(nato_array);

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }

    {
        output state = object(nato_array);

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }

    {
        output state;
        state += object(nato_vector);

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }

    {
        output state = object(nato_vector);

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }

    {
        output state = object(std::begin(nato_array), std::end(nato_array));

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }

    {
        output state;
        state += object(std::begin(nato_array), std::end(nato_array));

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }

    {
        output state;
        state += object(nato_vector.begin(), nato_vector.end());

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }

    {
        output state = object(nato_vector.begin(), nato_vector.end());

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }

    {
        output state = object({
            {"alpha", 1},
            {"bravo", 2},
            {"charlie", 3}
        });

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }

    {
        output state;
        state += object({
            {"alpha", 1},
            {"bravo", 2},
            {"charlie", 3}
        });

        assert(to_string(state) == R"({ "alpha": 1, "bravo": 2, "charlie": 3 })");
    }
}

static void test_array()
{
    const std::initializer_list<std::string> nato_list { "alpha", "bravo", "charlie" };
    const std::vector<std::string> nato_vector { nato_list };
    
    {
        const std::string nato_array[] { "alpha", "bravo", "charlie" };

        output state;
        state += array(nato_array);

        assert(to_string(state) == R"([ "alpha", "bravo", "charlie" ])");
    }

    {
        output state;
        state += array(nato_vector);

        assert(to_string(state) == R"([ "alpha", "bravo", "charlie" ])");
    }

    {
        output state;
        state += array(nato_vector.begin(), nato_vector.end());

        assert(to_string(state) == R"([ "alpha", "bravo", "charlie" ])");
    }

    {
        output state;
        state += array(nato_list);

        assert(to_string(state) == R"([ "alpha", "bravo", "charlie" ])");
    }

    {
        output state;
        state += array({1, "1", true});

        assert(to_string(state) == R"([ 1, "1", true ])");
    }

    {
        output state;
        state += array({"alpha", "bravo", "charlie"});

        assert(to_string(state) == R"([ "alpha", "bravo", "charlie" ])");
    }

    {
        output state;
        state += array({});

        assert(to_string(state) == R"([])");
    }

    {
        output state;
        std::initializer_list<int> empty;
        state += array(empty);

        assert(to_string(state) == R"([])");
    }

    {
        output state;
        state += array(std::vector<int>{});

        assert(to_string(state) == R"([])");
    }

    {
        output state;
        state += array(nato_vector.begin(), nato_vector.begin());

        assert(to_string(state) == R"([])");
    }

    {
        const size_t sizes[3] { 1, 2, 3 };

        output state;
        state += array(std::begin(sizes), std::end(sizes));

        assert(to_string(state) == R"([ 1, 2, 3 ])");
    }

    {
        const size_t sizes[3] { 1, 2, 3 };

        output state;
        state += array(sizes, sizes + 3);

        assert(to_string(state) == R"([ 1, 2, 3 ])");
    }

    {
        const size_t sizes[3] { 1, 2, 3 };

        output state;
        state += { "sizes", array(sizes, sizes + 3) };

        assert(to_string(state) == R"("sizes": [ 1, 2, 3 ])");
    }
}

static void test_value()
{
    {
        value v1 = true;
        assert(to_string(v1) == "true");

        value v2 = array({true});
        assert(to_string(v2) == "[ true ]");
    }

    {
        value v1 = false;
        assert(to_string(v1) == "false");

        value v2 = array({false});
        assert(to_string(v2) == "[ false ]");
    }

    {
        uintptr_t dummy_address = 0x00000000deadbeef;
        void* dummy_pointer = reinterpret_cast<void*>(dummy_address);

        value v1 = dummy_pointer;
        assert(to_string(v1) == "0x00000000deadbeef");

        value v2 = array({dummy_pointer});
        assert(to_string(v2) == "[ 0x00000000deadbeef ]");
    }

    {
        const uintptr_t dummy_address = 0x00000000deadbeef;
        const void* dummy_pointer = reinterpret_cast<void*>(dummy_address);

        value v1 = dummy_pointer;
        assert(to_string(v1) == "0x00000000deadbeef");

        value v2 = array({dummy_pointer});
        assert(to_string(v2) == "[ 0x00000000deadbeef ]");
    }

    {
        const std::string foo;

        value v1 = &foo;
        assert(to_string(v1).substr(0, 2) == "0x");
        assert(to_string(v1).length() == 18);

        const std::string* bar = nullptr;

        value v2 = bar;
        assert(to_string(v2) == "null");

        value v3 = nullptr;
        assert(to_string(v3) == "null");
    }

    {
        value v1 = "foobar";
        assert(to_string(v1) == R"("foobar")");

        value v2 = array({"foobar"});
        assert(to_string(v2) == R"([ "foobar" ])");
    }

    {
        value v1 = std::string{"foobar"};
        assert(to_string(v1) == R"("foobar")");

        value v2 = array({std::string{"foobar"}});
        assert(to_string(v2) == R"([ "foobar" ])");
    }

    {
        value v1 = 4711;
        assert(to_string(v1) == "4711");

        value v2 = array({4711});
        assert(to_string(v2) == "[ 4711 ]");
    }

    {
        value v1 = array({4711, 4712, 4713});
        assert(to_string(v1) == "[ 4711, 4712, 4713 ]");
    }

    {
        std::vector<int> ints { 4711, 4712, 4713 };

        value v1(array(ints));
        assert(to_string(v1) == "[ 4711, 4712, 4713 ]");

        value v2 = array(ints);
        assert(to_string(v2) == "[ 4711, 4712, 4713 ]");

        value v3(array(ints.begin(), ints.end()));
        assert(to_string(v3) == "[ 4711, 4712, 4713 ]");

        value v4 = array(ints.begin(), ints.end());
        assert(to_string(v4) == "[ 4711, 4712, 4713 ]");
    }

    {
        std::vector<int> ints;

        value v1(array(ints));
        assert(to_string(v1) == "[]");

        value v2 = array(ints);
        assert(to_string(v2) == "[]");

        value v3(array(ints.begin(), ints.end()));
        assert(to_string(v3) == "[]");

        value v4 = array(ints.begin(), ints.end());
        assert(to_string(v4) == "[]");
    }
}

static void test_property()
{
    {
        property p1("p1", true);
        assert(to_string(p1) == R"("p1": true)");

        property p3{"p3", true};
        assert(to_string(p3) == R"("p3": true)");
    }

    {
        property p2("p2", array({true}));
        assert(to_string(p2) == R"("p2": [ true ])");

        property p4{"p4", array({true})};
        assert(to_string(p4) == R"("p4": [ true ])");
    }

    {
        property p1("p1", false);
        assert(to_string(p1) == R"("p1": false)");

        property p3{"p3", false};
        assert(to_string(p3) == R"("p3": false)");
    }

    {
        property p2("p2", array({false}));
        assert(to_string(p2) == R"("p2": [ false ])");

        property p4{"p4", array({false})};
        assert(to_string(p4) == R"("p4": [ false ])");
    }

    {
        uintptr_t dummy_address = 0x00000000deadbeef;
        void* dummy_pointer = reinterpret_cast<void*>(dummy_address);

        property p1("p1", dummy_pointer);
        assert(to_string(p1) == R"("p1": 0x00000000deadbeef)");

        property p3{"p3", dummy_pointer};
        assert(to_string(p3) == R"("p3": 0x00000000deadbeef)");
    }

    {
        uintptr_t dummy_address = 0x00000000deadbeef;
        void* dummy_pointer = reinterpret_cast<void*>(dummy_address);

        property p2("p2", array({dummy_pointer}));
        assert(to_string(p2) == R"("p2": [ 0x00000000deadbeef ])");

        property p4{"p4", array({dummy_pointer})};
        assert(to_string(p4) == R"("p4": [ 0x00000000deadbeef ])");
    }

    {
        const uintptr_t dummy_address = 0x00000000deadbeef;
        const void* dummy_pointer = reinterpret_cast<void*>(dummy_address);

        property p1("p1", dummy_pointer);
        assert(to_string(p1) == R"("p1": 0x00000000deadbeef)");

        property p3{"p3", dummy_pointer};
        assert(to_string(p3) == R"("p3": 0x00000000deadbeef)");
    }

    {
        {
            const std::string foo;
            
            property p1("p1", &foo);
            assert(to_string(p1).substr(0, 8) == R"("p1": 0x)");
            assert(to_string(p1).length() == 24);

            property p2{"p2", &foo};
            assert(to_string(p2).substr(0, 8) == R"("p2": 0x)");
            assert(to_string(p2).length() == 24);
        }

        {
            const std::string* foo = nullptr;

            property p1("p1", foo);
            assert(to_string(p1) == R"("p1": null)");

            property p2{"p2", foo};
            assert(to_string(p2) == R"("p2": null)");
        }
    }

    {
        const uintptr_t dummy_address = 0x00000000deadbeef;
        const void* dummy_pointer = reinterpret_cast<void*>(dummy_address);

        property p2("p2", array({dummy_pointer}));
        assert(to_string(p2) == R"("p2": [ 0x00000000deadbeef ])");

        property p4{"p4", array({dummy_pointer})};
        assert(to_string(p4) == R"("p4": [ 0x00000000deadbeef ])");
    }

    {
        property p1("p1", "foobar");
        assert(to_string(p1) == R"("p1": "foobar")");

        property p3{"p3", "foobar"};
        assert(to_string(p3) == R"("p3": "foobar")");
    }

    {
        property p2("p2", array({"foobar"}));
        assert(to_string(p2) == R"("p2": [ "foobar" ])");

        property p4{"p4", array({"foobar"})};
        assert(to_string(p4) == R"("p4": [ "foobar" ])");
    }

    {
        property p1("p1", std::string{"foobar"});
        assert(to_string(p1) == R"("p1": "foobar")");

        property p3{"p3", std::string{"foobar"}};
        assert(to_string(p3) == R"("p3": "foobar")");
    }

    {
        property p2("p2", array({std::string{"foobar"}}));
        assert(to_string(p2) == R"("p2": [ "foobar" ])");

        property p4{"p4", array({std::string{"foobar"}})};
        assert(to_string(p4) == R"("p4": [ "foobar" ])");
    }

    {
        property p1("p1", 4711);
        assert(to_string(p1) == R"("p1": 4711)");

        property p3{"p3", 4711};
        assert(to_string(p3) == R"("p3": 4711)");
    }

    {
        property p2("p2", array({4711}));
        assert(to_string(p2) == R"("p2": [ 4711 ])");

        property p4{"p4", array({4711})};
        assert(to_string(p4) == R"("p4": [ 4711 ])");
    }

    //---------------------------

    {
        property p1("p1", array({4711, 4712, 4713}));
        assert(to_string(p1) == R"("p1": [ 4711, 4712, 4713 ])");

        property p2{"p2", array({4711, 4712, 4713})};
        assert(to_string(p2) == R"("p2": [ 4711, 4712, 4713 ])");

        property p3("p3", array({4711, "4712", 4713, true}));
        assert(to_string(p3) == R"("p3": [ 4711, "4712", 4713, true ])");

        property p4{"p4", array({4711, "4712", 4713, true})};
        assert(to_string(p4) == R"("p4": [ 4711, "4712", 4713, true ])");

        property p5 = {"p5", array({4711, "4712", 4713, true})};
        assert(to_string(p5) == R"("p5": [ 4711, "4712", 4713, true ])");

        property p7 = {"p7", array({4711, "4712", array({false, 0, "", (void*)0}), true})};
        assert(to_string(p7) == R"("p7": [ 4711, "4712", [ false, 0, "", null ], true ])");

        property p8 = {"p8", array({4711, "4712", array({false, 0, "", (const void*)0}), true})};
        assert(to_string(p8) == R"("p8": [ 4711, "4712", [ false, 0, "", null ], true ])");

        property p9 = {"p9", array({4711, "4712", array({false, 0, "", (void*)0}), true})};
        assert(to_string(p9) == R"("p9": [ 4711, "4712", [ false, 0, "", null ], true ])");

        property p10 = {"p10", array({4711, "4712", array({false, 0, "", (const void*)0}), true})};
        assert(to_string(p10) == R"("p10": [ 4711, "4712", [ false, 0, "", null ], true ])");

        property p11 = {"p11", array({4711, "4712", array({false, 0, "", nullptr}), true})};
        assert(to_string(p11) == R"("p11": [ 4711, "4712", [ false, 0, "", null ], true ])");
    }
}

int main()
{
    test_value();
    test_property();

    test_user_defined_type();
    test_user_defined_output();

    test_object();
    test_array();
    test_prefix();

    test_ctors_simple_value();
    test_static_ctors_simple_value();
    test_ctors_complex_value();
    test_ctors_simple_property();
    test_ctors_complex_property();
}
