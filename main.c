/*
 * main.c
 *
 *  Created on: 14 февр. 2018 г.
 *      Author: jake
 */


#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <pthread.h>

#define TCPPORT 1234
#define UDPPORT 1235


void* thread_waitPid(void*ppid);

int main(void)
{
	int tcpSock,ws, udpSock;
	char buf[BUFSIZ];
	int count_client=0;
	pid_t pid,*ppid;
	ppid = &pid;
	pthread_t tid;
	struct sockaddr_in tcp_serv;
	struct sockaddr_in udp_serv, from;
	int fromlen = sizeof(from);
	// обнуление длины и структур
	memset(&from,0,fromlen);
	bzero(&tcp_serv,sizeof(tcp_serv));
	bzero(&udp_serv,sizeof(udp_serv));

/* Инициализация tcp сокета */
	tcpSock = socket(AF_INET,SOCK_STREAM,0);
	//inet_aton("127.0.0.1",&local.sin_addr);
	tcp_serv.sin_port = htons(TCPPORT);
	tcp_serv.sin_family = AF_INET;
	tcp_serv.sin_addr.s_addr = htonl(INADDR_ANY);//0.0.0.0
	if (bind(tcpSock,(struct sockaddr*) &tcp_serv, sizeof(tcp_serv)) < 0)
	{
		perror("tcp binding error");
		exit(2);
	}
	listen(tcpSock,SOMAXCONN);
/*-------------------------*/
/* Инициализация udp сокета*/
	udpSock = socket(AF_INET,SOCK_DGRAM,0);
	if (udpSock < 0)
	{
		perror("socket error");
		exit(3);
	}
	//inet_aton("127.0.0.1",&(udp_serv.sin_addr));
	udp_serv.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // или так
	udp_serv.sin_port = htons(UDPPORT);
	udp_serv.sin_family = AF_INET;
	if (bind(udpSock, (struct sockaddr*) &udp_serv, sizeof(udp_serv)) < 0)
	{
		perror("udp binding error");
		exit(4);
	}
/*-------------------------*/
/* Определение набора дескрипторов и максимального количества*/
	fd_set rfds, afds;
	int nfds=getdtablesize();
	FD_ZERO(&afds);
	FD_SET(tcpSock,&afds);
	FD_SET(udpSock,&afds);

/* Цикличная обработка подключений*/
	while(1)
	{
		memcpy(&rfds, &afds, sizeof(rfds));
		select(nfds, &rfds, NULL,NULL,NULL);

/* Появление tcp клиента*/
		if ( FD_ISSET(tcpSock, &rfds) ) {
			ws=accept(tcpSock,NULL,NULL);
			//FD_SET(ws,&afds); // если без fork()
			if (0==(pid=fork())) { // процесс для tcp-клиента
				close(tcpSock); // закрыть дескр. слушающего сервера
				if(recv(ws,buf,BUFSIZ,0)<0)
				{
					perror("recv error");
					exit(5);
				}
				strcat(buf,"->");
				send(ws,buf,strlen(buf)+1,0);
				sleep(1);
				shutdown(ws,SHUT_RDWR);
				close(ws);
				exit(0);
			} else {// основной процесс ждет завершения tcp-процессов отдельными потоками
				printf("incoming tcp-client - %d\n",++count_client);
				pthread_create(&tid,NULL,thread_waitPid,ppid);
				pthread_detach(tid);
				close(ws);
			}
			continue;
		}

/* Появление udp клиента*/
		if ( FD_ISSET(udpSock, &rfds) ) {
			printf("incoming udp-client - %d\n",++count_client);
			if (recvfrom(udpSock,buf,sizeof(buf),0,(struct sockaddr*)&from,&fromlen) <0)
			{
				perror("recvfrom error");
				exit(6);
			}
			//printf("%s\n",buf);
			strcat(buf,"->");
			sendto(udpSock,buf,strlen(buf)+1,0,(struct sockaddr*)&from, fromlen);
			memset(buf,0,BUFSIZ);
		}
	}

/* Закрытие сокетов*/
	shutdown(tcpSock,SHUT_RDWR);
	shutdown(udpSock,SHUT_RDWR);
	close(tcpSock);
	close(udpSock);
	return 0;
}
