#include"lexer.h"
#include"register-vm.h"
#include<iostream>
int main(int argc, char* argv[]) {
    try{
        if(argc > 1) {
            lexer lex{argv[1]};
            lex.lex_source();
            lex.gen_code();

            vm::load_program(lex.get_output_name());
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