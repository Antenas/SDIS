#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

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
		alfa=0.9;	
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
	pthread_t first,second;
	int i,h,m,s;
	
	int sockfd, portno, n=0;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    
	//relogio.printTime();
	cout << "Hello World!\n";
	
	int i1 = pthread_create( &first, NULL, task, (void*) "thread 1");
	
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
   /* printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");*/
    while(1){
		
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) 
			error("ERROR reading from socket");
		cout <<"\nhora:"<< buffer <<"\n";
	}
	
	
	/*for(i=0;i<10;i++){
	int i1 = pthread_create( &second, NULL, correct, (void*) "thread 2");
	usleep(1000000);
	cout << i <<"\n";
	}
	pthread_cancel(first);
	pthread_cancel(second);*/
    pthread_join(first,NULL);
    
    //close(sockfd);
}
