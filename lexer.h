#ifndef lexer_H_
#define lexer_H_
#include"vm-utils.h"
#include<iomanip>
#include<map>
#include<cmath>
#include<stack>
#include<iostream>
#include<vector>
#include<array>
#include<fstream>
#include<sstream>
#include<algorithm>
// type defs
using commands = std::vector<std::vector<std::string>>;
constexpr auto OPCODE_SIZE = 10;
constexpr auto REG_SIZE = 6;
class lexer {
private:
    std::map<std::string, u64> op {{
        {"halt", 0}, {"add", 1}, {"sub", 2}, {"or", 3},
        {"ori", 4}, {"sl", 5}, {"sr", 6}, {"lui", 7},
        {"sli", 8}, {"sri", 9}, {"sub", 2}, {"slt", 10},
        {"jmp", 100}, {"jal", 101}, {"jeq", 102}, {"jne", 103},
        {"jr", 104}, {"lw", 200}, {"sw", 201}, {"ldw", 202},
        {"sdw", 104}, {"lqw", 200}, {"sqw", 201}
    }};
    std::map<std::string, i64> label_line;
    std::string source = "";
    std::string file_name = "";
    std::string output_name = "";
    commands cmds;
    u64 idx = 0;
    u64 line_number = 1;
    void new_line();
    void ignore();
    void advance();
    void lex_line();
    char peek();
    u64 get_three_reg(const u64 op_sec,const std::vector<std::string>& command);
    u64 get_two_reg_imm(const u64 op_sec,const std::vector<std::string>& command);
    std::string pad_instruction(const u64 instruction);
    u64 get_mem(const u64 op_sec,const std::vector<std::string>& command);
public:
    lexer(const std::string& file_name);
    void lex_source();
    void gen_code();
    std::string get_output_name();
};
#endif