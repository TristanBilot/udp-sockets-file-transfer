/**
 * @author Tristan Bilot : https://tristan-bilot.fr
 * @version 1.0 
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#define RECV_BUFF_SIZE 100
#define SEND_BUFF_SIZE 1024
#define PORT_NO 4444
#define NOT_FOUND "Not found"
int main(int argc, char *argv[])
{
    DIR *dir;
    struct dirent *ent;
    FILE *fp;
    char file_Name[200];
    strcpy(file_Name,".");
    int long buffer[1];
    
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  struct sockaddr_in sa;                    //define sa 
  char recvBuff[RECV_BUFF_SIZE];            //buffer of the received file 
  ssize_t recsize;
  socklen_t fromlen;
  char sendBuff[SEND_BUFF_SIZE];              //buffer of the sended file 
  int numrv;

  memset(sendBuff, 0, sizeof(sendBuff));

  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(PORT_NO);
  fromlen = sizeof(sa);

  if (-1 == bind(sock, (struct sockaddr *)&sa, sizeof sa)) {
    perror("error bind failed");
    close(sock);
    return 1;
  }



  for (;;) {
      
      recsize = recvfrom(sock, (void*)recvBuff, sizeof recvBuff, 0, (struct sockaddr*)&sa, &fromlen);//recv the file name
     
      if (recsize < 0) {
         fprintf(stderr, "%s\n", strerror(errno));
         return 1;
          }

        if ((dir = opendir(argv[1])) != NULL) {             //open directory
           
            while ((ent = readdir (dir)) != NULL) {         //read the directory   
                strcat(file_Name,ent->d_name);              //put all file names into a string
            }
            closedir (dir);                                 //close directory 
          } else { 
            perror ("");
            return EXIT_FAILURE;
          }
        
              if(strstr(file_Name,recvBuff ) != NULL) {         //match, file found
                  printf("Requested filename: %.*s\n", (int)recsize, recvBuff);
                  char str[100];                                // file path ./dir/file
                  strcpy(str,"./");
                  strcat(str,argv[1]);
                  strcat(str,"/");
                  strcat(str,recvBuff);
                  fp = fopen(str,"r");                          // open file
                  if(fp==NULL){                                 //file open error 
                      printf("File opern error");
                      return 1;
                  }
                  fseek(fp, 0, SEEK_END);                        //go through the file 
                  int long size = ftell(fp);                     //get the size of the file
                  rewind(fp);  
                  buffer[0]=size;                                //put the size of the file into buffer 
                  sendto(sock,buffer, sizeof buffer, 0,(struct sockaddr*)&sa, sizeof sa);//send the size of the file to client
                  while(1)
                  {
                      /* Read file in chunks of 1024 bytes */   
                      int nread = fread(sendBuff,1,SEND_BUFF_SIZE,fp);
                      printf("Bytes read %d \n", nread);
                      /* If read was success, send data. */
                      if(nread > 0) {
                          printf("Sending the file ...\n");
                          int n = sendto(sock,sendBuff, strlen(sendBuff), 0,(struct sockaddr*)&sa, sizeof sa);  
                          if(n<0){
                                         perror("Problem sendto\n");
                                         exit(1);
                                     }
                                     memset(sendBuff, 0, sizeof(sendBuff));               //clear the send buffer 
                      }
                      if (nread < SEND_BUFF_SIZE){
                          if (feof(fp))
                              printf("End of file\n");
                          if (ferror(fp))
                              printf("Error reading the file at server program\n");
                          break;
                      }
                  }
              }else{                                                                       //set file size to be -1
                buffer[0]=-1;
                sendto(sock,buffer, sizeof buffer, 0,(struct sockaddr*)&sa, sizeof sa);    //send the size of the file 
                strcpy(sendBuff, NOT_FOUND);                                                //file not found 
                sendto(sock,sendBuff, strlen(sendBuff), 0,(struct sockaddr*)&sa, sizeof sa);// send file not found message 
               
                
             }
           
  }

close(sock);
return 0;

}