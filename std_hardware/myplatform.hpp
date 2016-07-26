// tr18015 "Middle Layer", platform abstraction

#include <hardware>
namespace
{
// Middle layer (hardware register designator specifications)
using namespace std::hardware;
struct PlatformA : platform_traits
{
 typedef static_address<0x50> address_holder;
};
struct PlatformB : platform_traits
{
 typedef static_address<0x90> address_holder;
};
struct DynPlatform : platform_traits
{
 typedef dynamic_address address_holder;
 enum { address_mode=hw_base::dynamic_address };
};
struct PortA1_T : register_traits
{
 typedef static_address<0x1a> address_holder;
};
struct PortA2_T : register_traits
{
 typedef static_address<0x20> address_holder;
};
// Portable device driver function using the template approach:
template <class PlatformSpec>
uint8_t getDevData(typename PlatformSpec::address_holder const &addr =
typename PlatformSpec::address_holder())
{
 register_access<PortA1_T, PlatformSpec> devConfig(addr);
 register_access<PortA2_T, PlatformSpec> devData(addr);
 devConfig = 0x33;
 return devData;
}
} // unnamed namespace

int main()
{
// static version
 // Read data from device 1:
 uint8_t d1 = getDevData<PlatformA>();
 // Read data from device 2:
 uint8_t d2 = getDevData<PlatformB>();
// dynamic version
 uint8_t d3 = getDevData<DynPlatform>(0x40);
 uint8_t d4 = getDevData<DynPlatform>(0x80);

 return 0;
}
