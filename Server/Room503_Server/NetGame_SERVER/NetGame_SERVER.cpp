// NetGame_SERVER.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"


int main()
{
	vector<thread*> vWork_thread;
	CMainServer* MS = new CMainServer();

	MS->Initi();

	for (int i = 0; i < 6; ++i)
		vWork_thread.push_back(new thread{ [&]() {MS->Work_Process(); } });

	thread accept_thread{ [&]() {MS->Accept_Process(); } };
	thread update_thread{ [&] {MS->Update_Thread(); } };

	accept_thread.join();
	update_thread.join();

	for (auto pthread : vWork_thread) {
		pthread->join();
		delete pthread;
	}

	MS->~CMainServer();

	return 0;
}

