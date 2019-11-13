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
    ifstream ifs_template(argv[1]),ifs_task(argv[2]);
    if(!ifs_template || !ifs_task)
        return -1;

    string str_template((istreambuf_iterator<char>(ifs_template)),
        std::istreambuf_iterator<char>());
    string str_task((istreambuf_iterator<char>(ifs_task)),
        std::istreambuf_iterator<char>());

    void* handle = mixer_create(str_template.c_str());
    if(!handle)
        return -1;

    if(!mixer_run(handle,str_task.c_str()))
        return -1;

    mixer_wait(handle);
    mixer_delete(handle);
    return 0;
}
