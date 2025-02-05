#include "TCPEchoServer.h"
#include <pthread.h>

/* メインスレッド関数 */
void *ThreadMain(void *arg);

/* クライアントスレッドに渡す構造体 */
struct ThreadsArgs
{
    int clntSock;
};

int main(int argc, char const *argv[])
{
    int servSock;                   /* サーバのソケットディスクリプタ */
    int clntSock;                   /* クライアントのソケットディスクリプタ */
    unsigned short echoServPort;    /* サーバのポート番号 */
    pthread_t threadID;             /* スレッドID */
    struct ThreadsArgs *threadArgs; /* スレッド引数 */

    /* 引数の数をチェック */
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s <Server Port: default 7>\n", argv[0]);
        exit(1);
    }
    else if (argc == 2)
    {
        echoServPort = atoi(argv[1]);
    }
    else
    {
        echoServPort = 7;
    }

    /* サーバのソケットを作成 */
    servSock = CreateTCPServerSocket(echoServPort);

    for (;;)
    {
        /* クライアントの接続を待機 */
        clntSock = AcceptTCPConnection(servSock);

        /* クライアント引数用にメモリを新しく確保 */
        if ((threadArgs = (struct ThreadsArgs *)malloc(sizeof(struct ThreadsArgs))) == NULL)
        {
            DieWithError("malloc() failed");
        }
        threadArgs->clntSock = clntSock;

        /* クライアントスレッドを生成 */
        if ((pthread_create(&threadID, NULL, ThreadMain, (void *)threadArgs)) != 0)
        {
            DieWithError("pthread_create() failed");
        }

        printf("with thread %ld\n", (long int)threadID);
    }
}

void *ThreadMain(void *threadArgs)
{
    int clntSock; /* クライアントのソケットディスクリプタ */

    /* 戻り時に、スレッドのリソースを割り当て解除 */
    pthread_detach(pthread_self());

    /* ソケットディスクリプタを引数から取り出す */
    clntSock = ((struct ThreadsArgs *)threadArgs)->clntSock;
    free(threadArgs);

    HandleTCPClient(clntSock);

    return (NULL);
}
