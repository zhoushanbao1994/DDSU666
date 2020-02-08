#include "TcpThread.h"
#include "easylogging++.h"
#include <stdio.h>

//#define __DEBUG__

using namespace std;

TcpThread::TcpThread(int clientSocket, sockaddr_in clientAddr)
{
	m_sendTestFlag = 1;
	m_sendFlag = 1;
	m_modbusDevId = 0;
	m_clientSocket = clientSocket;
    m_clientAddr = clientAddr;
    
}
TcpThread::~TcpThread()
{

}
void TcpThread::TcpRecvThread() // 接收
{
	static char recvBuf[2048] = { 0 };
	for (;;) {
        // server读取client端键入的数据(第二类socket的句柄，存储数据的地方,flag)
		int recvLen = recv(m_clientSocket, recvBuf, sizeof(recvBuf), 0); 
		if (recvLen <= 0) {
			m_sendFlag = 0;
			LOG(INFO) << "[Socket " << m_clientSocket << "] [" << 
                inet_ntoa(m_clientAddr.sin_addr) << ":" << ntohs(m_clientAddr.sin_port) << "disconnect!";
			return;
		}
#ifdef __DEBUG__
        // 服务器显示客户端键入的字符串长度
		printf("[Socket %d] [%s:%d] Recv: ", 
            m_clientSocket, inet_ntoa(m_clientAddr.sin_addr), ntohs(m_clientAddr.sin_port)); 
		for (int i = 0; i < recvLen; i++) {
			printf(" %02X", recvBuf[i] & 0xFF);
		}
		printf("\r\n");
#endif

		// 非测试模式 并且 返回的设备地址不对 
		if ((m_sendTestFlag == 0) && ((recvBuf[0] & 0xFF) != m_modbusDevId)) {
			continue;
		}

		//功能码不对
		if ((recvBuf[1] & 0xFF) != 0x03) {
			continue;
		}

		if ((recvBuf[2] & 0xFF) == 0x22) {
			DataAnalysis1(&(recvBuf[3]));
		}
		else if ((recvBuf[2] & 0xFF) == 0x24) {
			DataAnalysis2(&(recvBuf[3]));
		}
		else if ((recvBuf[2] & 0xFF) == 0x04) {
			DataAnalysis3(&(recvBuf[3]));
		}

	}

#ifdef _WIN32 // 读取数据的第二类socket创建后要记得关闭
	closesocket(m_clientSocket);
#else
	close(m_clientSocket);
#endif

	//delete this; // 调用完后，自己清理调第二类socket的对象
}


void TcpThread::TcpSendThread() // 发送
{
	// 获取设备地址
	LOG(INFO) << "------------获取设备地址......--------------";
	int testModbusDevId = 0x01;
	while (m_sendTestFlag && m_sendFlag) {
		DataRead1(testModbusDevId++);
		sleep(1);		// 靠1s
		if (testModbusDevId > 0xFF) {
			testModbusDevId = 0x01;
		}
	}

	// 读取数据
	while (m_sendFlag) {
		LOG(INFO) << "[" << m_modbusDevId << "] ------------------------------------";
		DataRead2();
		sleep(1);
		DataRead3();
		sleep(10);
	}

	delete this; // 调用完后，自己清理调第二类socket的对象
}

// CRC运算
unsigned int TcpThread::CrcCal(const char* pBuff, int len)
{
	unsigned int mid = 0;
	unsigned char times = 0, Data_index = 0;
	unsigned int  cradta = 0xFFFF;
	while (len)
	{
        //把数据帧中的第一个字节的8位与CRC寄存器中的低字节进行异或运算，结果存回CRC寄存器
		cradta = pBuff[Data_index] ^ cradta;
		for (times = 0; times < 8; times++) {
			mid = cradta;
			cradta = cradta >> 1;
			if (mid & 0x0001) {
				cradta = cradta ^ 0xA001;
			}
		}
		Data_index++;
		len--;
	}
	return cradta;
}

// hex转dec
long TcpThread::HextoDec(const char* hex, int start, int length)
{
	int i;
	int end = start + length;
	long dec = 0;
	for (i = start; i < end; i++) {
		dec = dec << 8;
		dec += hex[i];
	}
	return dec;
}

