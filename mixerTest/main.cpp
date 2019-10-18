#include <iostream>
#include <fstream>
#include <sstream>
#include "../inc/mixerEngine.h"
using namespace std;

int main(int argc,char *argv[])
{
    if(argc < 3)
        return -1;
//    cout << "Hello world!" << endl;
//    ifstream ifs_template(argv[1]),ifs_task(argv[2]);
//    string str_template((istreambuf_iterator<char>(ifs_template)),
//        std::istreambuf_iterator<char>());
//    string str_task((istreambuf_iterator<char>(ifs_task)),
//        std::istreambuf_iterator<char>());
    std::shared_ptr<mixer> mix = create();
    mix->load(argv[1]);
    mix->run(argv[2]);
    return 0;
}
