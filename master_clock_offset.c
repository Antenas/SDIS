#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

class myClock{
	long long int s,m,h;
	float alfa;
	
	public:
	myClock(){
		s=0;
		m=0;
		h=0;
		alfa=1;	
	}	
	void set_s(long long int i){
		s=i;	
	}
	int get_s(){
		return s;
	}
	int get_h(){
		return h;
	}
	int get_m(){
		return m;
	}
	void increment_1s(){
		s++;
		/*if (60==s){
			m++;
			s=0;
			if (60==m){
				h++;
				m=0;
				if (24==h)
				{
					h=0;					
				}
			}
		}*/
		
	}
	void correct_drift(float i){
		alfa=alfa+i; 
	}
	void printTime(){
		std::cout << h;
		std::cout << ":";
		std::cout << m;
		std::cout << ":";
		std::cout << s;
		std::cout << "\n";
	}
	void run(){
		for(; ;){
			increment_1s();
			printTime();
			usleep(1000000*alfa);
		}		
	}	
};

myClock relogio;

void *task(void *argument){
     relogio.run();
}

int main(int argc, char *argv[])
{
	
	using namespace std;
	pthread_t first;
	int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    long long int s;  

	
	pthread_create( &first, NULL, task, (void*) "thread 1");
	
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
             error("ERROR on binding");
	listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, 
                (struct sockaddr *) &cli_addr, 
                &clilen);
    if (newsockfd < 0) 
         error("ERROR on accept");
	
	relogio.printTime();
	s= relogio.get_s();	
	n = write(newsockfd,&s,sizeof(s));     
	if (n < 0) error("ERROR writing to socket");
	
	//offset 
	s= relogio.get_s();	
	n = write(newsockfd,&s,sizeof(s));     
	if (n < 0) error("ERROR writing to socket");
	cout <<"\nFollow-up:"<< s <<"\n";		
	usleep(5000000);	
	n = read(newsockfd,&s,sizeof(s));
	if (n < 0) error("ERROR reading from socket");
	cout <<"\nDelay Request:"<< s <<"\n";
		
	s= relogio.get_s();
	n = write(newsockfd,&s,sizeof(s));     
	if (n < 0) error("ERROR writing to socket");
	cout <<"\nDelay Response:"<< s <<"\n";			
	
	usleep(5000000);		
	s= relogio.get_s();	
	n = write(newsockfd,&s,sizeof(s));     
	if (n < 0) error("ERROR writing to socket");
			
	s= relogio.get_s();		
	n = write(newsockfd,&s,sizeof(s));     
	if (n < 0) error("ERROR writing to socket");
	cout <<"\nFollow-up:"<< s <<"\n";
	//
	
	relogio.printTime();
	pthread_join(first,NULL);
    
    close(newsockfd);
    close(sockfd);
}
