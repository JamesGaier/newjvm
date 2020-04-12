#ifndef REGISTERVM_H_
#define REGISTERVM_H_
#include<vector>
#include<array>

// typedefs
using u64 = uint64_t;
using u32 = uint32_t;
using i32 = int32_t;
using u16 = uint16_t;
using u8 = uint8_t;

constexpr auto page_size_bytes = 4096;
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




class RegisterVM {
private:
    u64 pc = 0; // to be determined
    u64 sp = 0; // also to be determined;
    u64 ir = 0;
    u64 ra = 0;
    u16 opcode = 0;
    u32 instrData = 0;
    bool running = true;
    std::array<u64, NUM_REGS> registers;
    std::vector<u64> prog;
    std::vector<page> memory;
    void fetch();
    void decode();
    void execute();
    void abInstr();
    void brInstr();
    void memInstr();
    u16 getOp();
    bool isAb();
    bool isBr();
    bool isMem();
    void halt(const u64 code);
public:
    RegisterVM() = default;
    void run();
    void loadProgram(const std::vector<u64>& prog);
};


#endif