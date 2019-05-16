// https://github.com/FrankBau/raspi-repo-manifest/wiki/OpenCV
// g++ -W -Wall -pthread netcvc1.cpp -o netcvc.out -I/usr/local/include -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_videoio -lopencv_highgui

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <assert.h>
#include <string.h>
#include <ctime>


#include <iostream>
#include <pthread.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

#define SERVERPORT 8887
#define BUFFLEN 1452

VideoCapture    capture;

Mat             img0, img1;
int             is_data_ready = 0;
int             clientSock;
//char*     	server_ip;
//int       	server_port;
//vector<uchar> buf;
//char buf[BUFLEN];
std::vector<unsigned char> *ret = new std::vector<unsigned char>(BUFFLEN,'0');

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void* streamClient(void* arg);
void  quit(string msg, int retval);

void errno_abort(const char* header)
{
    perror(header);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    pthread_t   thread_c;
    int         key;

    //if (argc < 3) {
    //    quit("Usage: netcv_client <server_ip> <server_port> <input_file>(optional)", 0);
    //}
    //if (argc == 4) {
    //      capture.open(argv[3]);
    //} else {
    //      capture.open(0);
    //}

    // run the streaming client as a separate thread
    if (pthread_create(&thread_c, NULL, streamClient, NULL)) {
        quit("\n--> pthread_create failed.", 1);
    }

    cout << "\n--> Press 'q' to quit. \n\n" << endl;

    while(key != 'q') {
        /* get a frame from camera */
        //if (img0.empty()) break;

        pthread_mutex_lock(&mutex1);

        //flip(img0, img0, 1);
        pthread_mutex_unlock(&mutex1);

        usleep(500);
    }

    /* user has pressed 'q', terminate the streaming client */
    if (pthread_cancel(thread_c)) {
        quit("\n--> pthread_cancel failed.", 1);
    }

    /* free memory */
    //destroyWindow("stream_client");
    quit("\n--> NULL", 0);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * This is the streaming client, run as separate thread
 */
void* streamClient(void* arg)
{
    struct sockaddr_in send_addr;
    int trueflag = 1;
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        errno_abort("socket");
    //

    /* make this thread cancellable using pthread_cancel() */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &trueflag, sizeof trueflag) < 0)
    {
        errno_abort("setsockopt"); // one of them..
        quit("\n--> socket() failed.", 1);
    }

    memset(&send_addr, 0, sizeof send_addr);
    send_addr.sin_family = AF_INET;

    //htons() converts the unsigned short integer hostshort from host byte order to network byte order.
    send_addr.sin_port = (in_port_t) htons(SERVERPORT);

    //only to receive
    send_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //inet_aton() converts the Internet host address cp from the IPv4 numbers-and-dots notation into binary form (in network byte order) and stores it in the structure that inp points to.

    //if (inet_aton("192.168.2.117", &send_addr.sin_addr) == 0)
    //    std::cout<<"Inet Aton Failed"<<endl;
    if (inet_aton("127.0.0.1", &send_addr.sin_addr) == 0)
        std::cout<<"Inet Aton Failed"<<endl;
    int slen=sizeof(send_addr);
    socklen_t socksize = sizeof(send_addr);

    //std::string boundary = "--OPENCV_UDP_BOUNDARY \r\n";
    int counter;
    std::cout<<"Strating RecLoop"<<endl;
    char buf[BUFFLEN];
    if( bind(fd , (struct sockaddr*) &send_addr, slen ) == -1)
    {
        puts("bind");
    }
    vector<uchar> jpgbuf;
    /* start sending images */
    while(1)
    {

        //std::string header ="\r\nContent-type: image/jpeg\r\n";

        int i = 0;
        //

        //if (recvfrom(fd, &(*ret)[0], BUFFLEN, 0, (struct sockaddr*) &send_addr, &socksize ) < 0)
        int recSize = recvfrom(fd, buf, BUFFLEN, 0, (struct sockaddr*) &send_addr, &socksize );
        std::cout<<"RecSize: "<< recSize<<endl;
        if (recSize < 0)
        {
            //cerr << "\n--> bytes = " << bytes << endl;
            quit("\n--> send() failed", 1);
            errno_abort("send");
        }
        else if ( recSize == 1452 )
        {
            jpgbuf.insert(jpgbuf.end(), buf, buf+recSize);
        }
        else if (recSize < 1452)

        {
            //last frame
            jpgbuf.insert(jpgbuf.end(), buf, buf+recSize); //use vector to receive and only insert the part of buff which arrived..
            Mat decodedImage  =  imdecode( jpgbuf, 1 );
            if ( decodedImage.data == NULL )
            {
                puts("-----------------!!!!!!!!!!!!!!!!!!!!!!!!!!!!!decode failed");
            }
            else {
                imshow("ASASD", decodedImage);
            }
            jpgbuf.clear();

        }
        //we did it..
        //is_data_ready = 1;
        counter++;;
        //puts(buf);
        for(std::vector<unsigned char>::iterator it=ret->begin(); it<ret->end();it++)
        {

            //std::cout<<*it;
        }
        puts(buf);


    }
    pthread_mutex_lock(&mutex1);
    usleep(10);   //1 Micro Sec
    std::cout<<"Mutexwait"<<endl;
    pthread_mutex_unlock(&mutex1);

    cout << "\n This was the " << counter++ << "--> th package\n\n" <<endl;

    /* have we terminated yet? */
    pthread_testcancel();

    /* no, take a rest for a while */

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * this function provides a way to exit nicely from the system
 */
void quit(string msg, int retval)
{
    if (retval == 0) {
        cout << (msg == "NULL" ? "" : msg) << "\n" << endl;
    } else {
        cerr << (msg == "NULL" ? "" : msg) << "\n" << endl;
    }
    if (clientSock){
        close(clientSock);
    }
    if (capture.isOpened()){
        capture.release();
    }
    if (!(img0.empty())){
        (~img0);
    }
    if (!(img1.empty())){
        (~img1);
    }

    pthread_mutex_destroy(&mutex1);
    exit(retval);
}




