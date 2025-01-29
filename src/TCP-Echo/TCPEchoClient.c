#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 受信バッファサイズ */
#define RCVBUFSIZE 32

/* エラー処理関数 */
void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;                        /* ソケットディスクリプタ */
    struct sockaddr_in echoServAddr; /* エコーサーバのアドレス */
    unsigned short echoServPort;     /* エコーサーバのポート */
    char *servIP;                    /* サーバのIPアドレス */
    char *echoString;                /* エコーサーバに送信する文字列 */
    char echoBuffer[RCVBUFSIZE];     /* エコーサーバから受信するデータ */
    unsigned int echoStringLen;      /* エコーサーバに送信する文字列の長さ */
    int bytesRcvd, totalBytesRcvd;   /* エコーサーバから受信したバイト数 */

    /* 引数の数が正しいか確認 */
    if ((argc < 3) || (argc > 4))
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];     /* １つ目の引数：サーバのIPアドレス */
    echoString = argv[2]; /* ２つ目の引数：エコーサーバに送信する文字列 */

    /* エコーサーバのポートを指定するかどうかを確認 */
    if (argc == 4)
    {
        echoServPort = atoi(argv[3]);
    }
    else
    {
        echoServPort = 7;
    }

    /* ソケットの作成 */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* エコーサーバのアドレス構造体を作成 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 構造体をゼロで初期化 */
    echoServAddr.sin_family = AF_INET;                /* インターネットアドレスファミリ */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* サーバのIPアドレス */
    echoServAddr.sin_port = htons(echoServPort);      /* サーバのポート */

    /* サーバに接続 */
    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("connect() failed");
    }

    /* エコーサーバに送信する文字列の長さ */
    echoStringLen = strlen(echoString);

    /* データを送信 */
    if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
    {
        DieWithError("send() sent a different number of bytes than expected");
    }

    /* サーバからのエコーを受信 */
    totalBytesRcvd = 0;
    printf("Received: ");
    while (totalBytesRcvd < echoStringLen)
    {
        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        {
            DieWithError("recv() failed or connection closed prematurely");
        }
        totalBytesRcvd += bytesRcvd;  /* 受信したバイト数を加算 */
        echoBuffer[bytesRcvd] = '\0'; /* 文字列の終端を追加 */
        printf("%s", echoBuffer);     /* 受信した文字列を表示 */
    }

    printf("\n");

    close(sock); /* ソケットをクローズ */
    return 0;
}
