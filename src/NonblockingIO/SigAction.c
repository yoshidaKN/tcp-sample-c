#include <stdio.h>
#include <signal.h>
#include <unistd.h>

/* エラー処理関数 */
void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void InterruptSignalHandler(int signalType)
{
    printf("Interrupt Received. Exiting program.\n");
    exit(1);
}

int main(int argc, char const *argv[])
{
    struct sigaction handler; /* シグナルハンドラ */

    /* InterruptSignalHandler()をハンドラ関数として設定 */
    handler.sa_handler = InterruptSignalHandler;
    /* 全シグナルをマスクするマスを作成する */
    if (sigfillset(&handler.sa_mask) < 0)
    {
        DieWithError("sigfillset() failed");
    }

    /* 割り込みシグナルに対する処理を設定 */
    if (sigaction(SIGINT, &handler, 0) < 0)
    {
        DieWithError("sigaction() failed");
    }

    for (;;)
    {
        pause(); /* プログラムが終了するまで待機 */
    }

    return 0;
}
