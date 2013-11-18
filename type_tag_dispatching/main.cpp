// http://barendgehrels.blogspot.ch/2010/10/tag-dispatching-by-type-tag-dispatching.html

#include <iostream>

struct apple_tag {};
struct banana_tag {};
struct orange_tag {};


struct apple {
    double radius;
    std::string name;

    apple(std::string const& n)
        : name(n) {
    }
};

struct banana {
    double length;
    std::string name;

    banana(std::string const& n)
        : name(n) {
    }
};

template <typename T> struct tag {};
template <> struct tag<apple> { typedef apple_tag type; };
template <> struct tag<banana> { typedef banana_tag type; };


namespace dispatch {
    template <typename Tag> struct eat {};

    template<>
    struct eat<apple_tag> {
        typedef apple_tag type;
        static void apply(const apple& a) {
          std::cout << "bite the " << a.name << std::endl;
        }
    };

    template<>
    struct eat<banana_tag> {
        typedef banana_tag type;
        static void apply(const banana& b) {
          std::cout << "peel the " << b.name << std::endl;
        }
    };

    template <typename Tag> struct spherical {};

    template <> struct spherical<apple_tag> {
        static const bool value = true;
    };

    template <> struct spherical<banana_tag> {
        static const bool value = false;
    };
}

template <typename T>
void eat(const T& fruit) {
    dispatch::eat<typename tag<T>::type>::apply(fruit);
}

template <typename T>
struct spherical {
    static const bool value = dispatch::spherical<typename tag<T>::type>::value;
};


int main() {
    apple a("my apple");
    banana b("my banana");

    eat(a);
    eat(b);

    std::cout << "is apple spherical: " << spherical<apple>::value << std::endl;
    std::cout << "is banana spherical: " << spherical<banana>::value << std::endl;

    return 0;
}

