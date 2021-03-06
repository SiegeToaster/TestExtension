/*
* TestExtension.cpp : Defines the exported functions for the DLL.
* 
* https://stackoverflow.com/questions/1011339/how-do-you-make-a-http-request-with-c
*/


#include "pch.h"
#include "framework.h"
#include "TestExtension.h"
#include <Windows.h>
#include <winsock.h> // needed for WSADATA
#include <string>
#include <vector>
#include <stdio.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

HINSTANCE hInst;
WSADATA wsaData;


extern "C"
{
	__declspec (dllexport) void __stdcall RVExtension(char* output, int outputSize, const char* function);
	__declspec (dllexport) int __stdcall RVExtensionArgs(char* output, int outputSize, const char* function, const char** argv, int argc);
	__declspec (dllexport) void __stdcall RVExtensionVersion(char* output, int outputsize);
}

void __stdcall RVExtension(char* output, int outputSize, const char* input) {
	// do URL call stuff here
	vector<string> testVector = explode(input, ',');
	string testString(testVector.begin(), testVector.end());
	char* testInput = const_cast<char*>(testString.c_str());
	strncpy_s(output, outputSize, testInput, _TRUNCATE);
}

int __stdcall RVExtensionArgs(char* output, int outputSize, const char* function, const char** argv, int argc) {
	strncpy_s(output, outputSize, "Invalid params (use callExtension primary syntax)", _TRUNCATE);
	return 0;
}

void __stdcall RVExtensionVersion(char* output, int outputSize) {
	strncpy_s(output, outputSize, "Test-Extension v0.2", _TRUNCATE);
}

vector<string> explode(const string& str, const char& ch) {
	// https://stackoverflow.com/questions/890164/how-can-i-split-a-string-by-a-delimiter-into-an-array
	string next;
	vector<string> result;

	// For each character in the string
	for (string::const_iterator it = str.begin(); it != str.end(); it++) {
		// If we've hit the terminal character
		if (*it == ch) {
			// If we have some characters accumulated
			if (!next.empty()) {
				// Add them to the result vector
				result.push_back(next);
				next.clear();
			}
		}
		else {
			// Accumulate the next character into the sequence
			next += *it;
		}
	}
	if (!next.empty())
		result.push_back(next);
	return result;
}

void mParseUrl(char* mUrl, string& serverName, string& filePath, string& fileName) {
	string::size_type n;
	string url = mUrl;

	if (url.substr(0, 7) == "http://")
		url.erase(0, 7);

	if (url.substr(0, 8) == "https://")
		url.erase(0, 8);

	n = url.find('/');
	if (n != string::npos) {
		serverName = url.substr(0, n);
		filePath = url.substr(n);
		n = filePath.rfind('/');
		fileName = filePath.substr(n + 1);
	} else {
		serverName = url;
		filePath = "/";
		fileName = "";
	}
}

SOCKET connectToServer(char* szServerName, WORD portNum) {
	struct hostent* hp;
	unsigned int addr;
	struct sockaddr_in server;
	SOCKET conn;
	conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn == INVALID_SOCKET)
		return NULL;

	if (inet_addr(szServerName) == INADDR_NONE) {
		hp = gethostbyname(szServerName);
	} else {
		addr = inet_addr(szServerName);
		hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
	}

	if (hp == NULL) {
		closesocket(conn);
		return NULL;
	}

	server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(portNum);
	if (connect(conn, (struct sockaddr*)&server, sizeof(server))) {
		closesocket(conn);
		return NULL;
	}

	return conn;
}

int getHeaderLength(char* content) {
	const char* srchStr1 = "\r\n\r\n", * srchStr2 = "\n\r\n\r";
	char* findPos;
	int offset = -1;

	findPos = strstr(content, srchStr1);
	if (findPos != NULL) {
		offset = findPos - content;
		offset += strlen(srchStr1);
	} else {
		findPos = strstr(content, srchStr2);
		if (findPos != NULL) {
			offset = findPos - content;
			offset += strlen(srchStr2);
		}
	}

	return offset;
}

char* readUrl2(char* szUrl, long& bytesReturnedOut, char** headerOut) {
	const int bufSize = 512;
	char readBuffer[bufSize], sendBuffer[bufSize], tmpBuffer[bufSize];
	char* tmpResult = NULL, * result;
	SOCKET conn;
	string server, filepath, filename;
	long totalBytesRead, thisReadSize, headerLen;

	mParseUrl(szUrl, server, filepath, filename);

	///////////// step 1, connect //////////////////////
	conn = connectToServer((char*)server.c_str(), 80);

	///////////// step 2, send GET request /////////////
	sprintf(tmpBuffer, "GET %s HTTP/1.0", filepath.c_str());
	strcpy(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	sprintf(tmpBuffer, "Host: %s", server.c_str());
	strcat(sendBuffer, tmpBuffer);
	strcat(sendBuffer, "\r\n");
	strcat(sendBuffer, "\r\n");
	send(conn, sendBuffer, strlen(sendBuffer), 0);

	//    SetWindowText(edit3Hwnd, sendBuffer);
	printf("Buffer being sent:\n%s", sendBuffer);

	///////////// step 3 - get received bytes ////////////////
	// Receive until the peer closes the connection
	totalBytesRead = 0;
	while (1)
	{
		memset(readBuffer, 0, bufSize);
		thisReadSize = recv(conn, readBuffer, bufSize, 0);

		if (thisReadSize <= 0)
			break;

		tmpResult = (char*)realloc(tmpResult, thisReadSize + totalBytesRead);

		memcpy(tmpResult + totalBytesRead, readBuffer, thisReadSize);
		totalBytesRead += thisReadSize;
	}

	headerLen = getHeaderLength(tmpResult);
	long contenLen = totalBytesRead - headerLen;
	result = new char[contenLen + 1];
	memcpy(result, tmpResult + headerLen, contenLen);
	result[contenLen] = 0x0;
	char* myTmp;

	myTmp = new char[headerLen + 1];
	strncpy(myTmp, tmpResult, headerLen);
	myTmp[headerLen] = NULL;
	delete(tmpResult);
	*headerOut = myTmp;

	bytesReturnedOut = contenLen;
	closesocket(conn);
	return(result);
}