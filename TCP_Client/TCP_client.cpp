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

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags) // ��Ĺ�� ������ �޴´�.
{
	int received; // ��Ĺ�� ����
	char *ptr = buf; // ����
	int left = len; // ����

	while (left > 0)
	{
		received = recv(s, ptr, left, flags); // ��Ĺ�� ������ ����
		if (received == SOCKET_ERROR) // �Է� �޴µ� ����
			return SOCKET_ERROR; 
		else if (received == 0) // �Է� ������ ������ ����
			break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}

int main(int argc, char *argv[])
{
	int retval;

	// ���� ������ �ʱ�ȭ �Ѵ�.
	WSADATA wsa;			// WSADATA ����ü ���� wsa
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) // ������ ������ �Է� �޴µ� ���� �Ѵٸ� -1����
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // IPv4��������, ���� ������(TCP) ���� ����
	if (sock == INVALID_SOCKET) err_quit("socket()"); // ��Ĺ �Լ��� ���� �� �� �޽��� ���

	// connect()
	SOCKADDR_IN serveraddr; // ���� �ּ� �Է� ����
	ZeroMemory(&serveraddr, sizeof(serveraddr)); // serveraddr�� serveraddr��ŭ �޸𸮸� 0���� �ʱ�ȭ
	serveraddr.sin_family = AF_INET; // IPv4
	serveraddr.sin_port = htons(9000); // ��Ʈ �ѹ�
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // ���� ��Ĺ�� ����
	if (retval == SOCKET_ERROR) err_quit("connect()"); // ���� ����

	// ������ ��ſ� ����� ����
	char buf[BUFSIZ + 1]; // ���ڿ��� �־ �ι��� �� ĭ�� ���� +1�� ��.
	int len;

	// ������ ������ ���
	while (1)
	{
		// ������ �Է�
		ZeroMemory(buf, sizeof(buf));
		printf("\n[���� ������]");
		if (fgets(buf, BUFSIZ + 1, stdin) == NULL) // ���͸� �Է��ϸ� ����
			break;

		// '\n' ���� ����
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// ������ ������
		retval = send(sock, buf, strlen(buf), 0); // �������� �����͸� ����
		if(retval == SOCKET_ERROR)		// send����
		{
			err_display("send()");
			break;
		}
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n",retval);

		// ������ �ޱ�
		retval = recvn(sock, buf, retval, 0); // ������ ���� �����͸� ����
		if(retval == SOCKET_ERROR)			// recv���� 
		{
			err_display("recv()");
			break;
		}
		else if(retval == 0 )				// ����Ʈ�� 0�̸� ����
			break;

		// ���� ������ ���
		buf[retval] = '\0';
		printf("[TCP Ŭ���̾�Ʈ]%d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		printf("[���� ������]%s\n",buf);
	}

	// closesocket()
	closesocket(sock);

	//���� ����
	WSACleanup();
	return 0;
}