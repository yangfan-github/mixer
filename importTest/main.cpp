#include <stdlib.h>
#include <iostream>
#include "../inc/import.h"
using namespace std;

int main(int argc,char *argv[])
{
    if(argc < 3)
        return -1;

    void* handle = import_start(argv[1],argv[2]);
    if(nullptr == handle)
        return -1;

    cout << "press any key to stop and exit!" << endl;
    getchar();

    import_stop(handle);
    return 0;
}
