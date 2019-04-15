/**********************************
udp_ser.c: the source file of the server in udp transmission
***********************************/

#include "headsock.h"

void str_ser(int sockfd, struct sockaddr *addr, int addrlen);             // transmitting and receiving function

int main(void)
{
	int sockfd, ret;
	struct sockaddr_in my_addr;
  
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);          //create socket
	if (sockfd<0)
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
	if (ret<0)
	{
		printf("error in binding");
		exit(1);
	}
	
	printf("waiting for data\n");
	str_ser(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));                //receive packet and response
  
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd, struct sockaddr *addr, int addrlen)
{
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[4*DATALEN];
	struct ack_so ack;
	int end, n = 0;
	long lseek=0;
	end = 0;

	while(!end)
	{
   
		if ((n= recvfrom(sockfd, &recvs, 4*DATALEN, 0, addr, (socklen_t *)&addrlen))==-1)  //receive the packet
		{
			printf("error when receiving\n");
			exit(1);
		}
			
		if (recvs[n-1] == '\0')									//if it is the end of the file
		{
		  end = 1;
		  n --;
		}
		memcpy((buf+lseek), recvs, n);
		lseek += n;
		
		ack.num = 1;
		  ack.len = 0;
		  if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen))==-1)
		  {
			  printf("send error!");								//send the ack
			  exit(1);
		  }
	}

	if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exist\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}
