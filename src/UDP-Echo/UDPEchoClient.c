#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* エコーの最大文字列数 */
#define ECHOMAX 255

/* エラー処理関数 */
void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char const *argv[])
{
    int sock;                        /* ソケットディスクリプタ */
    struct sockaddr_in echoServAddr; /* エコーサーバのアドレス */
    struct sockaddr_in fromAddr;     /* 受信元アドレス */
    unsigned short echoServPort;     /* エコーサーバのポート */
    unsigned int fromSize;           /* 受信元アドレス構造体のサイズ */
    char *servIP;                    /* サーバのIPアドレス */
    char *echoBuffer[ECHOMAX + 1];   /* エコーサーバから受信するデータ */
    char *echoString;                /* エコーサーバに送信する文字列 */
    int echoStringLen;               /* エコーサーバに送信する文字列の長さ */
    int respStringLen;               /* 受信した文字列の長さ */

    /* 引数の数が正しいか確認 */
    if ((argc < 3) || (argc > 4))
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];     /* サーバーのIPアドレス */
    echoString = argv[2]; /* エコー文字列 */

    if ((echoStringLen = strlen(echoString)) > ECHOMAX)
    {
        DieWithError("Echo word too long");
    }

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
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* エコーサーバのアドレス構造体を作成 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* 構造体をゼロで初期化 */
    echoServAddr.sin_family = AF_INET;                /* インターネットアドレスファミリ */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* サーバのIPアドレス */
    echoServAddr.sin_port = htons(echoServPort);      /* サーバのポート */

    /* エコーサーバにエコー文字列を送信 */
    if (sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != echoStringLen)
    {
        DieWithError("sendto() sent a different number of bytes than expected");
    }

    /* サーバからの応答を受信 */
    fromSize = sizeof(fromAddr);
    if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *)&fromAddr, &fromSize)) < 0)
    {
        DieWithError("recvfrom() failed");
    }

    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr, "Error: received a packet from unknown source.\n");
        exit(1);
    }

    /* NULL文字を追加して文字列を完成させる */
    echoBuffer[respStringLen] = '\0';
    printf("Received: %s\n", echoBuffer);

    close(sock);

    return 0;
}
