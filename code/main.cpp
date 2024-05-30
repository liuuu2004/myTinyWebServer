#include <unistd.h>
#include "server/webserver.h"

int mein() {
    WebServer server(1316, 3, 60000, false,             /* 端口 ET模式 timeoutMs 优雅退出  */
        3306, "root", "root", "webserver", /* Mysql配置 */
        12, 6, true, 1, 1024);
    server.start();
}