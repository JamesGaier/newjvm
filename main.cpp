#include"lexer.h"
#include"register-vm.h"
int main(int argc, char* argv[]) {
    try{
        if(argc > 1) {
            lexer lex{argv[1]};
            lex.lex_source();

            lex.gen_code();
        }
        else {
            throw "please run the executable in the following format: run command file_name.vm";
        }
    } catch(const char* e) {
        std::cout << e << std::endl;
    }

}