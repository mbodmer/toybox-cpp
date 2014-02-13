// http://cpp-netlib.org/0.11.0/techniques/tag_metafunctions.html

#include <iostream>
#include <string>


struct stdstr_tag;
struct charptr_tag;

template <class Tag>
struct string {
    typedef void type;
};

template <>
struct string<stdstr_tag> {
    typedef std::string type;
};

template <>
struct string<charptr_tag> {
    typedef const char* type;
};

int main() {
    string<stdstr_tag>::type  str1 = "str1";
    string<charptr_tag>::type str2 = "str2";

    std::cout << str1 << " " << str2 << std::endl;

    return 0;
}

