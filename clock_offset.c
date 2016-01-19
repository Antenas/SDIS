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
#include <fstream>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

class myClock{
	long long int s,m,h, ms;
	float alfa;
	
	public:
	myClock(){
		s=0;
		m=0;
		h=0;
		ms=0;
		alfa=1;	
	}
	void set_s(long long int i){
		s=i;	
	}
	int get_s(){
		return s;
	}
	int get_ms(){
		return ms;
	}
	int get_h(){
		return h;
	}
	int get_m(){
		return m;
	}
	int get_alfa(){
		return alfa;
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
	void increment_ms(){
		ms++;
		if (1000==ms) {
			ms=0;
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
		
	}
	long long get_count(){
		return ms+1000*s+1000*60*m+1000*60*60*h;
	}
	void correct_drift(float i){
		alfa=alfa+i; 
		std::cout << "alfa: " << alfa << "\n";
	}
	void printTime(){
		//system("clear");
		std::cout << h;
		std::cout << ":";
		std::cout << m;
		std::cout << ":";
		std::cout << s;
		std::cout << ":";
		std::cout << ms;
		std::cout << "\n";
	}
	void run(){
		for(; ;){
			increment_ms();
			//printTime();
			usleep(1000*alfa);
		}		
	}	
};

myClock relogio;

void *task(void *argument){	
     relogio.run();
}

struct clock_data {
		long long tm1;
		long long ts1;
		long long delay;
};

int main(int argc, char *argv[])
{	
	using namespace std;
	pthread_t first,second;
	int i,delay,offset;
	long long int s=0, ts2=0, ts1=0, erro=0;
	int flag=0;
	int sockfd, portno, n=0;
	int jitter;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
	int cycle=0;
	clock_data info_rel[2000]={0};

    char buffer[256];
    
    ofstream outFile;
    outFile.open("output.txt");
    
    
	//relogio.printTime();
	cout << "Hello World!\n";
	
	pthread_create( &first, NULL, task, (void*) "thread 1");
	
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
		
			
	n = read(sockfd,&s,sizeof(s));
	if (n < 0) error("ERROR reading from socket");	
	cout << s;
	close(sockfd);
	
	portno = s;
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
		
	
	while(1){
		//usleep(relogio.get_alfa()*5000000);
				
		n = read(sockfd,&s,sizeof(s));
		ts1=relogio.get_count();
		if (n < 0) error("ERROR reading from socket");	
		//cout << "SyncMessage" << s;
		n = read(sockfd,&s,sizeof(s));
		if (n < 0) error("ERROR reading from socket");
		offset=ts1-s;	
		if (cycle <2000){
			info_rel[cycle].ts1=ts1;
			info_rel[cycle].tm1=s;	
		}
		//cout << "Offset" << offset;
		usleep(relogio.get_alfa()*1000000);//sleep 1s
		//offset
		ts2= relogio.get_count();
		jitter=rand()%100+1;
		usleep(10000*jitter);
		n = write(sockfd,&s,sizeof(s));	
		if (n < 0) error("ERROR writing to socket");
		//cout <<"\nDelay Request:"<< s <<"\n";		
		//offset=s;	
		//n = read(sockfd,&s,sizeof(s));
		//if (n < 0) error("ERROR reading from socket");
			
		n = read(sockfd,&s,sizeof(s));
		if (n < 0) error("ERROR reading from socket");		
		delay=(ts2-offset)-s;//s=tm2
		delay=delay/2;
		if (cycle <2000){
			info_rel[cycle].delay=delay;
			//printf("ts1=%lld\ntm1=%lld\ndelay=%lld\n",info_rel[cycle].ts1,info_rel[cycle].tm1, info_rel[cycle].delay	);
			cycle++;
			
			if (20==cycle){
				printf("hi\n");//Escrever em ficheiro				
				printf("ts1=%lld\ntm1=%lld\ndelay=%lld\n",info_rel[cycle].ts1,info_rel[cycle].tm1, info_rel[cycle].delay	);
				for (int i=0; i<cycle; i++){
					outFile<<info_rel[i].ts1 <<" " << info_rel[i].tm1 <<" " << info_rel[i].delay;
					outFile<<"\n";
				}
				outFile.close();
				cycle++;
			}
		}
		erro=offset+delay;
		//cout <<"\nFollow-up:"<< s <<"\nDelay:"<< delay <<"\n";
		//relogio.set_s((relogio.get_s())-delay);
		

		/*n = read(sockfd,&s,sizeof(s));
		if (n < 0) error("ERROR reading from socket");
		offset=(offset-s)/2;*/
		//cout <<"\nDelay Response:"<< s <<"\nOffset:"<< offset <<"\n";
		
		//usleep(5000000);
		n = read(sockfd,&s,sizeof(s));
		if (n < 0) error("ERROR reading from socket");	
		n = read(sockfd,&s,sizeof(s));
		if (n < 0) error("ERROR reading from socket");			
		//relogio.set_s(s-offset);	
		//cout <<"\nFollow-up:"<< s <<"\nTempo final:"<< relogio.get_s() <<"\n";
		//
	if (erro<0){
			//cout <<"\nDelay : "<< delay;
			//cout <<"\nFlag : "<< flag;
			if (flag>0){
				switch (flag) {
					case 1 : relogio.correct_drift(-0.1); flag=0; break;
					case 2 : relogio.correct_drift(-0.2); flag=0; break;
					case 3 : relogio.correct_drift(-0.3); flag=0; break;
					case 4 : relogio.correct_drift(-0.4); flag=0; break;
					case 5 : relogio.correct_drift(-0.5); flag=0; break;
					case 6 : relogio.correct_drift(-0.6); flag=0; break;
				}
			}
			if (flag==0){
				switch (erro)
				{
					case -1 : relogio.correct_drift(-0.1); flag=-1; break;
					case -2 : relogio.correct_drift(-0.2); flag=-2; break;
					case -3 : relogio.correct_drift(-0.3); flag=-3; break;
					case -4 : relogio.correct_drift(-0.4); flag=-4; break;
					case -5 : relogio.correct_drift(-0.5); flag=-5; break;
					default : relogio.correct_drift(-0.6); flag=-6; break;
				}
			}
			
		} else if ((erro>0)){
			//cout <<"\nDelay : "<< delay;
			//cout <<"\nFlag : "<< flag;
			if (flag<0){
				switch (flag) {
					case -1 : relogio.correct_drift(0.1); flag=0; break;
					case -2 : relogio.correct_drift(0.2); flag=0; break;
					case -3 : relogio.correct_drift(0.3); flag=0; break;
					case -4 : relogio.correct_drift(0.4); flag=0; break;
					case -5 : relogio.correct_drift(0.5); flag=0; break;
					case -6 : relogio.correct_drift(0.6); flag=0; break;
				}
			}
			if (flag==0){
				switch (erro)
				{
					case 1 : relogio.correct_drift(0.1); flag=1; break;
					case 2 : relogio.correct_drift(0.2); flag=2; break;
					case 3 : relogio.correct_drift(0.3); flag=3; break;
					case 4 : relogio.correct_drift(0.4); flag=4; break;
					case 5 : relogio.correct_drift(0.5); flag=5; break;
					default : relogio.correct_drift(0.6); flag=6; break;
				}
			}
		}
		else //erro =0 
		{
			//cout << "HELLO\n";
			switch (flag)
			{
				case -1 : relogio.correct_drift(0.1); flag=-1; break;
				case -2 : relogio.correct_drift(0.2); flag=-2; break;
				case -3 : relogio.correct_drift(0.3); flag=-3; break;
				case -4 : relogio.correct_drift(0.4); flag=-4; break;
				case -5 : relogio.correct_drift(0.5); flag=-5; break;
				case -6 : relogio.correct_drift(0.6); flag=-6; break;
				case 1 : relogio.correct_drift(-0.1); flag=-1; break;
				case 2 : relogio.correct_drift(-0.2); flag=-2; break;
				case 3 : relogio.correct_drift(-0.3); flag=-3; break;
				case 4 : relogio.correct_drift(-0.4); flag=-4; break;
				case 5 : relogio.correct_drift(-0.5); flag=-5; break;
				case 6 : relogio.correct_drift(-0.6); flag=-6; break;
				default : break;
			}
			flag=0;
		}
		
	}
	
	relogio.printTime();
    pthread_join(first,NULL);
    
    close(sockfd);
}
