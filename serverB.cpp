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
#define SERVER_B_UDP_PORT 22233

// define send buffer for aws
struct SendBufForAws {
    double delay[10][4];
};

// define receive buffer for aws
struct RecvBufForAws {
    double propSpeed;
    double transSpeed;
    long fileSize;
    long shortestPath[10][2];
};

int main() {
    // init send and recv buffer for aws
    struct SendBufForAws sendBufForAws;
    struct RecvBufForAws recvBufForAws;

    // init socket for serverA to aws
    int serverBSocket;
    if ((serverBSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        exit(EXIT_FAILURE);
    }

    // bind port to socket
    struct sockaddr_in serverBUdpAddr, awsUdpAddr;
    memset(&serverBUdpAddr, 0, sizeof(serverBUdpAddr));
    memset(&awsUdpAddr, 0, sizeof(awsUdpAddr));
    serverBUdpAddr.sin_family = AF_INET;
    serverBUdpAddr.sin_port = htons(SERVER_B_UDP_PORT);
    serverBUdpAddr.sin_addr.s_addr = inet_addr(HOST);
    if (bind(serverBSocket, (struct sockaddr *) &serverBUdpAddr, sizeof(serverBUdpAddr)) < 0) {
        exit(EXIT_FAILURE);
    }
    cout << "The Server B is up and running using UDP on port <" << SERVER_B_UDP_PORT << ">." << endl;

    // start listening service
    while (true) {
        // receive aws's send data
        socklen_t awsUdpAddrLen = sizeof(awsUdpAddr);
        memset(recvBufForAws.shortestPath, 0, sizeof(recvBufForAws.shortestPath));
        recvfrom(serverBSocket, &recvBufForAws, sizeof(recvBufForAws), 0, (
                struct sockaddr *) &awsUdpAddr, &awsUdpAddrLen);
        double propSpeed = recvBufForAws.propSpeed;
        double transSpeed = recvBufForAws.transSpeed;
        long fileSize = recvBufForAws.fileSize;
        long shortestPath[10][2];
        memcpy(shortestPath, recvBufForAws.shortestPath, sizeof(recvBufForAws.shortestPath));

        cout << "The Server B has received data for calculation:" << endl
             << "* Propagation speed: <" << fixed << setprecision(2) << propSpeed << "> km/s;" << endl
             << "* Transmission speed <" << fixed << setprecision(2) << transSpeed << "> Bytes/s;" << endl;
        for (auto item:shortestPath) {
            if (item[1] != 0) {
                cout << "* Path length for destination <" << item[0] << ">: <" << item[1] << ">;" << endl;
            }
        }
        cout << "The Server B has finished the calculation of the delays:" << endl
             << "------------------------" << endl
             << "Destination        Delay" << endl
             << "------------------------" << endl;

        // calculate the delay based on shortest path
        long shortestPathIndex = 0;
        memset(sendBufForAws.delay, 0, sizeof(sendBufForAws.delay));
        for (auto item:shortestPath) {
            if (item[1] != 0) {
                double propDelay = item[1] / propSpeed;
                double transDelay = fileSize / transSpeed / 8;
                sendBufForAws.delay[shortestPathIndex][0] = item[0];
                sendBufForAws.delay[shortestPathIndex][1] = transDelay;
                sendBufForAws.delay[shortestPathIndex][2] = propDelay;
                sendBufForAws.delay[shortestPathIndex][3] = transDelay + propDelay;
                cout << left << setw(19) << item[0] << setw(10) << setprecision(2) << propDelay + transDelay << endl;
                shortestPathIndex += 1;
            }
        }
        // send shortest path to aws
        sendto(serverBSocket, &sendBufForAws, sizeof(sendBufForAws), 0,
               (const struct sockaddr *) &awsUdpAddr, awsUdpAddrLen);
        cout << "------------------------" << endl
             << "The Server B has finished sending the output to AWS" << endl;
    }
}
