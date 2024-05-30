#include "../code/log/log.h"
#include "../code/pool/threadpool.h"
#include <features.h>

#if __GLIBC__ == 2 && __GLIBC__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void TestLog() {
    int cnt = 0, level = 0;
    Log::instance()->init(level, "./testlog1", ".log", 0);
    for (level = 3; level >= 0; level--) {
        Log::instance()->SetLevel(level);
        for (int j = 0; j < 10000; j++) {
            for (int i = 0; i < 4; i++) {
                LOG_BASE(i, "%s 11111111 %d ======== ", "Test", cnt++);
            }
        }
    }
    cnt = 0;
    Log::instance()->init(level, "/testlog2", ".log", 5000);
    for (level = 0; level < 4; level++) {
        Log::instance()->SetLevel(level);
        for (int j = 0; j < 10000; j++) {
            for (int i = 0; i < 4; i++) {
                LOG_BASE(i, "%s 22222222 %d ======== ", "Test", cnt++);
            }
        }
    }
}

void ThreadLogTest(int i, int cnt) {
    
}