/*******************************
assignment_udp_client.c: the source file of the client in udp transmission 
********************************/

#include "headsock.h"

float str_cli(FILE *fp, int sockfd, long *len, struct sockaddr *addr, int addrlen);                  //transmission function
void tv_sub(struct  timeval *out, struct timeval *in);	    //calcu the time interval between out and in

int main(int argc, char **argv)
{
	int sockfd;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc != 2) {
		printf("parameters not match");
	}

	sh = gethostbyname(argv[1]);	                                       //get host's information
	if (sh == NULL) {
		printf("error when gethostby name");
		exit(0);
	}

	printf("canonical name: %s\n", sh->h_name);					//print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);                           //create the socket
	if (sockfd <0)
	{
		printf("error in socket");
		exit(1);
	}
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);
	
	if((fp = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}

	//ti = str_cli(fp, sockfd, &len);                       //perform the transmission and receiving
	ti = str_cli(fp, sockfd, &len, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in));   // receive and send
	rt = (len/(float)ti);                                         //caculate the average transmission rate
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int)len, rt);

	close(sockfd);
	fclose(fp);

	exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len, struct sockaddr *addr, int addrlen)
{
	char *buf;
	long lsize, ci;
	char sends[DATALEN];
	struct ack_so ack;
	int n, slen;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;
	int numDU = 0; // whether to send 1, 2, 3, or 4 packets
					// When numDU == 0 , send 1, when numDU == 1, send 2...

	n = 0;
	
	fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n",DATALEN);

// allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL) exit (2);

  // copy the file into the buffer.
	fread (buf,1,lsize,fp);

  /*** the whole file is loaded in the buffer. ***/
	buf[lsize] ='\0';									//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time
	while(ci<= lsize)
	{
		// numDU starts from zero
		// so actual number of packets sent == numDU + 1
		for (int i=0; i<= numDU; i++) {
			if ((lsize+1-ci) <= DATALEN)
				slen = lsize+1-ci;
			else 
				slen = DATALEN;
			memcpy(sends, (buf+ci), slen);
			//n = send(sockfd, &sends, slen, 0);
			sendto(sockfd, &sends, slen, 0, addr, addrlen);                         //send the packet to server
			if(n == -1) {
				printf("send error!");								//send the data
				exit(1);
			}
			ci += slen;
		}
		
		if ((n=recvfrom(sockfd, &ack, 2, 0, addr, (socklen_t *)&addrlen)) == -1) {      //receive the packet
			printf("error receiving");
			exit(1);
		}
		
		if (ack.num != 1|| ack.len != 0)
			printf("error in transmission\n");

		// Make sure numDU varies between 0, 1, 2, 3
		numDU++;
		numDU %= 4;
	}
	
	gettimeofday(&recvt, NULL);
	*len= ci;                                                         //get current time
	tv_sub(&recvt, &sendt);                                                                 // get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}
