#include "log/log.h"

int main()
{
    Log log("../local_log/log.txt");
    log.log_d("wyan", "sfafsfsd");
    log.log_i("wyan", "sfafsfsd");
    log.log_w("wyan", "sfafsfsd");
    log.log_e("wyan", "sfafsfsd");
    return 0;
}