#include "TCPEchoServer.h"
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    int servSock;                    /* サーバーのソケットディスクリプタ */
    int clntSock;                    /* クライアントのソケットディスクリプタ */
    unsigned short echoServPort;     /* サーバーのポート */
    pid_t processID;                 /* プロセスID */
    unsigned int childProcCount = 0; /* 子プロセスの数 */

    /* 引数をチェック */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }
    echoServPort = atoi(argv[1]); /* 1つ目の引数: ポート */

    /* サーバーのソケットを作成 */
    servSock = CreateTCPClientSocket(echoServPort);

    for (;;)
    {
        /* クライアントの接続を待機 */
        clntSock = AcceptTCPConnection(servSock);

        /* プロセスをフォーク */
        if ((processID = fork()) < 0)
        {
            /* fork() に失敗 */
            DieWithError("fork() failed");
        }
        else if (processID == 0)
        {
            /* 子プロセス
             * クライアントのソケットをクローズし、
             * クライアントとの通信を処理 */
            close(servSock);
            HandleTCPClient(clntSock);
            exit(0);
        }

        printf("with child process: %d\n", processID);
        close(clntSock);  /* 親プロセスはクライアントのソケットをクローズ */
        childProcCount++; /* 子プロセスの数をインクリメント */

        /* ゾンビプロセスの処理 */
        while (childProcCount)
        {
            /* ゾンビプロセスの処理 */
            processID = waitpid((pid_t)-1, NULL, WNOHANG);
            if (processID < 0)
            {
                DieWithError("waitpid() failed");
            }
            else if (processID == 0)
            {
                break;
            }
            else
            {
                childProcCount--; /* 子プロセスの数をデクリメント */
            }
        }
    }
}