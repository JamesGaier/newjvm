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



struct three_reg {
    u8 r0, r1, r2;
};
struct two_reg_imm {
    u8 r0, r1;
    i32 imm;
};
three_reg parse_three_reg(u64 instr) {
    u8 r0 = (instr >> r0_offset) & register_mask;
    u8 r1 = (instr >> r1_offset) & register_mask;
    u8 r2 = (instr >> r2_offset) & register_mask;

    return {r0, r1, r2};
}

two_reg_imm parse_two_reg(u64 instr) {
    u8 r0 = (instr >> r0_offset) & register_mask;
    u8 r1 = (instr >> r1_offset) & register_mask;
    i32 imm = (instr >> imm_offset) & imm_mask;

    return {r0, r1, imm};
}


void halt(i32 code) {

}
void fetch() {
    ir = memory[pc/page_size_bytes][pc % page_size_bytes];
}
u16 getOp() {
    return ir >> opcode_offset;
}
void decode() {
    opcode = getOp();
}
void ab_instr() {
    const auto regs = parse_three_reg(ir);
    const auto reg_imm = parse_two_reg(ir);
    switch(opcode) {
        case 1:
            registers[regs.r0] = registers[regs.r1] + registers[regs.r2];
            break;
        case 2:
            registers[regs.r0] = registers[regs.r1] - registers[regs.r2];
            break;
        case 3:
            registers[regs.r0] = registers[regs.r1] | registers[regs.r2];
            break;
        case 4:
            registers[reg_imm.r0] = registers[reg_imm.r1] | reg_imm.imm;
            break;
        case 5:
            registers[regs.r0] = registers[regs.r1] << registers[regs.r2];
            break;
        case 6:
            registers[regs.r0] = registers[regs.r1] >> registers[regs.r2];
            break;
        case 7:
            registers[regs.r0] = registers[regs.r1] << 32;
            break;
        case 8:
            registers[reg_imm.r0] = registers[reg_imm.r1] << reg_imm.imm;
            break;
        case 9:
            registers[reg_imm.r0] = registers[reg_imm.r1] >> reg_imm.imm;
            break;
        case 10:
            registers[regs.r0] = (registers[regs.r1] < registers[regs.r2]);
            break;
    }
}
void br_instr() {
    constexpr auto offset = 100;
    const auto reg_imm = parse_two_reg(ir);
    auto address = (instrData & 0xffffffff);
    switch(opcode) {
        case offset:
            pc = address << 3;
            break;
        case offset+1:
            ra = pc + 8;
            pc = address << 3;
            break;
        case offset+2:
            pc = (reg_imm.r0 == reg_imm.r1) ? pc:reg_imm.imm << 3;
            break;
        case offset+3:
            pc = (reg_imm.r0 != reg_imm.r1) ? pc:reg_imm.imm << 3;
            break;
        case offset+4:
            pc = registers[reg_imm.r0];
            break;
    }
}
void mem_instr() {
    constexpr auto offset = 200;
    const auto reg_imm = parse_two_reg(ir);
    const auto address = reg_imm.imm+reg_imm.r1;
    auto & page = memory[address/page_size_bytes];
    const auto in_page = address%page_size_bytes;

    if(opcode >= offset and opcode <= offset+5) {
        bool is_load = opcode % 2 == 0;
        auto len = (1 << ((opcode - 200) + 2) / 2);

        if(is_load) {
            for(u8 pos = 0; pos < len; pos++){
                page[in_page + pos] = (registers[reg_imm.r0] >> (8 * pos)) & 0xff;
            }
        }
        else {
            registers[reg_imm.r0] = 0;
            for(u8 pos = 0; pos < len; pos++){
                registers[reg_imm.r0] = static_cast<u64>(page[in_page + pos]) << (8 * pos);
            }
        }
    }
}

void execute() {
    if(opcode > 0 && opcode <= 10) {
        ab_instr();
    }
    else if(opcode >= 100 && opcode <= 104) {
        br_instr();
    }
    else if(opcode >= 200 && opcode <= 205) {
        mem_instr();
    }
    else if(opcode == 0) {
        halt(0);
    }

}
void run() {
    while(running) {
        fetch();
        decode();
        execute();
    }
}
void loadProgram(const std::vector<u64>& prog) {
    auto k = 0;
    for(auto i = 0; i < prog.size(); i++) {
        auto index = (pc + i);
        if(pc + i > 64) { // change to constexpr value
            index %= 64;
            k++;
            memory.push_back(page());
        }
        memory[k][index] = prog[i];
    }
}

#endif