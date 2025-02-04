#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>


/* エコー文字列の最大長 */
#define ECHOMAX 255

/* エラーハンドリング関数 */
void DieWithError(const char *errorMessage);
/* UDPエコーサーバーとは別の処理をする関数 */
void UseIdleTime();
/* SIGIOを処理するシグナルハンドラ */
void SIGIOHandler(int signalType);


/* ソケットディスクリプタ */
int sock;

int main(int argc, char const *argv[])
{
    struct sockaddr_in echoServAddr; /* エコーサーバのアドレス */
    unsigned short echoServPort;     /* エコーサーバのポート */
    struct sigaction handler;        /* シグナルハンドラ */

    /* 引数の数が正しいか確認 */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }

    /* 第1引数: サーバのポート */
    echoServPort = atoi(argv[1]); 

    /* ソケットの作成 */
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        DieWithError("socket() failed");
    }

    /* サーバのアドレス構造体を作成 */
    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;  /* インターネットアドレスファミリ */   
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* サーバのIPアドレス */
    echoServAddr.sin_port = htons(echoServPort); /* サーバのポート */

    /* ソケットにアドレスをバインド */
    if(bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("bind() failed");
    }

    /* シグナルハンドラを設定 */
    /* 全てのシグナルをマスク（ブロック）する */
    handler.sa_handler = SIGIOHandler;
    if(sigfillset(&handler.sa_mask) < 0)
    {
        DieWithError("sigfillset() failed");
    }
    handler.sa_flags = 0;

    /* 割り込みシグナルだけを受け入れる */
    if(sigaction(SIGIO, &handler, 0) < 0)
    {
        DieWithError("sigaction() failed for SIGIO");
    }

    /* ソケットを所有してSIGIOメッセージを受信する 
        fcntl: ファイルディスクリプタを操作する関数
        getpid: 呼び出し元のプロセスのプロセス ID を返す
        
        このコードを実行すると、現在のプロセスをソケットの所有者として設定することで、
        非同期シグナルを受信できるようにできる */
    if(fcntl(sock, F_SETOWN, getpid()) < 0)
    {
        DieWithError("Unable to set process owner to us");
    }

    /* ソケットを非ブロッキングモードに設定 */
    if (fcntl(sock, F_SETFL, O_NONBLOCK | fcntl(sock, F_GETFL)) < 0)
    {
        DieWithError("Unable to put client sock into nonblocking mode");
    }

    /* 処理を離れて他の仕事をする */
    for(;;)
    {
        UseIdleTime();
    }
}

void UseIdleTime()
{
    printf(".");
    sleep(3);
}

void SIGIOHandler(int signalType)
{
    struct sockaddr_in echoClntAddr; /* データグラムの送信先アドレス */
    unsigned int clntlen;            /* クライアントアドレスの長さ */   
    int recvMsgSize;                 /* 受信メッセージのサイズ */
    char echoBuffer[ECHOMAX];        /* エコーバッファ */

    /* 入力がなくなるまで */
    do
    {
        clntlen = sizeof(echoClntAddr);

        /* クライアントからのメッセージを受信 */
        if((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *)&echoClntAddr, &clntlen)) < 0)
        {
            /* エラーが発生していない場合 */
            if(errno != EWOULDBLOCK)
            {
                DieWithError("recvfrom() failed");
            }
        }
        else
        {
            printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

            /* 受信したメッセージをクライアントにエコーバック */
            if(sendto(sock, echoBuffer, recvMsgSize, 0, (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
            {
                DieWithError("sendto() sent a different number of bytes than expected");
            }
        }
    } while (clntlen >= 0);
}

void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}