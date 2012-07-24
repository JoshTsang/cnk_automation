//
//  main.c
//  testing
//
//  Created by Josh Tsang on 12-7-12.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/timeb.h>

#include "consts.h"


#define BUFSIZE 1024

char* page[] = {
    "submitOrder.php",
    "updateTableStatus.php",
    "cleanTable.php"
};

int get(int sockfd, char *page, char *msg)
{
    int ret;
    char *resultHead;
    int i;
    char httpHeader[4096], requstPage[100], buf[BUFSIZE];
        
    memset(requstPage, 0, 100);
    sprintf(requstPage, "GET /orderPad/%s?%s HTTP/1.1\n", page, msg);
    memset(httpHeader, 0, 4096);
    strcat(httpHeader, requstPage);
    strcat(httpHeader, "Host: 192.168.1.1\n");
    strcat(httpHeader, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
    strcat(httpHeader, "Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(httpHeader, "Content-Length: 0");
    strcat(httpHeader, "\n\n");
    
    strcat(httpHeader, "\r\n\r\n");
    //printf("%s\n",httpHeader);
    
    ret = write(sockfd,httpHeader,strlen(httpHeader));
    if (ret < 0) {
        printf("Http failed：%s\n", strerror(errno));
        exit(0);
    }
    
    memset(buf, 0, BUFSIZE);
    i= read(sockfd, buf, BUFSIZE);
    if (i==0){
        close(sockfd);
        printf("读取数据报文时发现远端关闭，该线程终止！\n");
        return -1;
    }
    
    resultHead = strchr(buf, ' ');
    char result[4];
    //printf("%s\r\n", buf);
    strncpy(result, resultHead + 1, 3);
    
    return atoi(result);
}

int post(int sockfd, char *page, char *json)
{
    socklen_t len;
    int ret;
    char *resultHead;
    int i;
    char httpHeader[4096], postBuff[4096], requstPage[100], buf[BUFSIZE], *str;
    //发送数据
    memset(postBuff, 0, 4096);
    sprintf(postBuff, "json=%s", json);
    str=(char *)malloc(128);
    len = strlen(postBuff);
    sprintf(str, "%d", len);
    
    memset(requstPage, 0, 100);
    sprintf(requstPage, "POST /orderPad/%s HTTP/1.1\n", page);
    memset(httpHeader, 0, 4096);
    strcat(httpHeader, requstPage);
    strcat(httpHeader, "Host: 192.168.1.1\n");
    strcat(httpHeader, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
    strcat(httpHeader, "Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(httpHeader, "Content-Length: ");
    strcat(httpHeader, str);
    strcat(httpHeader, "\n\n");
    
    strcat(httpHeader, postBuff);
    strcat(httpHeader, "\r\n\r\n");
    //printf("%s\n",httpHeader);
    
    ret = write(sockfd,httpHeader,strlen(httpHeader));
    if (ret < 0) {
        printf("Http failed：%s\n", strerror(errno));
        exit(0);
    }
    
    memset(buf, 0, BUFSIZE);
    i= read(sockfd, buf, BUFSIZE);
    if (i==0){
        close(sockfd);
        printf("读取数据报文时发现远端关闭，该线程终止！\n");
        return -1;
    }
    
    resultHead = strchr(buf, ' ');
    char result[4];
    strncpy(result, resultHead + 1, 3);
    //printf("%s", buf);
    return atoi(result);
}

int constructOrderJson(const char *tableId, const char *dishes, char *json) 
{
    sqlite3 *db;
	char *zErrMsg = 0;
	int rc,i,k,m;
	int nrow = 0, ncolumn = 0;
	char **name;
	char sql[100][100];
	char sqlite[100];
	time_t now;
	struct tm *timenow;
	char strTemp[255]="";
	
	rc = sqlite3_open("menu.db",&db);
	if(rc){
		printf("Can't open database:%s",sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
    
	for( i = 0,k = 0,m=0;i  < strlen(dishes);i++){
		
		if(dishes[i] == ','){
			m = 0;
			k++;
		}else{
			sql[k][m] = dishes[i];
			m++;
		}
	}
	strcpy(json,"{\"order\":[");
	for(i = 0; i < k+1;i++){
		strcpy(strTemp,sql[i]);
		int b = atoi(strTemp);
		sprintf(sqlite,"select id,name,price from total_menu where id = %d",b);
		rc = sqlite3_get_table(db,sqlite,&name,&nrow,&ncolumn,&zErrMsg);
		if(rc != SQLITE_OK){
			printf("SQL error:%s",zErrMsg);
            return -1;
		}
		if(i > 0){
			sprintf(json,"%s,{\"quan\":1,\"id\":%s,\"price\":%s,\"name\":\"%s\"}",json,name[3],name[5],name[4]);
		}else{
			sprintf(json,"%s{\"quan\":1,\"id\":%s,\"price\":%s,\"name\":\"%s\"}",json,name[3],name[5],name[4]);
		}
	}
    
	time(&now);
	timenow = localtime(&now);
	sprintf(json,"%s],\"timestamp\":\"%4d-%02d-%02d %02d:%02d:%02d\",\"tableId\":%s}\n",
            json,timenow->tm_year+1900,
            timenow->tm_mon+1,timenow->tm_mday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec,tableId);
	//printf("%s\n",json);
	sqlite3_close(db);
	return 0;

}
int connectServer(struct sockaddr_in servaddr)
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("创建网络连接失败,本线程即将终止---socket error!\n");
        return -1;
    };
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVERIP, &servaddr.sin_addr) <= 0 ){
        printf("创建网络连接失败,本线程即将终止--inet_pton error!\n");
        return -1;
    };
    
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        printf("连接到服务器失败,connect error!%s\n", strerror(errno));
        return -1;
    }
    return sockfd;
}

double difftimeval(const struct timeval *tv1, const struct timeval *tv2)
{
    double d;
    time_t s;
    suseconds_t u;
    
    s = tv1->tv_sec - tv2->tv_sec;
    u = tv1->tv_usec - tv2->tv_usec;
    if (u < 0)
        --s;
    
    d = s;
    d *= 1000000.0;
    d += u;
    d = d/1000000;
    
    return d<0?-d:d;
}

int main(int argc, char **argv)
{
    int sockfd;
    int ret;
    struct sockaddr_in servaddr;
    double tCost;
    char buf[4096];
	char json[4096];
    struct timeval tStart;
    struct timeval tEnd;
    time_t now;
	struct tm *timestamp;
    
    if (argc < 3) {
        printf("test action args");
        return 0;
    }
    
    sockfd = connectServer(servaddr);
    if (sockfd < 0) {
        printf("failed to connetting server.\r\n");
        return 0;
    }
    
    gettimeofday(&tStart, NULL);
    if (strcmp(argv[2], "submit") == 0) {
        if (constructOrderJson(argv[3], argv[4], json) < 0) {
            printf("error occured when open database");
            return 0;
        }
        ret = post(sockfd, page[0], json);
    } else if (strcmp(argv[2], "open") == 0) {
        sprintf(buf, "TID=%s&TST=%s", argv[3], "1");
        ret = get(sockfd, page[1], buf);
    } else if (strcmp(argv[2], "clean") == 0) {
        time(&now);
        timestamp = localtime(&now);
        sprintf(buf, "{\"TID\":%s,\"timestamp\":\"%4d-%02d-%02d %02d:%02d:%02d\"}\r\n", argv[3],timestamp->tm_year+1900, timestamp->tm_mon+1,timestamp->tm_mday,timestamp->tm_hour,timestamp->tm_min,timestamp->tm_sec);
        ret = post(sockfd, page[2], buf);
        if (ret == 200) {
            sprintf(buf, "TID=%s&TST=%s", argv[3], "0");
            ret = get(sockfd, page[1], buf);
        }
    } else {
        printf("unsupported command\r\n");
        return 0;
    }
    gettimeofday(&tEnd, NULL);
    
    tCost = difftimeval(&tEnd, &tStart);
    //printf("%10s%15s%10s%10s%10s\r\n", "Case", "Action", "Return", "Result", "Duration");
    printf("%10s%15s%10d%10s%10f s\r\n", argv[1], argv[2], ret, ret==200?"Succ":"Failed", tCost);
    close(sockfd);
    
    return 0;
}



