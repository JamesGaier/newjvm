#ifndef REGISTERVM_H_
#define REGISTERVM_H_
#include <string>
namespace vm{
    /*
    * @purpose: Simulates a cpu
    */
    void run();
    /*
    * @purpose: loads instructions into the text segment of memory
    */
    void load_program(const std::string& file_name);
}





#endif