#include <WinSock2.h>	// 윈도우에서 소켓을 사용할 수 있게 함
#include <stdlib.h>
#include <stdio.h>
#define BUFSIZE 512 // 클라이언트에서 받을 데이터 버퍼 사이즈

//소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;					// 메시지 버퍼
	FormatMessage(						 // 에러 메세지 형식 설정
		FORMAT_MESSAGE_ALLOCATE_BUFFER | // message의 메모리를 시스템에서 할당 해달라고 요청
		FORMAT_MESSAGE_FROM_SYSTEM,      // 운영체제로부터 오류메세지를 가져옴.   
		NULL, WSAGetLastError(),         // 가장 최근 발생한 소켓에러에 대응되는 에러코드를 반환
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // message의 언어를 지정해주는 부분
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR); // 에러 시 메세지 박스 출력
	LocalFree(lpMsgBuf); // FORMAT_MESSAGE_ALLOCATE_BUFFER를 플래그로 지정한 경우엔
	exit(-1);			 // 사용자가 직접 메모리를 해제해주어야 메모리 누수가 발생하지 않음.
}

// 소켓 함수 오류 출력
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

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags) // 소캣의 정보를 받는다.
{
	int received; // 소캣을 받음
	char *ptr = buf; // 버퍼
	int left = len; // 길이

	while (left > 0)
	{
		received = recv(s, ptr, left, flags); // 소캣의 정보를 받음
		if (received == SOCKET_ERROR) // 입력 받는데 오류
			return SOCKET_ERROR; 
		else if (received == 0) // 입력 받은게 없으면 종료
			break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 정보를 초기화 한다.
	WSADATA wsa;			// WSADATA 구조체 변수 wsa
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) // 버전과 정보를 입력 받는데 실패 한다면 -1리턴
		return -1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // IPv4프로토콜, 연결 지향형(TCP) 소켓 생성
	if (sock == INVALID_SOCKET) err_quit("socket()"); // 소캣 함수가 오류 날 때 메시지 출력

	// connect()
	SOCKADDR_IN serveraddr; // 소켓 주소 입력 변수
	ZeroMemory(&serveraddr, sizeof(serveraddr)); // serveraddr에 serveraddr만큼 메모리를 0으로 초기화
	serveraddr.sin_family = AF_INET; // IPv4
	serveraddr.sin_port = htons(9000); // 포트 넘버
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // 서버 소캣과 연결
	if (retval == SOCKET_ERROR) err_quit("connect()"); // 연결 실패

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZ + 1]; // 문자열에 있어서 널문자 한 칸을 위해 +1을 함.
	int len;

	// 서버와 데이터 통신
	while (1)
	{
		// 데이터 입력
		ZeroMemory(buf, sizeof(buf));
		printf("\n[보낼 데이터]");
		if (fgets(buf, BUFSIZ + 1, stdin) == NULL) // 엔터만 입력하면 종료
			break;

		// '\n' 문자 제거
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// 데이터 보내기
		retval = send(sock, buf, strlen(buf), 0); // 서버에게 데이터를 보냄
		if(retval == SOCKET_ERROR)		// send오류
		{
			err_display("send()");
			break;
		}
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n",retval);

		// 데이터 받기
		retval = recvn(sock, buf, retval, 0); // 서버로 부터 데이터를 받음
		if(retval == SOCKET_ERROR)			// recv오류 
		{
			err_display("recv()");
			break;
		}
		else if(retval == 0 )				// 바이트가 0이면 종료
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[TCP 클라이언트]%d바이트를 받았습니다.\n", retval);
		printf("[받은 데이터]%s\n",buf);
	}

	// closesocket()
	closesocket(sock);

	//윈속 종료
	WSACleanup();
	return 0;
}