#ifndef  TRACKER_H
#define  TRACKER_H
#include <netinet/in.h>
#include "parse_metafile.h"

typedef struct _Peer_addr {
	char              ip[16];
	unsigned short    port;
	struct _Peer_addr *next;
} Peer_addr;

//  你好
//  httpЭ��涨,���������еķ����ֺͷ���ĸ��Ҫ���б���ת��
int http_encode(unsigned char *in,int len1,char *out,int len2);
//  �������ļ��д洢��tracker��URL��ȡtracker������
int get_tracker_name(Announce_list *node,char *name,int len);
//  �������ļ��д洢��tracker��URL��ȡtracker�˿ں�
int get_tracker_port(Announce_list *node,unsigned short *port);

//  �������͵�tracker��������������Ϣ
int create_request(char *request, int len,Announce_list *node,
				   unsigned short port,long long down,long long up,
				   long long left,int numwant);

//  ׼������tracker
int prepare_connect_tracker(int *max_sockfd);
//  ׼������peer
int prepare_connect_peer(int *max_sockfd);

//  ��ȡtracker���ص���Ϣ������
//  һ������Ϊ"5:peers"�ؼ���֮����һ���ַ���,��һ����һ���б�
int get_response_type(char *buffer,int len,int *total_length);
//  ������һ��tracker���ص���Ϣ
int parse_tracker_response1(char *buffer,int ret,char *redirection,int len);
//  �����ڶ���tracker���ص���Ϣ
int parse_tracker_response2(char *buffer,int ret);
//  ����֮�������ӵ�peer���뵽peer�б���
int add_peer_node_to_peerlist(int *sock,struct sockaddr_in saptr);

void free_peer_addr_head();
//  �ͷű��ļ������ж�̬������ڴ��Է�ֹ�ڴ�й©
void release_memory_in_tracker();

#endif
