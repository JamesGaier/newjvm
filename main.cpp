#include"lexer.h"
#include"register-vm.h"
int main() {
    lexer lex{"test.vm"};
    lex.lex_source();

    lex.gen_code();

}