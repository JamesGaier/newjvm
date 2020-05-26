#include"register-vm.h"
#include"vm-utils.h"
#include<vector>
#include<map>
#include<array>
#include<iostream>
#include<fstream>
#include<sstream>
#include<iterator>
#include<cmath>
namespace vm {

    u64 pc = pc_start;
    u64 ir = 0;
    u16 opcode = 0;
    bool running = true;
    std::array<u64, NUM_REGS> registers;

    std::vector<u64> prog;
    std::map<u64, page> memory;
    u8 load_byte(u64 loc) {
        return memory[loc/page_size_bytes][loc%page_size_bytes];
    }
    /*
    * @param instr: 64 bit instruction
    * @purpose: To parse an instruction into a three_reg struct
    */
   void init_reg() {
        registers[0]  = 0;
        registers[61] = top_stack; // stack pointer
        registers[62] = top_stack; // frame pointer
        registers[63] = 0; // return address
   }
    three_reg parse_three_reg(u64 instr) {
        u8 r0 = (instr >> r0_offset) & register_mask;
        u8 r1 = (instr >> r1_offset) & register_mask;
        u8 r2 = (instr >> r2_offset) & register_mask;
        return {r0, r1, r2};
    }
    /*
    * @param instr: 64 bit instruction
    * @purpose: To parse an instruction into a two_reg struct
    */
    two_reg_imm parse_two_reg(u64 instr) {
        u8 r0 = (instr >> r0_offset) & register_mask;
        u8 r1 = (instr >> r1_offset) & register_mask;
        i32 imm = (instr >> imm_offset) & imm_mask;

        return {r0, r1, imm};
    }

    void print(u64 option, u64 value) {

        switch(option) {
            case 0:
                std::cout << std::endl;
                break;
            case 1:
                std::cout << (value & 0xFFFF);
                std::cout.flush();
                break;
            case 2:
            case 3:
                std::cerr << "unimplmemnted floating point print" << std::endl;
                break;
            case 4:
                {
                    std::string str;
                    //auto str_ptr = value;
                    while(load_byte(value) != 0) {
                        str += load_byte(value);
                        value++;
                    }
                    std::cout << str;
                    std::cout.flush();
                }
                break;
            case 5:
                std::cout << value;
                std::cout.flush();
                break;
        }
    }
    void user_input(u64 option, u64 reg_num) {
        switch(option) {
            case 0:
                break;
            case 1:{
                u32 input_int32;
                std::cin >> input_int32;
                if(std::cin.fail()) {
                    running = false;
                }
                registers[reg_num] = input_int32;
            } break;
            case 2:
            case 3:
                std::cerr << "unimplemented" << std::endl;
                break;
            case 4:
                // dont know
                break;
            case 5:
                u64 input_int64;
                std::cin >> input_int64;
                if(std::cin.fail()) {
                    running = false;
                }
                registers[reg_num] = input_int64;
                break;
        }
    }
    /*
    void buffer(u64 off, u64 reg_num) {
        // don't know
    }
    void free_mem(u64 reg_num) {
        // don't know
    }

    */
    void halt() {
        running = false;
    }
    void syscall() {
        const auto reg_imm = parse_two_reg(ir);
        switch(reg_imm.imm) {
            case 0:
                break;
            case 1:
                print(registers[reg_imm.r0], registers[reg_imm.r1]);
                break;
            case 2:
                user_input(registers[reg_imm.r0], reg_imm.r1);
                break;
            case 3:
                //buffer(registers[reg_imm.r0], reg_imm.r1);
                break;
            case 4:
                //free_mem(reg_imm.r0);
                break;
            case 5:
                halt();
                break;
        }
    }
    /*
    * @purpose: To fetch an instruction from memory and load it
    * into the instruction register
    */
    void fetch() {
        ir = 0;
        for(u8 i = 0; i < BYTE; i++) {
            auto temp = static_cast<u64>(memory[pc/page_size_bytes][(pc % page_size_bytes) + i]) << BYTE*i;
            ir |= temp;
        }
    }
    /*
    * @purpose: To get the opcode from instructions.
    */
    u16 getOp() {

        return ir >> opcode_offset;
    }
    /*
    * @purpose: To get the opcode.
    */
    void decode() {
        opcode = getOp();
    }


