#pragma once
#include <DirectXMath.h>

struct IoContextEx
{
	WSAOVERLAPPED over;
	WSABUF m_wsabuf;
	UCHAR m_szIoBuf[MAX_BUFSIZE];
	eClientType m_eType;
	int m_itarget;
};

struct Item 
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4X4 m_WorldPos;
	WORD m_PlayerState;
	int m_number;				// �̰� ���° ����������.
	bool is_cli_connect;		// Ŭ�� �ǵ�ȳ�?
	int connected_number;		// � Ŭ��� ����Ǿ�����.
};


struct Client
{
	DirectX::XMFLOAT3 pos;
	int anistate;
	DirectX::XMFLOAT4X4 m_WorldPos;
	WORD m_PlayerState;

	bool m_bConnect;
	eBulletDir m_eDir;

	UCHAR m_szPacket_buf[MAX_PACKET];
	
	SOCKET m_ClientSocket;
	IoContextEx m_IoEx;
	int m_iPredata;
	int m_iCursize;

	mutex m_lock;

	float m_Timer;
};