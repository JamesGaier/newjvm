#ifndef REGISTERVM_H_
#define REGISTERVM_H_
#include<map>
#include<array>
#include <string>
#include"vm-utils.h"


u64 pc = 0; // to be determined
u64 sp = 0; // also to be determined;
u64 ir = 0;
u64 ra = 0;
u16 opcode = 0;
u32 instrData = 0;
bool running = true;
std::array<u64, NUM_REGS> registers;
std::vector<u64> prog;
std::map<u64, page> memory;


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
    ir = 0;
    u64 temp = 0;
    for(auto i = 0; i < BYTE; i++) {
        temp = static_cast<u64>(memory[pc/page_size_bytes][(pc % page_size_bytes) + i]) << 8*i;
        ir += temp;
    }
    if(pc < 32) {
        std::cout << std::hex << ir << " " << pc << std::endl;
    }
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
        case 0:
            running = false;
        case 1:
            registers[regs.r0] = registers[regs.r1] + registers[regs.r2];
            break;
        case 2:
            registers[regs.r0] = registers[regs.r1] - registers[regs.r2];
            std::cout << registers[regs.r0] << std::endl;
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
            pc = (registers[reg_imm.r0] == registers[reg_imm.r1]) ? pc:reg_imm.imm << 3;
            break;
        case offset+3:
            pc = (registers[reg_imm.r0] != registers[reg_imm.r1]) ? pc:reg_imm.imm << 3;
            break;
        case offset+4:
            pc = registers[reg_imm.r0];
            break;
    }
}
void mem_instr() {
    auto offset = 200;
    const auto reg_imm = parse_two_reg(ir);
    const auto address = reg_imm.imm+reg_imm.r1;
    auto & cur_page = memory[address/page_size_bytes][address % page_size_bytes];
    const auto in_page = address%page_size_bytes;

    if(opcode >= offset and opcode <= offset+5) {
        bool is_load = opcode % 2 == 0;
        auto len = (1 << ((opcode - 200) + 2) / 2);

        if(is_load) {
            for(u8 pos = 0; pos < len; pos++){
                //std::cout << cur_page << std::endl;
                //page[cur_page + pos] = (registers[reg_imm.r0] >> (8 * pos)) & 0xff; I do not know why but this does not work
            }
        }
        else {
            registers[reg_imm.r0] = 0;
            for(u8 pos = 0; pos < len; pos++){
                //registers[reg_imm.r0] = static_cast<u64>(page[cur_page + pos]) << (8 * pos); neither does this
            }
        }
    }
}

void execute() {
    //std::cout << opcode << std::endl;
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
        //hault on the last nibble of the program
        halt(0);
    }
}
void run() {
    while(running) {
        fetch();
        decode();
        execute();
        pc += 8;
    }
}
std::vector<std::string> read_source(const std::string& file_name) {
    std::ifstream input_file{file_name};

    try{
        if(!input_file) {
            throw "File not found.";
        }
    }
    catch(const char* e) {
        std::cout << e << std::endl;
    }

    std::stringstream ss;
    std::string line;
    while(getline(input_file, line)) {
        ss << line;
    }

    auto program = ss.str();
    std::stringstream cur_instr;
    std::vector<std::string> prog;
    cur_instr << program[0];
    for(auto i = 1; i < program.length()+1; i++) {
        auto ch = program[i];
        if(i % 16 == 0)  {
            prog.push_back(cur_instr.str());
            cur_instr.str("");
        }
        cur_instr << ch;
    }
    return prog;
}
void load_program(const std::string& file_name) {
    auto prog = read_source(file_name);
    auto k = 0;
    auto index = pc;
    memory.emplace(0, page());
    for(auto i = 0; i < prog.size(); i++) {
        if(pc + i > 511) {
            index %= 511;
            memory.emplace(k, page());
            k++;
        }
        auto cur_instr = prog[i];
        //std::cout << cur_instr << std::endl;
        for(auto j = (BYTE*2)-1; j >= 1; j-=2) {
            std::stringstream ss;
            ss << cur_instr[j-1];
            ss << cur_instr[j];
            memory[k][index] = static_cast<u8>(std::stoi(ss.str().c_str(), nullptr, 16));
            //std::cout << std::hex << " " << k << " " << index <<  " " << static_cast<u64>(memory[k][index]) << std::endl;
            //std::cout << index << std::endl;
            //std::cout << k << " " << std::hex << (unsigned)memory[k][index] << std::endl;
            index++;
        }
    }
}

#endif