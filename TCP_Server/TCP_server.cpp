#include <WinSock2.h>	// �����쿡�� ������ ����� �� �ְ� ��
#include <stdlib.h>
#include <stdio.h>
#define BUFSIZE 512 // Ŭ���̾�Ʈ���� ���� ������ ���� ������

//���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;					// �޽��� ����
	FormatMessage(						 // ���� �޼��� ���� ����
		FORMAT_MESSAGE_ALLOCATE_BUFFER | // message�� �޸𸮸� �ý��ۿ��� �Ҵ� �ش޶�� ��û
		FORMAT_MESSAGE_FROM_SYSTEM,      // �ü���κ��� �����޼����� ������.   
		NULL, WSAGetLastError(),         // ���� �ֱ� �߻��� ���Ͽ����� �����Ǵ� �����ڵ带 ��ȯ
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // message�� �� �������ִ� �κ�
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR); // ���� �� �޼��� �ڽ� ���
	LocalFree(lpMsgBuf); // FORMAT_MESSAGE_ALLOCATE_BUFFER�� �÷��׷� ������ ��쿣
	exit(-1);			 // ����ڰ� ���� �޸𸮸� �������־�� �޸� ������ �߻����� ����.
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main(int argc, char *argv[])
{
	int retval;				// ������ ��

	// ���� ������ �ʱ�ȭ �Ѵ�.
	WSADATA wsa;			// WSADATA ����ü ���� wsa
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)		// ������ ������ �Է� �޴µ� ���� �Ѵٸ� -1����
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);	 // IPv4��������, ���� ������(TCP) ���� ����
	if (listen_sock == INVALID_SOCKET) err_quit("socket()"); // ��Ĺ �Լ��� ���� �� �� �޽��� ���

	// Bind(), ���� �ּҿ� ���� ���ڸ� ������
	SOCKADDR_IN serveraddr;						 // ���� �ּ� �Է� ����
	ZeroMemory(&serveraddr, sizeof(serveraddr)); // serveraddr�� serveraddr��ŭ �޸𸮸� 0���� �ʱ�ȭ
	serveraddr.sin_family = AF_INET;			 // IPv4
	serveraddr.sin_port = htons(9000);			 // ��Ʈ �ѹ�
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);// ���� ip�ּ�

	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // ���ϰ� ���� �ּҸ� ���´�
	if (retval == SOCKET_ERROR) err_quit("bind()"); // bind ����

	//listen()
	retval = listen(listen_sock, SOMAXCONN);// ������ ���� ��û�� ���� �غ��Ѵ�, ���� ���� ����, ��� ������ ��⿭
	if(retval == SOCKET_ERROR) err_quit("listen()"); // listen ����

	// ������ ��ſ� ����� ����
	SOCKET client_sock;		// Ŭ���̾�Ʈ ��Ĺ
	SOCKADDR_IN clientaddr; // Ŭ���̾�Ʈ �ּ�
	int addrlen;			// �ּ� ����
	char buf[BUFSIZE + 1];	// ���ڿ��� �־ �ι��� �� ĭ�� ���� +1�� ��.

	while (1)
	{
		// accept()
		addrlen = sizeof(clientaddr);		// Ŭ���̾�Ʈ �ּ� ����
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen); // Ŭ���̾�Ʈ�� ����
		if(client_sock == INVALID_SOCKET)	// accept ����
		{
			err_display("accept()");
			continue;
		}
		printf("\n[TCP ����] Ŭ���̾�Ʈ ���� : IP�ּ� = %s, ��Ʈ ��ȣ = %d\n", // ���� ������ �޽��� ���
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ���
		while(1)
		{
			// ������ �ޱ�
			retval = recv(client_sock, buf, BUFSIZE, 0); // ��Ĺ�� ������ ����
			if(retval == SOCKET_ERROR) // recv����
			{
				err_display("recv()");
				break;
			}
			else if(retval == 0 ) // ����Ʈ���� 0�̸� ����
				break;

			// ���� ������ ���
			buf[retval] = '\0'; // ������ ���ڿ� ���� �����Ͽ� ���� ��
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
					ntohs(clientaddr.sin_port),buf);
			
			// ������ ������
			retval = send(client_sock, buf, retval, 0); // Ŭ���̾�Ʈ���� �����͸� ����
			if(retval == SOCKET_ERROR) // send����
			{
				err_display("send()");
				break;
			}
		}
	

		// closesocket()
		closesocket(client_sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP�ּ� = %s, ��Ʈ ��ȣ = %d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);

	//���� ����
	WSACleanup();
	return 0;
}