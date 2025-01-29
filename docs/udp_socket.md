# UDP

UDPは以下の機能しか提供しない。

1. IP層にもう1つの別のアドレッシングの層を追加する
2. 転送中に発生し得るデータの破損を検出し、破損したデータグラムを破棄する。


UDPはTCPと違い、接続の確率は不要である。UDPソケットは、作成後すぐにメッセージの送受信に使用することが出来る。このとき、メッセージは任意のアドレスから複数のアドレスへ連続的に送ることが可能である。各メッセージに宛先アドレスを指定するため、ソケットAPIにはUDPソケットで汎用的に使用できるメッセージ関数が容易されている。

```c
int sendto(int socket, const void *message, unsigned int length, int flags, struct sockaddr *to, unsigned int toLength);
int ssize_t recvfrom(int socket, void *msg, unsigned msglen, int flags, struct sockaddr from, unsigned int *fromlen);
```

- sendto()の最初の４つのパラメータはsend()と同じである。残り２つのパラメータは、メッセージの送信先を指定する。
- recvfrom()もrecv()と同じパラメータを受け取るが、加えて、受信したデータグラムの送信先（送信関数の呼び出し元）に関する情報を、2つのパラメータで取得できるようになっている。

## UDPエコークライアント

```c
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
```

## UDPサーバー

```c
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
```

## UDPソケットによるデータの送受信

### UDPのsendto()とsend()

UDPは **「メッセージ指向」** のプロトコルであり、送信時のデータのまとまり（パケット）の単位が維持されます。sendto() で送信したデータが recvfrom() で受信する際に分割されたり、結合されたりすることはない。

- 例: sendto() で 100 バイトのデータを送信した場合、recvfrom() で一度に 200 バイト受け取ることはない。
- 逆に sendto() で 100 バイト送ったデータを recvfrom() で 2 回に分けて 50 バイトずつ受信することもない。
- ただし、MSG_PEEK フラグを使う場合は例外

MSG_PEEK は「データを読み取るが、バッファから削除しない」ため、同じデータを繰り返し recvfrom() で読むことができる。UDP は「1 つの sendto() ＝ 1 つの recvfrom()」が基本です。

### TCPのsendto()とsend()

一方、TCP は **「ストリーム指向」** のプロトコルであり、データの境界を保持しません。send() の戻り値は「データがカーネルバッファにコピーされた」ことを意味する。

send() を呼び出した時点では、データが実際に相手に送信されたかどうかは分からない。カーネル内部の送信バッファにコピーされた後、OSのネットワークスタックによって送信される。送信したデータが受信側で recv() するときに分割される可能性がある

- 例: send() で 100 バイト送信しても、受信側が recv(50) を呼び出すと 50 バイトだけ読み取り、次回の recv(50) で残りを受け取ることになる。
- TCP では送信時にデータの分割・結合が起こるため、受信側はデータの境界を意識しなければならない。