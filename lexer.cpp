#include"lexer.h"
#include<iomanip>
#include<stack>
#include<cmath>
#include<iostream>
#include<fstream>
#include<sstream>
#include<algorithm>
/*
* @param _file_name: file path
* @purpose: the constructor's purpose is to load the assembly into the source string
*
*/
lexer::lexer(const std::string& file_name_) : file_name{file_name_} {
    std::ifstream input_file{file_name_};
    if(!input_file) {
      std::cout << "Cannot open file: " << file_name_ << std::endl;
      return;
    }
    std::stringstream ss;
    std::string line;
    while(getline(input_file, line)) {
      ss << line << '\n';
      //std::cout << line << std::endl;
    }
    source = ss.str();
}
/*
*
* @param command: list of commands
* @purpose: to print out the commands that he lexer gets from the source
*
*/
void debug_print(const std::vector<std::string>& command) {

  for(const auto& part: command) {
    std::cout << part << std::endl;
  }
  std::cout <<std::endl;
}
/*
* @purpose: move the index forward one
*
*/
void lexer::advance() {
  idx++;
}
/*
* @purpose: look at the current character
*
*/
char lexer::peek() {
  if(!isalpha(source[idx])) {
    return source[idx];
  }
  return tolower(source[idx]);
}
/*
* ignores a number of different characters
*
*/
void lexer::ignore() {
  while(peek() == ' ' || peek() == ',' || peek() == '\t') {
    advance();
  }
}
/*
* @purpose: reads commands and breaks them into this format:
* command name, register 0...register n, imm
*/
void lexer::lex_line() {
  std::vector<std::string> command;
  ignore();
  bool is_label = false;
  if(isalpha(peek())) {
    auto start = idx;
    while(isalpha(peek()) || peek() == ':') {
      if(peek() == ':') is_label = true;
      advance();
    }
    auto val = source.substr(start, idx-start);
    if(is_label) {
      label_line.emplace(val.substr(0,val.length()-1), line_number);
    }
    command.push_back(val);
  }
  else {
    throw "expression must start with a letter\n";
  }
  if(is_label) {
    return;
  }
  ignore();
  if(isdigit(peek())) {
    auto start = idx;
    while(isdigit(peek())) {
      advance();
    }
    command.push_back(source.substr(start, idx-start));
  }
  ignore();
  if(isdigit(peek())) {
    auto start = idx;
    while(isdigit(peek()) || tolower(peek()) == 'x') {
      advance();
    }
    command.push_back(source.substr(start, idx-start));
    if(peek() == '[') {
      advance();
      start = idx;
      while(isdigit(peek())) {
        advance();
      }
      command.push_back(source.substr(start, idx-start));
    }
  }
  ignore();
  if(isdigit(peek()) || peek() == '#' || isalpha(peek())) {
    auto start = idx;
    while(isdigit(peek()) || peek() == '#' || isalpha(peek())) {
      advance();
      //std::cout << "advancing peeking " << peek() << std::endl;

    }
    //std::cout << peek() << std::endl;
    //std::cout << start << " " << idx-start << std::endl;
    command.push_back(source.substr(start, idx-start));
  }
  if(!isalpha(peek()) && idx < source.length()){
    while(!isalpha(peek()) && idx < source.length()) {
      advance();
    }
  }
  //debug_print(command);
  cmds.push_back(command);
}
/*
* @purpose: break the instructions into pieces line by line
*/
void lexer::lex_source() {
  while(idx < source.length()) {
    try{
      lex_line();
      line_number++;
    } catch(const char* e) {
      std::cout << e << std::endl;
      break;
    }
  }
}
/*
* @param op_sec, command: opcode section is the opcode shifted to its correct position.
* Command is the list of instructions
* @purpose: to break apart an instruction that has two registers and one immediate value
*/
u64 lexer::get_two_reg_imm(const u64 op_sec,const std::vector<std::string>& command) {
  u64 r0 = strtoull(command[1].c_str(), nullptr, 10) << r0_offset;
  u64 r1 = strtoull(command[2].c_str(), nullptr, 10) << r1_offset;
  u64 r2 = label_line[command[3]] << imm_offset;
  if(command[3][0] == '#') {
    //TODO: hexadecimal check
    r2 = strtoull(command[3].substr(1,command[3].length()-1).c_str(), nullptr, 10) << imm_offset;
    //std::cout << r2 << std::endl;
  }
  return op_sec + r0 + r1 + r2;
}
/*
* @param op_sec, command: opcode section is the opcode shifted to its correct position.
* Command is the list of instructions
* @purpose: to break apart an instruction with three registers
*/
u64 lexer::get_three_reg(const u64 op_sec,const std::vector<std::string>& command) {
  u64 r0 = strtoull(command[1].c_str(), nullptr, 10) << r0_offset;
  u64 r1 = strtoull(command[2].c_str(), nullptr, 10) << r1_offset;
  u64 r2 = strtoull(command[3].c_str(), nullptr, 10) << r2_offset;
  return op_sec + r0 + r1 + r2;
}
/*
* @param op_sec, command: opcode section is the opcode shifted to its correct position.
* Command is the list of instructions
* @purpose: to break apart an instruction that accesses memory
*/
u64 lexer::get_mem(const u64 op_sec, const std::vector<std::string>& command) {
  u64 r0 = strtoull(command[1].c_str(), nullptr, 10) << r0_offset;
  u64 r1 = strtoull(command[3].c_str(), nullptr, 10) << r1_offset;
  std::stringstream off_str;
  auto base = (command[2].substr(0,2) == "0x") ? 16:10;
  off_str << std::dec << strtoull(command[2].c_str(), nullptr, base);
  u64 offset = strtoull(off_str.str().c_str(), nullptr, 10) << imm_offset;
  return op_sec + r0 + r1 + offset;
}
/*
* @param instruction: the current instruction
* @purpose: to pad the instruction with zeros if it is not 16 nibbles long
*/
std::string lexer::pad_instruction(const u64 instruction) {
  std::stringstream ss;
  u64 pad_len = log(instruction)/log(16) + 1;
  if(pad_len == 16){
    ss << std::hex << instruction;
    return ss.str();
  }
  ss << std::setw(16 - pad_len) << std::setfill('0') << 0 << std::hex << instruction;
  return ss.str();
}
/*
* @purpose: generates the machine code for the virtual machine to interpret.
*/
void lexer::gen_code() {
  auto name = file_name.substr(0,file_name.find(".vm"));
  output_name = name+".bin";
  std::ofstream output_file{output_name, std::ios::binary};
  for(const auto& command: cmds) {
    if(op.find(command[0]) != op.end()) {
      u64 op_sec = op[command[0]] << opcode_offset;
      if(command[0] == "add" || command[0] == "sub" || command[0] == "or"
        || command[0] == "sl" || command[0] == "sr" || command[0] == "slt") {
          output_file << pad_instruction(get_three_reg(op_sec, command));
      }
      else if(command[0] == "lw" || command[0] == "sw" || command[0] == "ldw"
             || command[0] == "sdw" || command[0] == "lqw" || command[0] == "sqw") {
        output_file << pad_instruction(get_mem(op_sec, command));
      }
      else if(command[0] == "jmp" || command[0] == "jal") {
        u64 addr = strtoull(command[1].substr(1,command[1].length()-1).c_str(), nullptr, 10);
        if(label_line.find(command[1].substr(0,command[1].length())) != label_line.end()) {
          addr = label_line[command[1].substr(0,command[1].length())];
        }
        output_file << pad_instruction((op_sec + addr));
      }
      else if(command[0] == "jeq" || command[0] == "jne"
            || command[0] == "ori" || command[0] == "sli"
            || command[0] == "sri") {

        output_file << pad_instruction(get_two_reg_imm(op_sec, command));
      }
      else if(command[0] == "halt" || command[0] == "jr") {
        u64 reg = strtoull(command[1].c_str(), nullptr, 10);
        output_file << pad_instruction((op_sec + reg));
      }
      else if(command[0] == "lui") {
        u64 reg = strtoull(command[1].c_str(), nullptr, 10) << r0_offset;
        u64 imm = strtoull(command[2].substr(1,command[2].length() - 1).c_str(), nullptr, 10) << imm_offset;
        output_file << pad_instruction((op_sec + reg + imm));
      }
    }
  }
}
std::string lexer::get_output_name() {
  return output_name;
}