void TcpThread::DataAnalysis1(const char* pBuff)
{
	unsigned int iUcodE = HextoDec(pBuff, 0, 2);
	unsigned int iREV = HextoDec(pBuff, 2, 2);
	unsigned int iAddr = HextoDec(pBuff, 12, 2);
	m_modbusDevId = iAddr;
	unsigned int iBAud = HextoDec(pBuff, 24, 2);
	LOG(INFO) << "[" << m_modbusDevId << "] UcodE:" << iUcodE << ",  REV:" << iREV << ",  Addr:" << iAddr << ",  BAud:" << iBAud;

	m_sendTestFlag = 0;
}
void TcpThread::DataAnalysis2(const char* pBuff)
{
	unsigned int iV = HextoDec(pBuff, 0, 4);
	float* fV = (float*)& iV;

	unsigned int iI = HextoDec(pBuff, 4, 4);
	float* fI = (float*)& iI;

	unsigned int iP1 = HextoDec(pBuff, 8, 4);
	unsigned int iP2 = HextoDec(pBuff, 12, 4);
	unsigned int iP3 = HextoDec(pBuff, 16, 4);
	float* fP1 = (float*)& iP1;
	float* fP2 = (float*)& iP2;
	float* fP3 = (float*)& iP3;

	unsigned int iPF = HextoDec(pBuff, 20, 4);
	float* fPF = (float*)& iPF;

	unsigned int iF = HextoDec(pBuff, 28, 4);
	float* fF = (float*)& iF;
    
	LOG(INFO) << "[" << m_modbusDevId << "] V:" << *fV << "V,  I:" << *fI << "A,  "
        << "P:" << *fP1 << "kw " << *fP2 << "kw " << *fP3 << "kw,  PF:" << *fPF << " F:" << *fF << "Hz";
}
void TcpThread::DataAnalysis3(const char* pBuff)
{
	unsigned int iImpEp = HextoDec(pBuff, 0, 4);
	float* fImpEp = (float*)& iImpEp;
	LOG(INFO) << "[" << m_modbusDevId << "] ImpEp:" << *fImpEp << "Kwh";
}


void TcpThread::DataRead1(int testModbusDevId)
{
	static char sendBuf[1024] = { 0 };
	int len = 8;
	sendBuf[0] = testModbusDevId;
	sendBuf[1] = 0x03;	// 功能码
	sendBuf[2] = 0x00;	// 起始地址高
	sendBuf[3] = 0x00;	// 起始地址低
	sendBuf[4] = 0x00;	// 数据个数高
	sendBuf[5] = 0x11;	// 数据个数低
	int crc = CrcCal(sendBuf, len - 2);
	sendBuf[6] = crc & 0xFF;			// 校验位低
	sendBuf[7] = (crc & 0xFF00) >> 8;	// 校验位高
	if (send(m_clientSocket, sendBuf, len, 0) <= 0) {
		LOG(INFO) << "[" << m_modbusDevId << "] [Socket " << m_clientSocket << "] [" << 
            inet_ntoa(m_clientAddr.sin_addr) << ":" << ntohs(m_clientAddr.sin_port) << "Send error!";
	}
	else {
#ifdef __DEBUG__
		printf("[Socket %d] [%s:%d] Send: ",  
                m_clientSocket, inet_ntoa(m_clientAddr.sin_addr), ntohs(m_clientAddr.sin_port));
		for (int i = 0; i < len; i++) {
			printf(" %02X", sendBuf[i] & 0xFF);
		}
		printf("\r\n");
#endif
	}
}
void TcpThread::DataRead2()
{
	static char sendBuf[1024] = { 0 };
	int len = 8;
	sendBuf[0] = m_modbusDevId;
	sendBuf[1] = 0x03;	// 功能码
	sendBuf[2] = 0x20;	// 起始地址高
	sendBuf[3] = 0x00;	// 起始地址低
	sendBuf[4] = 0x00;	// 数据个数高
	sendBuf[5] = 0x12;	// 数据个数低
	int crc = CrcCal(sendBuf, len - 2);
	sendBuf[6] = crc & 0xFF;			// 校验位低
	sendBuf[7] = (crc & 0xFF00) >> 8;	// 校验位高
	if (send(m_clientSocket, sendBuf, len, 0) <= 0) {
		LOG(INFO) << "[" << m_modbusDevId << "] [Socket " << m_clientSocket << "] [" << 
            inet_ntoa(m_clientAddr.sin_addr) << ":" << ntohs(m_clientAddr.sin_port) << "Send error!";
	}
	else {
#ifdef __DEBUG__
		printf("[Socket %d] [%s:%d] Send: ",  
                m_clientSocket, inet_ntoa(m_clientAddr.sin_addr), ntohs(m_clientAddr.sin_port));
		for (int i = 0; i < len; i++) {
			printf(" %02X", sendBuf[i] & 0xFF);
		}
		printf("\r\n");
#endif
	}
}
void TcpThread::DataRead3()
{
	static char sendBuf[1024] = { 0 };
	int len = 8;
	sendBuf[0] = m_modbusDevId;
	sendBuf[1] = 0x03;	// 功能码
	sendBuf[2] = 0x40;	// 起始地址高
	sendBuf[3] = 0x00;	// 起始地址低
	sendBuf[4] = 0x00;	// 数据个数高
	sendBuf[5] = 0x02;	// 数据个数低
	int crc = CrcCal(sendBuf, len - 2);
	sendBuf[6] = crc & 0xFF;			// 校验位低
	sendBuf[7] = (crc & 0xFF00) >> 8;	// 校验位高
	if (send(m_clientSocket, sendBuf, len, 0) <= 0) {
		LOG(INFO) << "[" << m_modbusDevId << "] [Socket " << m_clientSocket << "] [" << 
            inet_ntoa(m_clientAddr.sin_addr) << ":" << ntohs(m_clientAddr.sin_port) << "Send error!";
	}
	else {
#ifdef __DEBUG__
		printf("[Socket %d] [%s:%d] Send: ",  
                m_clientSocket, inet_ntoa(m_clientAddr.sin_addr), ntohs(m_clientAddr.sin_port));
		for (int i = 0; i < len; i++) {
			printf(" %02X", sendBuf[i] & 0xFF);
		}
		printf("\r\n");
#endif
	}
}
