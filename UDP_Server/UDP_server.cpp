#include <WinSock2.h> // �����쿡�� ������ ����� �� �ְ� ��
#include <stdlib.h>
#include <stdio.h>
#undef UNICODE
#undef _UNICODE
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512 // Ŭ���̾�Ʈ���� ���� ������ ���� ������

// ���� �Լ� ���� ���
void err_Msg(const char *msg, bool option)
{
	LPVOID lpMsgBuf;					 // �޽��� ����
	FormatMessage(						 // ���� �޼��� ���� ����
		FORMAT_MESSAGE_ALLOCATE_BUFFER | // message�� �޸𸮸� �ý��ۿ��� �Ҵ� �ش޶�� ��û
		FORMAT_MESSAGE_FROM_SYSTEM,      // �ü���κ��� �����޼����� ������.   
		NULL, WSAGetLastError(),         // ���� �ֱ� �߻��� ���Ͽ����� �����Ǵ� �����ڵ带 ��ȯ
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // message�� �� �������ִ� �κ�
		(LPTSTR)&lpMsgBuf, 0, NULL);
	if (option)				// �ɼ� ���� �� ������ �޽��� �ڽ� ���
	{
		MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR); // ���� �� �޼��� �ڽ� ���
		LocalFree(lpMsgBuf); // FORMAT_MESSAGE_ALLOCATE_BUFFER�� �÷��׷� ������ ��쿣
		exit(-1);			 // ����ڰ� ���� �޸𸮸� �������־�� �޸� ������ �߻����� ����.
	}
	else					 // �ɼ� �� ������ �޽����� ���
	{
		printf("[%s]%s", msg, lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
}

int main(int argc, char *argv[])
{
	int retval;				// ������ ��

	// ���� ������ �ʱ�ȭ �Ѵ�.
	WSADATA wsa;			// WSADATA ����ü ���� wsa
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) // ������ ������ �Է� �޴µ� ���� �Ѵٸ� -1����
		return -1;

	// socket(UDP���� ����)
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);	// IPv4��������, �񿬰� ������(UDP) ���� ����
	if (sock == INVALID_SOCKET) err_Msg("socket()", TRUE); // ��Ĺ �Լ��� ���� �� �� �޽��� ���

	// Bind(), ���� �ּҿ� ���� ���ڸ� ������
	SOCKADDR_IN serveraddr; // ���� �ּ� �Է� ����
	ZeroMemory(&serveraddr, sizeof(serveraddr)); // serveraddr�� serveraddr��ŭ �޸𸮸� 0���� �ʱ�ȭ
	serveraddr.sin_family = AF_INET;  // IPv4
	serveraddr.sin_port = htons(9501);// ��Ʈ �ѹ�
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // ���� ip�ּ�
												
	retval = bind(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // ���ϰ� ���� �ּҸ� ���´�
	if (retval == SOCKET_ERROR) err_Msg("connect()", TRUE); // ���� ����

	// ������ ��ſ� ����� ����
	SOCKADDR_IN	clientaddr; // Ŭ���̾�Ʈ �ּ�
	int addrlen;			// �ּ� ����
	char buf[BUFSIZE + 1];	// ���ڿ��� �־ �ι��� �� ĭ�� ���� +1�� ��.

	// Ŭ���̾�Ʈ�� ������ ���
	while (1) // ������ ���� �� ���� �ݺ�
	{
		// ������ �ޱ�
		addrlen = sizeof(clientaddr); // ���� �Ҵ�
		retval = recvfrom(sock, buf, BUFSIZE, 0,	// �������κ��� �����͸� ����
			(SOCKADDR*)&clientaddr, &addrlen);	

		if (retval == SOCKET_ERROR)		// ���� ���� �� �����޼��� ���
		{
			err_Msg("recvfrom()", FALSE);
			continue;
		}

		// ���� ������ ���
		buf[retval] = '\0'; // ������ ���ڿ��� �ι��ڸ� �����ؼ� ������
		printf("[UDP\%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port), buf);

		// ������ ������
		retval = sendto(sock, buf, retval, 0,
			(SOCKADDR *)&clientaddr, sizeof(clientaddr)); // Ŭ���̾�Ʈ���� �����͸� ����
		if (retval == SOCKET_ERROR) // ���� ���� �� �����޼��� ���
		{
			err_Msg("sendto()", FALSE);
			continue;
		}
	}

	// closesocket()
	closesocket(sock);

	//���� ����
	WSACleanup();
	return 0;
}