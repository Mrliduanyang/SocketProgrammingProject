#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <iomanip>

using namespace std;

#define HOST "127.0.0.1"
#define AWS_TCP_PORT 24233

// define send buffer for aws
struct SendBufForAws {
    char mapID[20];
    long sourceNode;
    long fileSize;
};

// define receive buffer for aws
struct RecvBufForAws {
    double result[10][5];
};

int main(int argc, char *argv[]) {
    if (argc != 4) {
        exit(1);
    }
    cout << "The client is up and running." << endl;

    string mapID = argv[1];
    long sourceNode = stol(argv[2]);
    long fileSize = stol(argv[3]);

    // init send and recv buffer for client
    struct SendBufForAws sendBufForAws;
    struct RecvBufForAws recvBufForAws;
    strcpy(sendBufForAws.mapID, mapID.c_str());
    sendBufForAws.sourceNode = sourceNode;
    sendBufForAws.fileSize = fileSize;

    // bind port to socket
    int clientSocket;
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        exit(EXIT_FAILURE);
    }
    // establish a tcp connection with aws
    struct sockaddr_in awsTcpAddr, clientTcpAddr;
    memset(&awsTcpAddr, 0, sizeof(awsTcpAddr));
    memset(&clientTcpAddr, 0, sizeof(clientTcpAddr));
    awsTcpAddr.sin_family = AF_INET;
    awsTcpAddr.sin_port = htons(AWS_TCP_PORT);
    awsTcpAddr.sin_addr.s_addr = inet_addr(HOST);
    if (connect(clientSocket, (struct sockaddr *) &awsTcpAddr, sizeof(awsTcpAddr)) < 0) {
        exit(EXIT_FAILURE);
    }
    // get the tcp port of client
    socklen_t clientTcpAddrLen = sizeof(clientTcpAddr);
    getsockname(clientSocket, (struct sockaddr *) &clientTcpAddr, &clientTcpAddrLen);

    // send data to aws
    send(clientSocket, &sendBufForAws, sizeof(sendBufForAws), 0);
    cout
            << "The client has sent query to AWS using TCP over port <" << ntohs(clientTcpAddr.sin_port) << ">: "
            << "start vertex <" << sourceNode << ">; "
            << "map <" << mapID << ">; "
            << "file size <" << fileSize << ">."
            << endl;

    // receive aws's send data
    recv(clientSocket, &recvBufForAws, sizeof(recvBufForAws), 0);
    cout << "The client has received results from AWS:" << endl
         << "--------------------------------------------------" << endl
         << "Destination    Min Length    Tt      Tp      Delay" << endl;

    for (auto item:recvBufForAws.result) {
        if (item[1] != 0) {
            cout << left << setw(15) << (long) item[0]
                 << setw(14) << (long) item[1]
                 << setw(8) << fixed << setprecision(2) << item[2]
                 << setw(8) << setprecision(2) << item[3]
                 << setw(5) << setprecision(2) << item[4] << endl;
        }
    }
    cout << "--------------------------------------------------" << endl;
    close(clientSocket);
    return 0;
}
