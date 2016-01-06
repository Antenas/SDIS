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
#include <signal.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void warning(const char *msg)
{
    perror(msg);
    return ;
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
		//system("clear");
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

void *task_correct(void* i){
	
	using namespace std;
	int sockfd,newsockfd;
	int portno = *((int*) i);
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    long long int s; 
	
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGPIPE);
    pthread_sigmask(SIG_BLOCK,&set,NULL);
    
	 cout <<"\nSTART\n";		
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0){ 
        warning("ERROR opening socket");
        return 0;
	}
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) {
             warning("ERROR on binding");
             return 0;
             }
     listen(sockfd,5);
	 clilen = sizeof(cli_addr);
	 newsockfd = accept(sockfd, 
				(struct sockaddr *) &cli_addr, 
				&clilen);
	 if (newsockfd < 0) 
		warning("ERROR on accept");
            
	relogio.printTime();	
			
	cout <<"\nCOMEÇA AQUI\n";
	while(1){
		s= relogio.get_s();	
		//sync
		n = write(newsockfd,&s,sizeof(s));     
		if (n < 0){ warning("ERROR writing to socket");break;}
		
		//follow-up
		n = write(newsockfd,&s,sizeof(s));     
		if (n < 0){ warning("ERROR writing to socket");break;}
		
		//delay request
		n = read(newsockfd,&s,sizeof(s));
		if (n < 0){ warning("ERROR reading from socket");break;}
				
		//delay response 
		s= relogio.get_s();	
		n = write(newsockfd,&s,sizeof(s));     
		if (n < 0){ warning("ERROR writing to socket");break;}		
		
		//sync
		s= relogio.get_s();
		n = write(newsockfd,&s,sizeof(s));     
		if (n < 0){ warning("ERROR writing to socket");break;}
		
		//follow-up	
		n = write(newsockfd,&s,sizeof(s));     
		if (n < 0){ warning("ERROR writing to socket");break;}		
	}	
    close(newsockfd);
    close(sockfd);
	return 0;
}

int main(int argc, char *argv[])
{
	
	using namespace std;
	pthread_t first[10];
	int sockfd, newsockfd, portno,newport,i=1;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    long long int s;  
	pthread_create( &first[0], NULL, task, (void*) "thread 1");
	
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
    while(1){
		listen(sockfd,5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, 
					(struct sockaddr *) &cli_addr, 
					&clilen);
		if (newsockfd < 0){ 
			warning("ERROR on accept"); continue;}
		newport = portno + i;		
        write(newsockfd,&newport,sizeof(newport));
		pthread_create( &first[i], NULL, task_correct,(void*) &newport);		
		i++;
	}
		
	
	relogio.printTime();
	for(i=0;i<10;i++){
	pthread_join(first[i],NULL);
	}   
    close(newsockfd);
    close(sockfd);
}
