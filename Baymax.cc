#include "Baymax.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define LOG_PATH "./log.txt"

int main()
{
#ifdef _LOG_
    int fd = open(LOG_PATH, O_WRONLY | O_CREAT, 0644);
    if(fd < 0)
        cerr << "open file failed." << endl;
    dup2(fd, 2);
#endif
    Baymax ivmt;
    while(true)
    {
        ivmt.Run();
    }
#ifdef _LOG_
    close(fd);
#endif 
    return 0;
}