#include"lexer.h"
#include"vm-utils.h"
#include<iomanip>
#include<stack>
#include<cmath>
#include<iostream>
#include<fstream>
#include<sstream>
#include<algorithm>
#include<optional>
#include<iterator>
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
  if(idx < source.length()) {
    idx++;
  }
  else {
    std::cerr << "Lexer out of range" << std::endl;
  }
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
  while(peek() == ' ' || peek() == ',' || peek() == '\t' || peek() == '\n' || peek() == '\r') {
    advance();
  }
}
std::optional<std::string> lexer::get_section() {
  if(peek() == '.') {
      advance();
      auto start = idx;
      while(isalpha(peek())) {
        advance();
      }

      return source.substr(start, idx-start);
  }
  else {
    std::cerr << "section not found" << std::endl;
    return {};
  }
}
/*
* @purpose: reads commands and breaks them into this format:
* command name, register 0...register n, imm
*/
void lexer::lex_line() {
  std::vector<std::string> command;
  // ignore any initial whitespace
  ignore();
  bool is_label = false;
  // get the instruction
  if(isalpha(peek())) {
    auto start = idx;
    // checks if label
    while(isalpha(peek()) || peek() == ':') {
      if(peek() == ':') is_label = true;
      advance();
    }
    auto val = source.substr(start, idx-start);
    if(is_label) {
      label_line.emplace(val.substr(0,val.length()-1), line_number);
    }
    // push back command or label
    command.push_back(val);
  }
  else {
    std::cout << static_cast<u32>(peek()) << std::endl;
    std::cout << "expression must start with a letter\n";
  }
  if(is_label) {
    // no need to continue lexing if there is a label
    return;
  }
  // ignore more whitespace
  ignore();
  // check for first argument
  if(isdigit(peek())) {
    auto start = idx;
    while(isdigit(peek())) {
      advance();
    }
    command.push_back(source.substr(start, idx-start));
  }
  // ignore whitespace
  ignore();
  if(isdigit(peek())) {
    auto start = idx;
    // if offset is hex or is a number
    if(isdigit(peek()) || tolower(peek()) == 'x') {
      while(isdigit(peek()) || tolower(peek()) == 'x') {
        advance();
      }
      command.push_back(source.substr(start, idx-start));
    }
  }
  // if it is a memory read/write
  if(peek() == '[') {
    advance();
    auto start = idx;
    while(isdigit(peek())) {
      advance();
    }
    //std::cout << idx-start << std::endl;
    command.push_back(source.substr(start, idx-start));
    advance();
  }
    //std::cout <<  command[0] << " " << command[2] << std::endl;
  ignore();
  if(isdigit(peek()) || peek() == '#' || isalpha(peek())) {
    auto start = idx;
    advance();
    if(peek() == '-') {
      advance();
    }
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
  std::vector<std::string> command;
  auto section = *get_section();
  if(section == "data") {
    command.push_back("data");
    std::stringstream ss;
    while(peek() != '.') {
      if(peek() != ' ') {
        ss << peek();
      }
      advance();
    }
    command.push_back(ss.str());
  }

  if(section == "text") {
    command.push_back("text");
  }
  cmds.push_back(command);

  while(idx < source.length() && peek() != '.') {
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
    auto is_hex = command[3][1] == 'x';
    i32 base = (is_hex) ? 16:10;
    auto hex_off = (is_hex) ? 1:0;
    //std::cout << std::hex << r2 << std::endl;
    r2 = strtoll(command[3].substr(1+hex_off,command[3].length()-1).c_str(), nullptr, base) << imm_offset;
  }
  return op_sec | r0 | r1 | (r2 & imm_mask << imm_offset);
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
  return op_sec | r0 | r1 | r2;
}
/*
* @param op_sec, command: opcode section is the opcode shifted to its correct position.
* Command is the list of instructions
* @purpose: to break apart an instruction that accesses memory
*/
u64 lexer::get_mem(const u64 op_sec, const std::vector<std::string>& command) {
  u64 r0 = strtoull(command[1].c_str(), nullptr, 10) << r0_offset;
  u64 r1 = strtoull(command[(command.size() == 4) ? 3:2].c_str(), nullptr, 10) << r1_offset;
  //std::cout << command.size() << std::endl;
  // if command 2 is not empty set offset, otherwise return empty
  std::stringstream off_str;
  if(command.size() == 4) {
    auto base = (command[2].substr(0,2) == "0x") ? 16:10;
    off_str << std::dec << strtoull(command[2].c_str(), nullptr, base);
  }
  u64 offset = strtoull(off_str.str().c_str(), nullptr, 10) << imm_offset;
  return op_sec + r0 + r1 + offset;
}
/*
* @param instruction: the current instruction
* @purpose: to pad the instruction with zeros if it is not 16 nibbles long
*/
/*
tds::u64 lexer::pad_instruction(const u64 instruction) {
  std::stringstream ss;
  u64 pad_len = log(instruction)/log(16) + 1;
  if(pad_len == 16){
    ss << std::hex << instruction;
    return ss.str();
  }
  ss << std::setw(16 - pad_len) << std::setfill('0') << 0 << std::hex << instruction;
  return ss.str();
}
*/
/*
* @purpose: generates the machine code for the virtual machine to interpret.
*/
void lexer::gen_code() {
  //std::cout << std::hex << -100 << std::endl;
  std::vector<u8> text_buffer;
  std::vector<u8> data_buffer;
  std::vector<u8> header_buffer;
  auto name = file_name.substr(0,file_name.find(".vm"));
  output_name = name+".bin";
  std::ofstream output_file{output_name, std::ios::binary | std::ios::out};
  const auto add_u64 = [&text_buffer](u64 data) {
    auto cur_qword = data;
    for(auto i = 0; i < QWORD/BYTE; i++) {
      text_buffer.push_back(cur_qword & 0xFF);
      cur_qword = cur_qword >> BYTE;
    }
  };
  const auto add_header_entry = [&header_buffer](const std::string& section,const u32 section_offset,
                                                    const u32 section_length){

    // copy the bytes of the string into the buffer
    std::copy(section.begin(), section.end(), std::back_inserter(header_buffer));
    // null byte
    header_buffer.push_back(0);

    // read the offset into the buffer
    auto off = section_offset;
    for(auto i = 0; i < QWORD/BYTE; i++) {
      header_buffer.push_back(off & 0xFF);
      off = off >> BYTE;
    }
    // read the length into the buffer
    auto length = section_length;
    for(auto i = 0; i < QWORD/BYTE; i++) {
      header_buffer.push_back(length & 0xFF);
      length = length >> BYTE;
    }
    //read the null byte into the buffer
    header_buffer.push_back(0);
  };
  auto text_length = 0;
  auto text_offset = 0;
  auto data_length = 0;
  auto data_offset = 0;
  std::vector<u8> hex;
  for(const auto& command: cmds) {
    if(command[0] == "text") {
      text_offset = text_length;
    }
    if(command[0] == "data") {
      data_offset = data_length;
      /*
      for(const auto& ch: command[1]) {
        if(ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
          data_buffer.push_back(ch);
        }
      }
      */

    }
    if(op.find(command[0]) != op.end()) {
      u64 op_sec = op[command[0]] << opcode_offset;
      if(command[0] == "add" || command[0] == "sub" || command[0] == "or"
        || command[0] == "sl" || command[0] == "sr" || command[0] == "slt") {
          add_u64(get_three_reg(op_sec, command));
      }
      else if(command[0] == "lw" || command[0] == "sw" || command[0] == "ldw"
             || command[0] == "sdw" || command[0] == "lqw" || command[0] == "sqw") {
        add_u64(get_mem(op_sec, command));
      }
      else if(command[0] == "jmp" || command[0] == "jal") {
        u64 addr = strtoull(command[1].substr(1,command[1].length()-1).c_str(), nullptr, 10);
        if(label_line.find(command[1].substr(0,command[1].length())) != label_line.end()) {
          addr = label_line[command[1].substr(0,command[1].length())];
        }
        add_u64(op_sec | addr);
      }
      else if(command[0] == "jeq" || command[0] == "jne"
            || command[0] == "ori" || command[0] == "sli"
            || command[0] == "sri" || command[0] == "addi"
            || command[0] == "syscall") {
        add_u64(get_two_reg_imm(op_sec, command));
      }
      else if(command[0] == "halt" || command[0] == "jr") {
        u64 reg = strtoull(command[1].c_str(), nullptr, 10);
        add_u64(op_sec | reg);
      }
      else if(command[0] == "lui") {
        u64 reg = strtoull(command[1].c_str(), nullptr, 10) << r0_offset;
        u64 imm = strtoull(command[2].substr(1,command[2].length() - 1).c_str(), nullptr, 10) << imm_offset;
        add_u64(op_sec | reg | imm);
      }
      text_length += 64;
      data_length += 64;
    }
  }
  header_buffer.push_back(0x7E);
  header_buffer.push_back('N');
  header_buffer.push_back('J');
  for(auto i = 0; i < 4; i++) {
    header_buffer.push_back(0);
  }



  //auto data_size = write_header_entry(".data", data_start + data_offset, data_length, 5);
  //auto text_size = write_header_entry(".text", pc_start + text_offset,text_length, data_size);
  //write_header(data_size + text_size);
}
std::string lexer::get_output_name() {
  return output_name;
}