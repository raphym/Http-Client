#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */

//Define
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define sys_cal 1
#define usage	2
#define wrong_input	3
#define closesocket(s) close(s)

//typedef
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

//Global
char timebuf[128]={0};//Time

char *host=NULL;//HOST
char *fullPath=NULL;
int port=80;//PORT
int url_size=-1;

char *request=NULL;//The request
char *response=NULL;//The response of the request

int flagHead=0;//flag if only head
int flagDate=0;//flag if there is a date
int flagPort=0;	//flag if there is a port

//Declaration of the functions
void ERROR(int flag);
void checkUrl(char *url);
int checkTime(char *str);
int countWord(char *str,char *word);
int onlyNumber(char *str);
int initConnection();
void endConnection(int sock);
void requestConstractor();
int exeRequest(int sock);

//////////////////////////////////////////
		//MAIN//
/////////////////////////////////////////
int main(int argc, char *argv[])
{
	int verify=0,iterator=0,size=-1;

	//Check the input
	if(argc > 5 || argc < 2 )//size doesn't compatible
		ERROR(usage);

	if(argc == 2 )//size=2
		//check the Url
		checkUrl(argv[1]);//there is only the url

	if(argc == 3)//size=3
	{
		//flag Head
		flagHead=1;

		if(strcmp(argv[1],"-h")==0) //if -h is in argv[1]
			checkUrl(argv[2]);

		else if(strcmp(argv[2],"-h")==0)//if -h is in argv[2]
			checkUrl(argv[1]);

		else
			ERROR(usage);
	}

	if(argc == 4)//size=4
	{
		//flag
		flagDate=1;

		if(strcmp(argv[1],"-d")==0) //if -d is in argv[1]
		{
			//check time
			verify = checkTime(argv[2]);
			if(verify==-1)
				ERROR(wrong_input);

			//check the Url
			checkUrl(argv[3]);
		}

		else if(strcmp(argv[2],"-d")==0) //if -d is in argv[2]
		{
			//check time
			verify = checkTime(argv[3]);
			if(verify==-1)
				ERROR(wrong_input);

			//check the Url
			checkUrl(argv[1]);
		}

		else
			ERROR(usage);
	}

	if(argc == 5)//size=5
	{
		int posH=-1, posD=-1;
		//find the flags h and d and their positions
		iterator=0;
		for( ; iterator<argc ; iterator++)
		{
			if(strcmp(argv[iterator],"-h")==0)
			{
				posH=iterator;
				flagHead=1;
			}
			if(strcmp(argv[iterator],"-d")==0)
			{
				posD=iterator;
				flagDate=1;
			}
		}

		if(posH==-1 || posD==-1)
			ERROR(usage);

		//check time
		verify = checkTime(argv[posD+1]);
		if(verify==-1)
			ERROR(wrong_input);

		//check the Url
		if( (posH==1 && posD==2) || (posH==3 && posD==1) )
			checkUrl(argv[4]);


		else if( (posH==2 && posD==3) || (posH==4 && posD==2)  )
			checkUrl(argv[1]);

		else if(posH==1 && posD!=2)
			checkUrl(argv[2]);

		else if(posH==4 && posD!=3)
			checkUrl(argv[3]);

		else
			ERROR(usage);
	}

	//Connect to the serveur
	int sock = initConnection();

	//construct the request
	requestConstractor();//construct the request
	if(request==NULL)
	{
		perror("Error to construct the request\n");
		exit(-1);
	}

	//Execute the request
	size=exeRequest(sock);

	//print to the screen
	printf("HTTP request =\n%s\nLEN = %d\n", request, (int)strlen(request));
	printf("\n%s",response);
	printf("\n Total received response bytes: %d\n",size);
//Finish the entire program
endConnection(sock);
free(fullPath);
free(host);
free(request);
free(response);
//END OF THE MAIN
}

//////////////////////////////////////////
		//ERROR//
/////////////////////////////////////////

void ERROR(int flag)
{
	if(flag==1)
		perror("<sys_call>\n");

	if(flag==2)
		printf("Usage: client [-h] [-d <timeinterval>]<URL>\n");


	if(flag==3)
		printf("wrong input \n");

	exit(-1);
}

//////////////////////////////////////////
		//check time//
/////////////////////////////////////////

