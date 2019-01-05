/**
 * @author Tristan Bilot : https://tristan-bilot.fr
 * @version 1.0 
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define SEND_BUFF_SIZE 100
#define RECV_BUFF_SIZE 1024
#define SERV_PORT_NO 4444
#define SERV_IP_ADDR  "127.0.0.1"
#define FILE_NAME "./files/sample_file.txt"
int main(int argc, char *argv[])
{
    int sockfd = 0;
    int  bytesReceived = 0;
    int bytesSent = 0;
    int long sizeReceived=0;
    int sum_bytes=0;
    ssize_t recsize;
    char sendBuff[SEND_BUFF_SIZE];
    int long sizeBuff[1];
    char recvBuff[RECV_BUFF_SIZE];
    struct sockaddr_in sa;
    socklen_t length;
    /* Create a socket first */
    if((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    /* Zero out socket address */
    memset(&sa, 0, sizeof sa);
    /* Initialize sockaddr_in data structure */
    sa.sin_family = AF_INET;
    sa.sin_port = htons(SERV_PORT_NO); // port
    sa.sin_addr.s_addr = inet_addr(SERV_IP_ADDR);
    length = sizeof sa;

    strcpy(sendBuff, argv[1]);  
    /*Send the filename*/

    bytesSent = sendto(sockfd,sendBuff, sizeof sendBuff, 0,(struct sockaddr*)&sa, sizeof sa);
    if (bytesSent < 0) {
        printf("Sendto: Error sending the file name: %s\n", strerror(errno));
        return 1;
      }    

    //  printf("Filename sent!\n");    

    /* Create file where data will be stored */
    FILE *fp;
    fp = fopen(FILE_NAME, "w");                             //open the file for writing 
    if(NULL == fp)
    {
        printf("Error opening the file");
        return 1;
    }

    //printf("Begin reciving the file...\n"); 
    
    recvfrom(sockfd,sizeBuff,sizeof sizeBuff ,0,  (struct sockaddr *)&sa, &length);
    if(sizeBuff[0]>0){                                                //file exist 
      while(sum_bytes<sizeBuff[0]){                                   //if file received is not the entire thing 
        /* Receive data in chunks of 1024  bytes */       
        bytesReceived =recvfrom(sockfd,recvBuff,sizeof recvBuff ,0,  (struct sockaddr *)&sa, &length);
        if (bytesReceived<0)
        { 
          printf("Recvfrom: Error in recieving the file\n");
          exit(1);
        }
        else
        {   
          sum_bytes = sum_bytes+bytesReceived;                         //sum up all the received bytes 
        //  printf("Number of bytes received: %d\n", sum_bytes);
        }
        if(fwrite(recvBuff,1,bytesReceived,fp)<0)                      //write into local file 
        {
          printf("Error writting the file\n");
          exit(1);
        }
        memset(recvBuff, 0, sizeof(recvBuff));                         //clear the receive buffer 
     }
    }else{                                                              //if the size of the file is -1, file not exist
      bytesReceived =recvfrom(sockfd,recvBuff,sizeof recvBuff ,0,  (struct sockaddr *)&sa, &length);   //receive 'Not found'
    //  printf("Number of bytes received: %d\n", bytesReceived);
    }
   fclose(fp);
   close(sockfd);
   return 0;
}