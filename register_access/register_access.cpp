// https://yogiken.files.wordpress.com/2010/02/c-register-access.pdf

#include <cstdint>
#include <iostream>

constexpr std::uint32_t cr_base(0xfffe0000);
constexpr std::uint32_t mr_base(0xfffe0004);
constexpr std::uint32_t sr_base(0xfffe0008);
constexpr std::uint32_t reset_base(0xfffe0080);

template<unsigned long address, unsigned mask, unsigned offset, typename mutability_policy>
struct reg_t {
  static void write(unsigned value) {
    mutability_policy::write(
      reinterpret_cast<volatile unsigned*>(address), mask, offset, value);
  }
  static unsigned read() {
    return mutability_policy::read(
      reinterpret_cast<volatile unsigned*>(address), mask, offset);
  }
};

/** writeonly policy */
struct wo_t {
  static void write(volatile unsigned* reg, unsigned mask, unsigned offset, unsigned value) {
    *reg = (value & mask) << offset;
  }
};

/** readonly policy */
struct ro_t {
  static unsigned read(volatile unsigned* reg, unsigned mask, unsigned offset) {
    return (*reg >> offset) & mask;
  }
};

/** read/write policy */
struct rw_t : public ro_t {
  static void write(volatile unsigned* reg, unsigned mask, unsigned offset, unsigned value) {
    *reg = (*reg & ~(mask << offset)) | ((value & mask) << offset);
  }
};

/** password protected writeonly policy */
template<unsigned key_mask, unsigned key_offset, unsigned key_value>
struct keyed_wo_t {
  static void write(volatile unsigned* reg, unsigned mask, unsigned offset, unsigned value) {
    volatile unsigned tmp = (value & mask) << offset;
    tmp &= ~(key_mask << key_offset);
    tmp |= (key_value & key_mask) << key_offset;
    *reg = tmp;
  }
};

/** readonly test policy */
template<unsigned initialized_to>
struct soft_ro_t {
  static unsigned read(unsigned volatile* reg, unsigned mask, unsigned offset) {
    (void) reg;
    unsigned volatile soft_register = initialized_to;
    return ro_t::read(&soft_register, mask, offset);
  }
};


namespace hw {
  namespace cr {
    typedef reg_t<cr_base, 0x1, 0, wo_t> enable;
    typedef reg_t<cr_base, 0x1, 1, wo_t> disable;
  }
  namespace mr {
    typedef reg_t<mr_base, 0xff, 0, rw_t> clockdiv;
    typedef reg_t<mr_base, 0xf, 8, rw_t> delay;
  }
  namespace sr {
    typedef reg_t<sr_base, 0x1, 0, ro_t> enable;
  }
  namespace rst {
    typedef reg_t<reset_base, 0x1, 0, keyed_wo_t<0xff, 24, 0xac> > reset;
  }
}

// host executable example

/** readonly test */
template<unsigned mask, unsigned offset>
struct ro_test_t {
  static void run() {
    typedef reg_t<0, mask, offset, soft_ro_t<mask << offset> > on;
    std::cout << on::read() << " : " << mask << std::endl;
    typedef reg_t<0, mask, offset, soft_ro_t<~(mask << offset)> > off;
    std::cout << off::read() << std::endl;
  }
};

/** test generator */

template<unsigned mask>
struct num_shifts_t {
  static const unsigned value = 1 + num_shifts_t<(mask << 1) | 1>::value;
};
template<>
struct num_shifts_t<0xffffffff> {
  static const unsigned value = 0;
};

template<template <unsigned, unsigned> class test, unsigned mask, unsigned offset>
struct generate_offsets_t {
  static void run() {
    test<mask, offset>::run();
    generate_offsets_t<test, mask, offset-1>::run();
  }
};
template<template <unsigned, unsigned> class test, unsigned mask>
struct generate_offsets_t<test, mask, 0> {
  static void run() {
    test<mask, 0>::run();
  }
};

template<template <unsigned, unsigned> class test, unsigned mask>
struct generate_masks_t {
  static void run() {
    generate_offsets_t<test, mask, num_shifts_t<mask>::value>::run();
    generate_masks_t<test, (mask << 1) | 1>::run();
  }
};
template<template <unsigned, unsigned> class test>
struct generate_masks_t<test, 0xffffffff> {
  static void run() {
    generate_offsets_t<test, 0xffffffff, num_shifts_t<0xffffffff>::value>::run();
  }
};

template<template <unsigned, unsigned> class test>
struct generate_tests_t {
  static void run() {
    generate_masks_t<test, 0x1>::run();
  }
};


int main() {
  generate_tests_t<ro_test_t>::run();
}
