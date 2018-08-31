#include <WinSock2.h> // 윈도우에서 소켓을 사용할 수 있게 함
#include <stdlib.h>
#include <stdio.h>
#undef UNICODE
#undef _UNICODE
#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE 512 // 클라이언트에서 받을 데이터 버퍼 사이즈

// 소켓 함수 오류 출력
void err_Msg(const char *msg, bool option)
{
	LPVOID lpMsgBuf;					// 메시지 버퍼
	FormatMessage(						 // 에러 메세지 형식 설정
		FORMAT_MESSAGE_ALLOCATE_BUFFER | // message의 메모리를 시스템에서 할당 해달라고 요청
		FORMAT_MESSAGE_FROM_SYSTEM,      // 운영체제로부터 오류메세지를 가져옴.   
		NULL, WSAGetLastError(),         // 가장 최근 발생한 소켓에러에 대응되는 에러코드를 반환
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // message의 언어를 지정해주는 부분
		(LPTSTR)&lpMsgBuf, 0, NULL);
	if (option)				// 옵션 설정 시 설정된 메시지 박스 출력
	{
		MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR); // 에러 시 메세지 박스 출력
		LocalFree(lpMsgBuf); // FORMAT_MESSAGE_ALLOCATE_BUFFER를 플래그로 지정한 경우엔
		exit(-1);			 // 사용자가 직접 메모리를 해제해주어야 메모리 누수가 발생하지 않음.
	}
	else					// 옵션 비 설정시 메시지만 출력
	{
		printf("[%s]%s", msg, lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
}

int main(int argc, char *argv[])
{
	int retval;				// 돌려줄 값

							// 윈속 정보를 초기화 한다.
	WSADATA wsa;			// WSADATA 구조체 변수 wsa
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) // 버전과 정보를 입력 받는데 실패 한다면 -1리턴
		return -1;

	// socket(UDP소켓 생성)
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);	// IPv4프로토콜, 비연결 지향형(UDP) 소켓 생성
	if (sock == INVALID_SOCKET) err_Msg("socket()", TRUE); // 소캣 함수가 오류 날 때 메시지 출력

														   // Bind(), 소켓 주소와 고유 숫자를 묶어줌
	SOCKADDR_IN serveraddr; // 소켓 주소 입력 변수
	ZeroMemory(&serveraddr, sizeof(serveraddr)); // serveraddr에 serveraddr만큼 메모리를 0으로 초기화
	serveraddr.sin_family = AF_INET;  // IPv4
	serveraddr.sin_port = htons(9501);// 포트 넘버
	serveraddr.sin_addr.s_addr = inet_addr("1.214.232.37"); // 보낼 ip주소
														
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // 소켓과 소켓 주소를 묶는다
	if (retval == SOCKET_ERROR) err_Msg("connect()", TRUE); // 연결 실패

	// 데이터 통신에 사용할 변수
	SOCKADDR_IN	peeraddr; // 클라이언트 주소
	int addrlen;			// 주소 길이
	char buf[BUFSIZE + 1];	// 문자열에 있어서 널문자 한 칸을 위해 +1을 함.
	int len;

	// 서버와 데이터 통신
	while (1)	// 연결이 끝날 때 까지 반복
	{
		// 데이터 입력
		printf("\n[보낼 데이터]");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)	break; // 엔터만 입력하면 종료

		// '\n' 문자 제거
		len = strlen(buf);
		if (buf[len - 1] == '\n') 
			buf[len - 1] = '\0';
		if (strlen(buf) == 0) break;

		// 데이터 보내기
		retval = sendto(sock, buf, strlen(buf), 0,
			(SOCKADDR *)&serveraddr, sizeof(serveraddr)); // 서버에게 데이터를 보냄
		if (retval == SOCKET_ERROR) { // 소켓 에러가 나면 에러 메세지 출력
			err_Msg("sendto()", FALSE);	continue;
		}

		// 데이터 받기
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, retval, 0, (SOCKADDR*)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) { // 소켓 에러가 나면 에러 메세지 출력
			err_Msg("recvfrom()", FALSE);
			continue;
		}

		// 송신자의 IP주소 체크
		if (memcmp(&peeraddr, &serveraddr, sizeof(peeraddr))) { // peeraddr와 serveraddr의 길이가 다르면
			printf("[오류] 잘못된 데이터 입니다.\n");          // 에러 출력
			continue;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[UDP 클라이언트]%d바이트를 받았습니다.\n", retval);
		printf("[받은 데이터]%s\n", buf);
	}

	// closesocket()
	closesocket(sock); // 소켓종료

	//윈속 종료
	WSACleanup();
	return 0;
}