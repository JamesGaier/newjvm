#include"lexer.h"
#include"register-vm.h"
#include<iostream>
int main(int argc, char* argv[]) {
    try{
        if(argc > 1) {
            auto file_name = std::string{argv[1]};
            std::string bin_file = "";
            auto file_ending = file_name.substr(file_name.find(".")+ 1, file_name.length());
            if(file_ending == "vm") {
                lexer lex{argv[1]};
                lex.lex_source();
                lex.gen_code();

                bin_file = lex.get_output_name();
            }
            else if(file_ending == "bin") {
                bin_file = file_name;
            }
            // TODO: seperate mnemonics(human readable) from bytecode(compiler output)
            vm::load_program(bin_file);
            vm::run();
            //std::cout << "program ran" << std::endl;
        }
        else {
            throw "please run the executable in the following format: run command file_name.vm";
        }
    } catch(const char* e) {
        std::cout << e << std::endl;
    }

}