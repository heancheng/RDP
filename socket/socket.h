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
// 原始Socket接口，低级接口
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
// 原始的TCP接口，可以保证网络传输数据安全
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
// 对客户服务器处理，每个命令要求相应的数据处理，处理完需要确认信息。采用异步确认，可以大大提高网络传输速度。
#define TCPSEQ_HEAD_LEN	 (1+sizeof(unsigned long)+sizeof(unsigned long))
class TCPSeq : public TCP
{
public:
	enum {
		ReturnEvery =0,			// 每个信息处理完毕都返回
		ReturnWhenError=1,		// 信息处理错误时才返回
		Resend	= 2,			// 要求对端重新传送，一般是按照序列号，丢失数据后重新传送丢失的数据。
		ErrorResend=0,			// 错误重发
		ErrorContinue=16};		// 错误进错误队列，并继续处理后面事务

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
	int m_set;	// 设置：（处理完马上返回确认；处理错误才返回确认）（出错等待重新发送，出错继续后续处理但保留可重新发送队列）
	char m_pkg_head[TCPSEQ_HEAD_LEN]; // 1个字节控制或数据标识，序列号，数据长度
};

//原始的UDP接口
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
// UDP不保证数据到达的顺序，通过使用序列号和CRC校验，可以按照SN顺序重新组装数据顺序，同时通过CRC保证数据的正确性。
// 通过UDP的一定长度的缓冲队列，加快数据传输的效率，可以达到比TCP更加高速的效果。同时在单线程处理多客户端连接方面，UDP服务器具有更加优良的性能。
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
	char m_pkg_head[UDPSEQ_HEAD_LEN]; // 1个字节控制或数据标识，序列号，数据长度
	unsigned int m_buff_len;
	char * m_buff;
};

#endif