//this function return 0 if the time is legale
int checkTime(char *t)
{
	int day , hour , min , i=1 , verify;

	char *a[3];//array char** of "char*"
	time_t now;
	char *b;

	verify = countWord(t,":");//verify if there is twice ":"
	if(verify!=2 )
		return -1;


	if(t[0]==':')
		return -1;


	//strTok on the input
	b =strtok(t,":");
	a[0]=b;

	while(b!=NULL)
	{
		b = strtok(NULL,":");
		if(i<3 && b==NULL)
			return -1;
		if(i==3)
			break;
		a[i]=b;
		i++;
	}


	//verify if there is only numbers
	verify=0;
	for(i=0 ; i<3 ; i++)
	{
		verify= onlyNumber(a[i]);
		if(verify==-1)
			return -1;
	}

	//All is ok !!!! lets initiate the time
	day=atoi(a[0]);
	hour=atoi(a[1]);
	min=atoi(a[2]);

	now = time(NULL);
	now=now-(day*24*3600+hour*3600+min*60);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	//timebuf holds the correct format of the time.

	return 0;
}
//////////////////////////////////////////
		//countWord//
/////////////////////////////////////////

//this function return the numbers of times
//that a word appear in the string

int countWord(char *str,char *word)
{
	int count = 0;
	char *str2=strstr(str,word);
	while (str2!=NULL)
	{
		count++;
		str2=strstr(str2+1,word);
	}
	return count;
}

//////////////////////////////////////////
		//OnlyNumber//
/////////////////////////////////////////

//this function return 0 if there is only numbers

int onlyNumber(char *str)
{
	if(strlen(str)==0)
		return -1;
	int i=0;
	while(str[i] != '\0')
	{
		if(str[i]<48 || str[i]>57)
			return -1;
		i++;
	}
	return 0;
}

//////////////////////////////////////////
		//check URL//
/////////////////////////////////////////
void checkUrl(char *url)
{
	url_size = (int)strlen(url) ;

						/*Variable of this function*/
	int verify=0, i,p ,q=0;
	char *http = "http://";

	//hostAdress for this function
	char *hostAddr=(char*)malloc( strlen(url) * sizeof(char) );
	if(hostAddr==NULL)
	{
		perror("Error of malloc in checkUrl()\n");
		exit(-1);
	}
	bzero (hostAddr, url_size);

	//port char *
	char *thePort=(char*)malloc( strlen(url) * sizeof(char) );
	if(thePort==NULL)
	{
		perror("Error of malloc in checkUrl()\n");
		free(hostAddr);
		exit(-1);
	}
	bzero (thePort, url_size);

						/*END Variable of this function*/

	//Host Global
	host=(char*)malloc( strlen(url) * sizeof(char) );
	if(host==NULL)
	{
		perror("Error of malloc in checkUrl()\n");
		free(hostAddr);
		free(thePort);
		exit(-1);
	}
	bzero (host, url_size);

	//ALl the path ===> host + path
	fullPath=(char*)malloc( strlen(url) * sizeof(char) );
	if(fullPath==NULL)
	{
		perror("Error of malloc in checkUrl()\n");
		free(host);
		free(hostAddr);
		free(thePort);
		exit(-1);
	}
	bzero (fullPath, url_size);



//start of the algorithm

	verify=countWord(url,http);
	if(verify<1)
	{
		free(fullPath);
		free(host);
		free(hostAddr);
		free(thePort);
		ERROR(wrong_input);
	}

	//we check if the http:// is in the start
	for(i=0 ; i<(int)strlen(http) ; i++)
	{
		if(url[i]!=http[i])
		{
			free(fullPath);
			free(host);
			free(hostAddr);
			free(thePort);
			ERROR(wrong_input);
		}
	}


	//continue to scan the string URL

	//Scan for the host
	p=0;
	for( i = 7 ; i <url_size ; i++)
	{
		//If there is a path or port so Stop
		if(url[i] == '/' || url[i] == ':'  )
			break;
		hostAddr[p]=url[i];
		p++;
	}
	strcat(hostAddr,"\0");
	strcpy(host,hostAddr);//Global host

	//if there is only the host without '/' so error (1)
	if(i>=url_size)
		{
			free(hostAddr);
			free(thePort);
			free(host);
			free(fullPath);
			ERROR(wrong_input);
		}

	if(url[i]=='/')//If there is the host and the path (2)
	{
		p=7;
		q=0;
		for( ; p<url_size; p++)
		{
			fullPath[q]=url[p];
			q++;
		}
		strcat(fullPath,"\0");

		free(hostAddr);
		free(thePort);

		return;
	}


	//If there is "the host and the port" or "the host,the port and the path"
	p=0;
	if( i < url_size && url[i]== ':')
	{
		i++;
		for( ; i<url_size;i++)
		{
			//if there is a path so Stop
			if(url[i]== '/')
				break;
			thePort[p]=url[i];
			p++;
		}
		strcat(thePort,"\0");

		verify=onlyNumber(thePort);//valid the port
		if(verify==-1)
		{
			free(fullPath);
			free(host);
			free(hostAddr);
			free(thePort);
			ERROR(wrong_input);
		}
		flagPort=1; //flag there is a port !
		port=(int)atoi(thePort);  //Global port

		if(i>=url_size)//if there is the host and the port without '/' so error (3)
			{
				free(fullPath);
				free(host);
				free(hostAddr);
				free(thePort);
				ERROR(wrong_input);
			}


		//If there is the host , the port and also a path (4)
		//so i have to complete the fullPath
		i++;
		q=strlen(host)+1;
		strcat(fullPath,host);
		strcat(fullPath,"/");
		while(i<url_size)
		{
			fullPath[q]=url[i];
			q++;
			i++;
		}

		strcat(fullPath,"\0");
	}

	free(hostAddr);
	free(thePort);
}

