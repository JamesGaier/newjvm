#include"register-vm.h"


struct ThreeRegister {
    u8 r0, r1, r2;
};
struct TwoRegImm {
    u8 r0, r1;
    i32 imm;
};
ThreeRegister parseThreeReg(u64 instr) {
    u8 r0 = (instr >> r0_offset) & register_mask;
    u8 r1 = (instr >> r1_offset) & register_mask;
    u8 r2 = (instr >> r2_offset) & register_mask;

    return {r0, r1, r2};
}

TwoRegImm parseTwoReg(u64 instr) {
    u8 r0 = (instr >> r0_offset) & register_mask;
    u8 r1 = (instr >> r1_offset) & register_mask;
    i32 imm = (instr >> imm_offset) & imm_mask;

    return {r0, r1, imm};
}



void RegisterVM::fetch() {
    ir = memory[pc++];
}
void RegisterVM::decode() {
    opcode = getOp(memory[pc]);
    instrData = getData(memory[pc]);
}

i16 RegisterVM::getOp() {
    return ir >> opcode_offset;
}
void RegisterVM::execute() {
    if(opcode > 0 && opcode <= 10) {
        abInstr();
    }
    else if(opcode >= 100 && opcode <= 104) {
        brInstr();
    }
    else if(opcode >= 200 && opcode <= 205) {
        memInstr();
    }
    else if(opcode == 0) {
        halt(0);
    }

}
void RegisterVM::abInstr() {
    const auto regs = parseThreeReg(ir);
    const auto reg_imm = parseTwoReg(ir);
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
void RegisterVM::brInstr() {
    constexpr auto offset = 100;
    const auto reg_imm = parseTwoReg(ir);
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
void RegisterVM::mem_instr() {
    constexpr auto offset = 200;
    const auto reg_imm = parseTwoReg(ir);
    const auto address = reg_imm.offset+reg_imm.r1;
    auto & page = memory[address/page_size_bytes];
    const auto inPage = address%page_size_bytes;

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
void RegisterVM::run() {
    while(running) {
        fetch();
        decode();
        execute();
    }
}
void RegisterVM::loadProgram(const std::vector<i64>& prog) {
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