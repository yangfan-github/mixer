#include <dlfcn.h>
#include <iostream>
#include "../inc/media.h"
using namespace std;

int main()
{
    cout << "Hello world!" << endl;
    init(&g_dump);
    media_type mt_in,mt_out;
    mt_in.set_sub(MST_H264);
    mt_out.set_sub(MST_RAWVIDEO);
    std::shared_ptr<media_transform> filter = create_filter<media_transform>(nullptr,&mt_in,&mt_out);
    filter.reset();
    return 0;
}
