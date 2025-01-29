#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* エコー文字列の最大長 */
#define ECHOMAX 255

/* エラー処理関数 */
void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char const *argv[])
{
    int sock;                        /* ソケット */
    struct sockaddr_in echoServAddr; /* エコーサーバのアドレス */
    struct sockaddr_in echoClntAddr; /* クライアントのアドレス */
    unsigned short cliAddrLen;       /*着信メッセージの長さ */
    char echoBuffer[ECHOMAX];        /* エコーバッファ */
    unsigned short echoServPort;     /* サーバのポート */
    int recvMsgSize;                 /* 受信メッセージのサイズ */

    /* 引数の数が正しいか確認 */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    /* 1つ目の引数: サーバのポート */
    echoServPort = atoi(argv[1]);

    /* データグラムの送受信に使うソケットを作成 */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* ローカルアドレス構造体を作成 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 構造体をゼロで埋める */
    echoServAddr.sin_family = AF_INET;                /* インターネットアドレスファミリ */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 任意のローカルアドレス */
    echoServAddr.sin_port = htons(echoServPort);      /* サーバのポート */

    /* ソケットにアドレスをバインド */
    if (bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("bind() failed");
    }

    for (;;)
    {
        /* 入出力パラメータセット */
        cliAddrLen = sizeof(echoClntAddr);

        /* クライアントからメッセージを受信するまでブロックする */
        if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *)&echoClntAddr, &cliAddrLen)) < 0)
        {
            DieWithError("recvfrom() failed");
        }

        printf("Handling clinet %s\n", inet_ntoa(echoClntAddr.sin_addr));

        /* 受信したメッセージをクライアントにエコーバック */
        if (sendto(sock, echoBuffer, recvMsgSize, 0, (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
        {
            DieWithError("sendto() failed");
        }
    }

    return 0;
}
