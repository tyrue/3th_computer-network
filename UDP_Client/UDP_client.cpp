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
	LPVOID lpMsgBuf;					// �޽��� ����
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
	else					// �ɼ� �� ������ �޽����� ���
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
	serveraddr.sin_addr.s_addr = inet_addr("1.214.232.37"); // ���� ip�ּ�
														
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // ���ϰ� ���� �ּҸ� ���´�
	if (retval == SOCKET_ERROR) err_Msg("connect()", TRUE); // ���� ����

	// ������ ��ſ� ����� ����
	SOCKADDR_IN	peeraddr; // Ŭ���̾�Ʈ �ּ�
	int addrlen;			// �ּ� ����
	char buf[BUFSIZE + 1];	// ���ڿ��� �־ �ι��� �� ĭ�� ���� +1�� ��.
	int len;

	// ������ ������ ���
	while (1)	// ������ ���� �� ���� �ݺ�
	{
		// ������ �Է�
		printf("\n[���� ������]");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)	break; // ���͸� �Է��ϸ� ����

		// '\n' ���� ����
		len = strlen(buf);
		if (buf[len - 1] == '\n') 
			buf[len - 1] = '\0';
		if (strlen(buf) == 0) break;

		// ������ ������
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR *)&serveraddr, sizeof(serveraddr)); // �������� �����͸� ����
		if (retval == SOCKET_ERROR) { // ���� ������ ���� ���� �޼��� ���
			err_Msg("sendto()", FALSE);	continue;
		}

		// ������ �ޱ�
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, retval, 0, (SOCKADDR*)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) { // ���� ������ ���� ���� �޼��� ���
			err_Msg("recvfrom()", FALSE);
			continue;
		}

		// �۽����� IP�ּ� üũ
		if (memcmp(&peeraddr, &serveraddr, sizeof(peeraddr))) { // peeraddr�� serveraddr�� ���̰� �ٸ���
			printf("[����] �߸��� ������ �Դϴ�.\n");          // ���� ���
			continue;
		}

		// ���� ������ ���
		buf[retval] = '\0';
		printf("[UDP Ŭ���̾�Ʈ]%d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		printf("[���� ������]%s\n", buf);
	}

	// closesocket()
	closesocket(sock); // ��������

	//���� ����
	WSACleanup();
	return 0;
}