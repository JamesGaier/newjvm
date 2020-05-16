#ifndef VM_UTILS_H_
#define VM_UTILS_H_
#include<stdint.h>
#include<array>
// typedefs
using i64 = int64_t;
using u64 = uint64_t;
using i32 = int32_t;
using u32 = uint32_t;
using i16 = int16_t;
using u16 = uint16_t;
using i8 = int8_t;
using u8 = uint8_t;

constexpr auto page_size_bytes = 4096;
constexpr auto pc_start = 0x80000000;
constexpr auto data_start = 0x8C000000;
constexpr auto top_stack = 0x300000000;
using page = std::array<u8, page_size_bytes>;
constexpr auto opcode_offset = 54;
constexpr auto data_mask = 0x3fffffffffffff;
constexpr auto register_mask = 0x3f;
constexpr auto r0_offset = 48;
constexpr auto r1_offset = 42;
constexpr auto r2_offset = 36;
constexpr auto imm_offset = 10;
constexpr auto imm_mask = 0xffffffff;
constexpr auto NUM_REGS = 64;
constexpr auto BYTE = 8;

struct three_reg {
    u8 r0, r1, r2;
};

struct two_reg_imm {
    u8 r0, r1;
    i32 imm;
};



#endif