    /*
    * @purpose: To do arithemetic and bitwise instructions
    */
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
                registers[reg_imm.r0] = registers[reg_imm.r1] | static_cast<u32>(reg_imm.imm);
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
            case 11:{
                const auto top_half = (reg_imm.imm & 0x80000000) ? 0xFFFFFFFFul : 0;
                registers[reg_imm.r0] = registers[reg_imm.r1] < (top_half << 32u | reg_imm.imm);
            } break;
            case 12:
                {
                    const auto top_half = (reg_imm.imm & 0x80000000) ? 0xFFFFFFFFul : 0;
                    registers[reg_imm.r0] = registers[reg_imm.r1] + (top_half << 32u | reg_imm.imm);

                }
                break;

        }
    }
    /*
    * @purpose: To do branch instructions
    */
    void br_instr() {
        constexpr auto offset = 100;
        const auto reg_imm = parse_two_reg(ir);

        auto address = ir & data_mask;
        switch(opcode) {
            case offset:
                pc = address << 3;
                break;
            case offset+1:
                registers[63] = pc;
                pc = address << 3;
                break;
            case offset+2:
                if(registers[reg_imm.r0] == registers[reg_imm.r1]) {
                    pc += reg_imm.imm << 3;
                }
                break;
            case offset+3:
                if(registers[reg_imm.r0] != registers[reg_imm.r1]) {
                    pc += reg_imm.imm << 3;
                }
                break;
            case offset+4:
                pc = registers[reg_imm.r0];
                break;
        }
    }
    /*
    * @purpose: To do memory instructions
    *  use .at() for lw and sw memory accesses
    */
    void mem_instr() {
        auto offset = 200;
        const auto reg_imm = parse_two_reg(ir);
        const auto address = reg_imm.imm+registers[reg_imm.r1];
        if(opcode >= offset && opcode <= offset+5) {
            bool is_load = opcode % 2 == 0;
            auto len = (1 << ((opcode - 200) + 2) / 2);
            if(!is_load) {
                for(u8 pos = 0; pos < len; pos++){
                    memory[address/page_size_bytes][address % page_size_bytes + pos] =
                        (registers[reg_imm.r0] >> (8 * pos)) & 0xff;
                }
            }
            else {
                registers[reg_imm.r0] = 0;
                for(u8 pos = 0; pos < len; pos++){
                    registers[reg_imm.r0] += static_cast<u64>(load_byte(address + pos)) << (8 * pos);
                }
            }
        }
    }
    /*
    * @purpose: To execute instructions based on the opcode
    */
    void execute() {
        if(opcode > 0 && opcode <= 12) {
            ab_instr();
        }
        else if(opcode >= 100 && opcode <= 104) {
            br_instr();
        }
        else if(opcode >= 200 && opcode <= 205) {
            mem_instr();
        }
        else if(opcode == 0) {
            syscall();
        }
    }
    /*
    * @purpose: Reads the source file and parses out the 64 bit instructions
    */
    std::vector<u8> read_source(const std::string& file_name) {
        std::ifstream input_file{file_name, std::ios::in | std::ios::binary};
        input_file.seekg(0, std::ios::end);
        auto file_size = static_cast<i32>(input_file.tellg());
        input_file.seekg(0, std::ios::beg);
        if(!input_file) {
              std::cout << "File not found" << std::endl;
        }
        std::vector<u8> byte_code;
        u8 instr;
        while(input_file.tellg() < file_size) {
            input_file.read((char*)&instr, sizeof(instr));
            byte_code.push_back(instr);
        }
        input_file.close();
        return byte_code;
    }
    void run() {
        init_reg();
        while(running) {
            fetch();
            pc += 8;
            decode();
            execute();
        }
    }
    void load_program(const std::string& file_name) {
        auto prog = read_source(file_name);
        auto page_index = pc/page_size_bytes;
        auto index = pc % page_size_bytes;

        memory.emplace(0, page());

        // iterator for the block of data being read from the file
        auto cur_block = prog.begin();
        if(*cur_block == 0x7E && *(cur_block + 1) == 'N' && *(cur_block + 2) == 'J') {
            auto consume_next_byte = [&cur_block] {
                auto value = *cur_block;
                cur_block++;
                return value;
            };
            cur_block+=3;
            u32 table_length = 0;
            for(auto i = 0; i < 4; i++) {
                table_length |= consume_next_byte() << (i*BYTE);
            }
            auto get_section = [&cur_block, &consume_next_byte] {
                std::stringstream ss;
                if(*cur_block == '.') {
                    while(*cur_block != 0) {
                        ss << consume_next_byte();
                    }
                    cur_block++;
                    return ss.str();
                }
                else {
                    return std::string();
                }
            };
            auto read_table_entry = [&cur_block, &consume_next_byte] {
                std::pair<u32, u32> to_ret = std::make_pair(0,0);
                // read data offset
                for(auto i = 0; i < 4; i++) {
                    to_ret.first |= consume_next_byte() << (i*BYTE);
                }
                // read section length
                for(auto i = 0; i < 4; i++) {
                    to_ret.second |= consume_next_byte() << (i*BYTE);
                }
                // null byte
                if(consume_next_byte() != 0)
                {
                    std::cout << "expected cur_block to be 0 found: " << std::hex << static_cast<u32>(*cur_block) << std::endl;
                }
                return to_ret;
            };
            auto read_section = [&](u64 offset, u64 length, u64 mem_start) -> void{
                if(memory.count(mem_start/page_size_bytes) == 0) {
                    auto num_pages = static_cast<u32>(
                        std::ceil(static_cast<double>(length)/page_size_bytes));
                    for(auto i = 0u; i < num_pages; i++) {
                        memory.emplace(mem_start/page_size_bytes, page());
                    }
                }
                u64 dest = mem_start;
                for(auto i = 0; i < length; i++) {
                    memory[dest/page_size_bytes][dest%page_size_bytes] = prog[i+offset];
                    dest++;
                }
            };
            for(auto i = 0; i < num_sections; i++) {
                auto section_name = get_section();
                if(section_name == ".data") {
                    const auto data_entry = read_table_entry();
                    read_section(data_entry.first, data_entry.second, data_start);
                }
                else if(section_name == ".text") {
                    const auto text_entry = read_table_entry();
                    read_section(text_entry.first, text_entry.second, pc);
                }
            }
        }
        else {
            std::cerr << "file format incorrect" << std::endl;
            running = false;
        }
    }
}
