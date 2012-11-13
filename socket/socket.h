#ifndef _SOCKET_HAC_HPP
#define _SOCKET_HAC_HPP
#ifdef _MSC_VER
	#define FD_SETSIZE 64
	#include <winsock2.h>
	//#include <winsock.h>
	#include <sys/types.h>
	#define socklen_t int
	#pragma comment(lib,"ws2_32.lib")
	class SocketInit
	{
	public:
	       	SocketInit();
	        ~SocketInit();
	        int Startup();
	        void Clearup();
	private:
	        WORD wVersionRequested;
	        WSADATA wsaData;
	};
#else
	#include <sys/types.h>
	#include <sys/socket.h>	// used in GNU C++
	#include <unistd.h>  // used in GNU C++
	#define SOCKET	int
#endif
#include <cstdio>
class SocketSelect;
// ԭʼSocket�ӿڣ��ͼ��ӿ�
class Socket
{
	friend class SocketSelect;
public:
	Socket();
	virtual ~Socket();
	int Create(int type=SOCK_STREAM); // SOCK_DGRAM
	void Close();
	int Bind(unsigned int port, const char * hostaddr=NULL);
	unsigned long ParseAddress(const char * hostaddr=NULL) const;
	struct sockaddr_in GetSockAddr(const char * hostaddr=NULL,unsigned int port = 0) const;
	unsigned int GetMaxMsgSize() const;
	void SetKeepAlive(int bVal=1) const;
	void SetOOBInline(int bVal=1) const;
	void SetLinger(int bOnOff=1,int timeout=30) const; // default:timeout=30 second
	void SetReuseAddr(int bVal=1) const;
	void SetReceiveBuffSize(unsigned int size) const;
	void SetSendBuffSize(unsigned int size) const;
	void SetBroadcast(int bVal=1) const;
protected:
	SOCKET m_socket;
};

class SocketSelect
{
public:
	SocketSelect(){Clear();}
	~SocketSelect(){}
	void Clear();
	void SetRead(int bVal=1){m_bRead=bVal;}
	void SetReceive(int bVal=1){SetRead(bVal);}
	void SetWrite(int bVal=1){m_bWrite=bVal;}
	void SetSend(int bVal=1){SetWrite(bVal);}
	void SetExcept(int bVal=1){m_bExcept=bVal;}
	void SetTimeOut(int bVal=1,long sec=30, long usec=0){
		m_bTimeout=bVal;m_timeout.tv_sec=sec;m_timeout.tv_usec=usec;}
	int Select();
	int IsReadReady(const Socket & sock);
	int IsWriteReady(const Socket & sock);
	int IsExcept(const Socket & sock);
	int Add(const Socket & sock);
	int AddForSend(const Socket & sock);
	int AddForRecv(const Socket & sock);
private:
	int m_bRead,m_bWrite,m_bExcept,m_bTimeout;
	fd_set m_rset, m_wset,m_eset;
	SOCKET m_sock_max;
	struct timeval m_timeout;
};
// ԭʼ��TCP�ӿڣ����Ա�֤���紫�����ݰ�ȫ
class TCP : public Socket
{
public:
	TCP();
	virtual ~TCP();
	int CreateServer(unsigned int port,const char * hostaddr=NULL);
	int CreateClient(const char * hostaddr, unsigned int port);
	int Accept(TCP & tcp);
	void Attach(SOCKET sock){m_socket = sock;}
	SOCKET GetSocket(){return m_socket;}
	int Receive(char * buff, int len);
	int Send(const char * buff, int len);
	int IsSendReady();
	int IsReceiveReady();
	void Close(int soft_type=0);
protected:
	int Create();
	int Listen(int backlog=5);
	int Connect(const char * hostaddr, unsigned int port);
protected:
	char m_is_client;
};
// �Կͻ�����������ÿ������Ҫ����Ӧ�����ݴ�����������Ҫȷ����Ϣ�������첽ȷ�ϣ����Դ��������紫���ٶȡ�
#define TCPSEQ_HEAD_LEN	 (1+sizeof(unsigned long)+sizeof(unsigned long))
class TCPSeq : public TCP
{
public:
	enum {
		ReturnEvery =0,			// ÿ����Ϣ������϶�����
		ReturnWhenError=1,		// ��Ϣ�������ʱ�ŷ���
		Resend	= 2,			// Ҫ��Զ����´��ͣ�һ���ǰ������кţ���ʧ���ݺ����´��Ͷ�ʧ�����ݡ�
		ErrorResend=0,			// �����ط�
		ErrorContinue=16};		// �����������У������������������

public:
	TCPSeq():m_serial_number(0),m_set(0){}
	virtual ~TCPSeq(){}
	int Receive(char * buff, int len);
	int Send(const char * buff, int len);
	int SendCtrl();
	int Set(int mode);
	unsigned long GetHeadSerial();
private:
	void SetHeadSerial(unsigned long serial);
	unsigned long GetHeadMsgLen();
	void SetHeadMsgLen(unsigned long msg_len);

	unsigned long m_serial_number;
	int m_set;	// ���ã������������Ϸ���ȷ�ϣ��������ŷ���ȷ�ϣ�������ȴ����·��ͣ�������������������������·��Ͷ��У�
	char m_pkg_head[TCPSEQ_HEAD_LEN]; // 1���ֽڿ��ƻ����ݱ�ʶ�����кţ����ݳ���
};

//ԭʼ��UDP�ӿ�
class UDP : public Socket
{
public:
	UDP();
	virtual ~UDP();
	int Create();
	int CreateServer(unsigned int port);
	int SendTo(const struct sockaddr_in & addr, const char * buff, unsigned int len);
	int SendTo(const char * host_addr, unsigned int port, const char * buff, unsigned int len);
	int ReceiveFrom(struct sockaddr_in & addr, char * buff, unsigned int len);
	int ReceiveFrom(const char * host_addr, unsigned int port, char * buff, unsigned int len);
	void Close();
};
// UDP����֤���ݵ����˳��ͨ��ʹ�����кź�CRCУ�飬���԰���SN˳��������װ����˳��ͬʱͨ��CRC��֤���ݵ���ȷ�ԡ�
// ͨ��UDP��һ�����ȵĻ�����У��ӿ����ݴ����Ч�ʣ����Դﵽ��TCP���Ӹ��ٵ�Ч����ͬʱ�ڵ��̴߳����ͻ������ӷ��棬UDP���������и������������ܡ�
#define UDPSEQ_HEAD_LEN	 (1+sizeof(unsigned long)+sizeof(unsigned long)) // CRC+SN+LEN
class UDPSeq : public UDP
{
public:
	UDPSeq():m_buff_len(0),m_buff(NULL){}
	virtual ~UDPSeq();

	int SendTo(const struct sockaddr_in & addr,unsigned long sn,const char * buff, unsigned int len);
	int ReceiveFrom(struct sockaddr_in & addr,unsigned long & sn, char * buff, unsigned int &len);
private:
	int  ResetBuff(unsigned int len);
	char ComputeCRC(const char * buff, unsigned int len);
	char m_pkg_head[UDPSEQ_HEAD_LEN]; // 1���ֽڿ��ƻ����ݱ�ʶ�����кţ����ݳ���
	unsigned int m_buff_len;
	char * m_buff;
};

#endif