//////////////////////////////////////////
		//Connect//
/////////////////////////////////////////

//connect the client to the server

int initConnection()
{
	//creation of the socket
	SOCKET sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
		perror("socket()");
		exit(-1);
	}


	struct hostent *hostinfo = NULL;
	SOCKADDR_IN sin = { 0 }; /* initialise la structure avec des 0 */
	const char *hostname = host;

	hostinfo = gethostbyname(hostname); /* on récupère les informations de l'hôte auquel on veut se connecter */
	if (hostinfo == NULL) /* l'hôte n'existe pas */
	{
		herror("");//Unknown host
		free(fullPath);
		free(host);
		exit(-1);
	}

	sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr; /* l'adresse se trouve dans le champ h_addr de la structure hostinfo */
	sin.sin_port = htons(port); /* on utilise htons pour le port */
	sin.sin_family = PF_INET;

	if(connect(sock,(SOCKADDR *) &sin, sizeof(SOCKADDR)) == -1)
	{
		perror("connect()");
		free(fullPath);
		free(host);
		exit(-1);
	}

	return sock;
}


//////////////////////////////////////////
		//EndConnection//
/////////////////////////////////////////

//to end the connection
void endConnection(int sock)
{
	closesocket(sock);
}

//////////////////////////////////////////
		//Request constractor//
/////////////////////////////////////////

void requestConstractor()
{
	int request_size= 5 + 7 + strlen(fullPath) + 29 + strlen(timebuf) + 8 ;

	request=(char*)malloc( request_size * sizeof(char) );
	if(request==NULL)
	{
		perror("Error of malloc in requestConstractor()\n");
		free(fullPath);
		free(host);
		exit(-1);
	}

	bzero(request,request_size);

	//if there is no date
	if(flagDate==0)
	{
		//if the user want all the page (head and body)
		if(flagHead==0)
			strcat(request,"GET http://");//Get


		//if the user want only the head
		else
			strcat(request,"HEAD http://");//Head

		//End of the request (full Path + symbols)
		strcat(request,fullPath);//Add the host to the request
		strcat(request," HTTP/1.0\r\n\r\n");
	}

	//if there is a date
	if(flagDate==1)
	{
		//if the user want all the page (head and body)
		if(flagHead==0)
			strcat(request,"GET http://");

		//if the user want only the head
		else
			strcat(request,"HEAD http://");

		//End of the request (full Path + if modified + time + endSymbols)
		strcat(request,fullPath);
		strcat(request," HTTP/1.0\n");
		strcat(request,"If-Modified-Since: ");
		strcat(request,timebuf);
		strcat(request,"\r\n\r\n");
	}
}

//////////////////////////////////////////
		//executeRequest//
/////////////////////////////////////////

int exeRequest(int sock)
{
	//For the send of the request
	if( send(sock, request, strlen(request), 0) < 0)
	{
		perror("send()");
		free(fullPath);
		free(host);
		free(request);
		exit(-1);
	}

	//For the receive of data
	int i = 0;
	int nbytes = 0;
	int currentSize = 4096;
	int oldSize = currentSize;
	response = (char*) malloc(currentSize);
	if(response==NULL)
	{
		perror("Error of malloc in the exeRequest function\n");
		free(fullPath);
		free(host);
		free(request);
		exit(-1);
	}

	while ( (nbytes = recv(sock, response + i, 4096, 0)) > 0 )
	{
  		i += nbytes;
    	oldSize = currentSize;
    	currentSize += 4096;
    	char *newBuffer = (char*) malloc(currentSize);
    	if(newBuffer==NULL)
			{
				perror("Error of malloc in the exeRequest function\n");
				free(fullPath);
				free(host);
				free(request);
				free(response);
				exit(-1);
			}

    	memcpy(newBuffer,response,oldSize);
    	free(response);
    	response = newBuffer;
	}

	return (int)strlen(response);
}
//////////////////////////////////////////
	    //*******END**********//
/////////////////////////////////////////
