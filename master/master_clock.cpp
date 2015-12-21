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
	int s,m,h;
	float alfa;
	
	public:
	myClock(){
		s=0;
		m=0;
		h=0;
		alfa=1;	
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
		if (60==s){
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
		}
		
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
    int n,h,m,s;
     
	relogio.printTime();
	cout << "Hello World!\n";
	
	int i1 = pthread_create( &first, NULL, task, (void*) "thread 1");
	
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
    bzero(buffer,256);  
    while(1){   
		usleep(1000000);  
		s= relogio.get_s();
		h= relogio.get_h();
		m= relogio.get_m();	
	
		snprintf(buffer, 10, "%d:%d:%d", h,m,s);	
	
		n = write(newsockfd,buffer,strlen(buffer));     
		if (n < 0) error("ERROR writing to socket");
	}
	 pthread_join(first,NULL);
    
    close(newsockfd);
    close(sockfd);
}
