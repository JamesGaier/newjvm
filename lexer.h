#ifndef lexer_H_
#define lexer_H_
#include"vm-utils.h"
#include<map>
#include<vector>
// type defs
using commands = std::vector<std::vector<std::string>>;
constexpr auto OPCODE_SIZE = 10;
constexpr auto REG_SIZE = 6;
class lexer {
public:
    explicit lexer(const std::string& file_name);
    void lex_source();
    void gen_code();
    std::string get_output_name();
private:
    std::map<std::string, u64> op {{
        {"syscall", 0}, {"add", 1}, {"sub", 2}, {"or", 3},
        {"ori", 4}, {"sl", 5}, {"sr", 6}, {"lui", 7},
        {"sli", 8}, {"sri", 9}, {"slt", 10}, {"slti", 11},
        {"addi", 12}, {"jmp", 100}, {"jal", 101}, {"jeq", 102}, {"jne", 103},
        {"jr", 104}, {"lw", 200}, {"sw", 201}, {"ldw", 202},
        {"sdw", 203}, {"lqw", 204}, {"sqw", 205}
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
};
#endif