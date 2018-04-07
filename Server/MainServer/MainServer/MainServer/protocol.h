#pragma once

#define WM_SOCKET	WM_USER+1
#define MAX_USER	1000
#define MAIN_PORT	9000
#define SERVER_IP	127.0.0.1
#define MAX_BUF		4000
#define MAX_PACKET	255

#define SC_POS	1
#define SC_MOVE 2



#define CS_UP		1
#define CS_DOWN		2
#define CS_LEFT		3
#define CS_RIGHT	4

#define SC_POS		1
#define SC_MOVE		2
#define SC_REMOVE	3

enum clients_state {
	e_Recv,
	e_Send,
};


#pragma pack (push,1)

typedef struct IoEx {
	WSAOVERLAPPED	over;
	WSABUF			m_wsabuf;
	UCHAR			m_Iobuf[MAX_BUF];
	clients_state	m_eState;
};


typedef struct USER {
	bool			connect;
	int				id;
	int				x;
	int				y;
	clients_state	e_Type;
	IoEx			m_IoEx;

	int				cur_packet;
	int				pre_packet;
	unsigned char	packet_buffer[MAX_PACKET];
	SOCKET			client_socket;
};

struct sc_position_packet {
	BYTE size;
	BYTE type;
	WORD id;
	int x;
	int y;
};


#pragma pack (pop)