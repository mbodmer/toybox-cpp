// http://cpp-netlib.org/0.11.0/techniques/directives.html

// Using the object-oriented notion of message passing, where an object accepts
// a message (usually a function call) we define a simple DSEL in order for the
// protocol to be supported by certain object types. In the cpp-netlib the protocol
// implemented is similar to that of the standard iostream formatting system
//
// object << directive1(...)
//        << directive2(...)
//        ...
//        << directiveN(...);

#include <iostream>
#include <string>

struct Message {
  std::string destination;
  std::string source;
  uint64_t    id;
  Message() : destination("empty"), source("empty"), id(0ULL) {}
};

// In cpp-netlib the directives are simple function objects that take a target
// object as reference and returns a reference to the same object as a result.
// In code the directive pattern looks like the following:
struct destination_directive {
    const std::string& value;
    explicit destination_directive(const std::string& dest) : value(dest) {}
    destination_directive(const destination_directive& other) : value(other.value) {}
    template <class Input> Input& operator()(Input& input) const {
        // do something to Input
        input.destination = value;
        return input;
    }
};

struct source_directive {
    const std::string& value;
    explicit source_directive(const std::string& src) : value(src) {}
    source_directive(const source_directive& other) : value(other.value) {}
    template <class Input> Input& operator()(Input& input) const {
        // do something to Input
        input.source = value;
        return input;
    }
};

struct id_directive {
    const uint64_t& value;
    explicit id_directive(const uint64_t& id) : value(id) {}
    id_directive(const id_directive& other) : value(other.value) {}
    template <class Input> Input& operator()(Input& input) const {
        // do something to Input
        input.id = value;
        return input;
    }
};

// To simplify directive creation, usually factory or generator functions are
// defined to return concrete objects of the directive’s type.
inline const destination_directive destination(const std::string& value) {
    return destination_directive(value);
}

inline const source_directive source(const std::string& value) {
    return source_directive(value);
}

inline const id_directive id(const uint64_t& value) {
    return id_directive(value);
}


// The trivial implementation of the directive protocol then boils down to the
// specialization of the shift-left operator on the target type.
template <class Directive>
inline Message& operator<<(Message& msg, const Directive& directive) {
  return directive(msg);
}


// Encapsulation - by moving logic into the directive types the target object’s
// interface can remain rudimentary and even hidden to the user’s immediate
// attention. Adding this layer of indirection also allows for changing the
// underlying implementations while maintaining the same syntactic and semantic
// properties.
//
// Flexibility - by allowing the creation of directives that are independent
// from the target object’s type, generic operations can be applied based on the
// concept being modeled by the target type. The flexibility also afforded comes
// in the directive’s generator function, which can also generate different
// concrete directive specializations based on parameters to the function.
//
// Extensibility - because the directives are independent of the target object’s
// type, new directives can be added and supported without having to change the
// target object at all.
//
// Reuse - truly generic directives can then be used for a broad set of target
// object types that model the same concepts supported by the directive. Because
// the directives are self-contained objects, the state and other object references
// it keeps are only accessible to it and can be re-used in different contexts as
// well.
//
// Extending a system that uses directives is trivial in header-only systems
// because new directives are simply additive. The protocol is simple and can be
// applied to a broad class of situations. In a header-only library, the static
// nature of the wiring and chaining of the operations lends itself to compiler abuse.
int main() {
    Message msg;
    msg << source("me")
        << destination("you")
        << id(47ULL);

    std::cout << "message.source: " << msg.source << std::endl;
    std::cout << "message.destination: " << msg.destination << std::endl;
    std::cout << "message.id: " << msg.id << std::endl;

    return 0;
}
