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

int main(int argc, char *argv[])
{
	int retval;				// 돌려줄 값

	// 윈속 정보를 초기화 한다.
	WSADATA wsa;			// WSADATA 구조체 변수 wsa
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)		// 버전과 정보를 입력 받는데 실패 한다면 -1리턴
		return -1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);	 // IPv4프로토콜, 연결 지향형(TCP) 소켓 생성
	if (listen_sock == INVALID_SOCKET) err_quit("socket()"); // 소캣 함수가 오류 날 때 메시지 출력

	// Bind(), 소켓 주소와 고유 숫자를 묶어줌
	SOCKADDR_IN serveraddr;						 // 소켓 주소 입력 변수
	ZeroMemory(&serveraddr, sizeof(serveraddr)); // serveraddr에 serveraddr만큼 메모리를 0으로 초기화
	serveraddr.sin_family = AF_INET;			 // IPv4
	serveraddr.sin_port = htons(9000);			 // 포트 넘버
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);// 보낼 ip주소

	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // 소켓과 소켓 주소를 묶는다
	if (retval == SOCKET_ERROR) err_quit("bind()"); // bind 실패

	//listen()
	retval = listen(listen_sock, SOMAXCONN);// 상대방의 연결 요청을 받을 준비한다, 소켓 고유 숫자, 듣기 상태의 대기열
	if(retval == SOCKET_ERROR) err_quit("listen()"); // listen 실패

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;		// 클라이언트 소캣
	SOCKADDR_IN clientaddr; // 클라이언트 주소
	int addrlen;			// 주소 길이
	char buf[BUFSIZE + 1];	// 문자열에 있어서 널문자 한 칸을 위해 +1을 함.

	while (1)
	{
		// accept()
		addrlen = sizeof(clientaddr);		// 클라이언트 주소 길이
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen); // 클라이언트와 연결
		if(client_sock == INVALID_SOCKET)	// accept 오류
		{
			err_display("accept()");
			continue;
		}
		printf("\n[TCP 서버] 클라이언트 접속 : IP주소 = %s, 포트 번호 = %d\n", // 연결 성공시 메시지 출력
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while(1)
		{
			// 데이터 받기
			retval = recv(client_sock, buf, BUFSIZE, 0); // 소캣의 정보를 받음
			if(retval == SOCKET_ERROR) // recv오류
			{
				err_display("recv()");
				break;
			}
			else if(retval == 0 ) // 바이트수가 0이면 종료
				break;

			// 받은 데이터 출력
			buf[retval] = '\0'; // 마지막 문자에 널을 삽입하여 끝을 냄
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
					ntohs(clientaddr.sin_port),buf);
			
			// 데이터 보내기
			retval = send(client_sock, buf, retval, 0); // 클라이언트에게 데이터를 보냄
			if(retval == SOCKET_ERROR) // send에러
			{
				err_display("send()");
				break;
			}
		}
	

		// closesocket()
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP주소 = %s, 포트 번호 = %d\n",
				inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);

	//윈속 종료
	WSACleanup();
	return 0;
}