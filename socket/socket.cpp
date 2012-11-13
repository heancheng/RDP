#include "socket.h"
#include <cstdlib>
extern "C"
{
#include <ctype.h>
}
#ifdef _MSC_VER

SocketInit _g_socket_init_;

SocketInit::SocketInit()
{
        wVersionRequested = MAKEWORD(2, 2);
		Startup();
        //if(!Startup())
		//	;//cout << "Init socket fail." << endl;
}
SocketInit::~SocketInit()
{
        Clearup();
}
int SocketInit::Startup()
{
        return (WSAStartup(wVersionRequested, &wsaData)==0);
}
void SocketInit::Clearup()
{
        WSACleanup( );
}
#endif
/////////////// Socket //////////////////////////////
Socket::Socket()
:m_socket(INVALID_SOCKET)
{
}

Socket::~Socket()
{
	Close();
}
unsigned long Socket::ParseAddress(const char * hostaddr) const
{
	if(hostaddr==NULL || hostaddr[0]=='\0')
		return INADDR_ANY;

	if(isdigit(hostaddr[0]))	// if is ip_address
	{
		return inet_addr(hostaddr);
	}
	else	// if is host name
	{
		hostent* host;
		unsigned char * p;
		host = gethostbyname(hostaddr);
		if(host==NULL)
			throw "Get hostbyname fail.";
		else
			p =(unsigned char *) host->h_addr_list[0];

		return *(long *)p;
	}
}
struct sockaddr_in Socket::GetSockAddr(const char * hostaddr, unsigned int port) const
{
	struct sockaddr_in addr;
	memset(&addr,'\0',sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port =  htons(port);
	addr.sin_addr.s_addr = ParseAddress(hostaddr);
	return addr;
}
int Socket::Create(int type)
{
	Close();
	m_socket = socket(AF_INET,type,0);
	return m_socket!=INVALID_SOCKET;
}
int Socket::Bind(unsigned int port, const char * hostaddr)
{
	struct sockaddr_in addr;
	addr = GetSockAddr(hostaddr,port);
	return bind(m_socket,(struct sockaddr *)&addr,sizeof(addr))!=SOCKET_ERROR;
}
void Socket::Close()
{
	if(m_socket!=INVALID_SOCKET)
	{
#ifdef _MSC_VER
		closesocket(m_socket);
#else
		close(m_socket);
#endif
		m_socket =INVALID_SOCKET;
	}
}
unsigned int Socket::GetMaxMsgSize() const
{
	unsigned int size;
	int len;
	len = sizeof(size);
	if(getsockopt(m_socket,SOL_SOCKET,SO_MAX_MSG_SIZE,(char *)&size,&len)==SOCKET_ERROR)
		throw "SOCKET_ERROR.";
	return size;
}
void Socket::SetKeepAlive(int bVal) const
{
	if(setsockopt(m_socket,SOL_SOCKET,SO_KEEPALIVE,(const char *)&bVal,sizeof(bVal))==SOCKET_ERROR)
		throw "SOCKET_ERROR.";
}
void Socket::SetOOBInline(int bVal) const
{
	if(setsockopt(m_socket,SOL_SOCKET,SO_OOBINLINE,(const char *)&bVal,sizeof(bVal))==SOCKET_ERROR)
		throw "SOCKET_ERROR.";
}
void Socket::SetLinger(int bOnOff,int timeout) const
{
	LINGER ling;
	ling.l_onoff = bOnOff;
	ling.l_linger = timeout;
	if(setsockopt(m_socket,SOL_SOCKET,SO_LINGER,(const char *)&ling,sizeof(ling))==SOCKET_ERROR)
		throw "SOCKET_ERROR.";
}
void Socket::SetReuseAddr(int bVal) const
{
	if(setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(const char *)&bVal,sizeof(bVal))==SOCKET_ERROR)
		throw "SOCKET_ERROR.";
}
void Socket::SetReceiveBuffSize(unsigned int size) const
{
	if(setsockopt(m_socket,SOL_SOCKET,SO_RCVBUF,(const char *)&size,sizeof(size))==SOCKET_ERROR)
		throw "SOCKET_ERROR.";
}
void Socket::SetSendBuffSize(unsigned int size) const
{
	if(setsockopt(m_socket,SOL_SOCKET,SO_SNDBUF,(const char *)&size,sizeof(size))==SOCKET_ERROR)
		throw "SOCKET_ERROR.";
}
void Socket::SetBroadcast(int bVal) const
{
	if(setsockopt(m_socket,SOL_SOCKET,SO_BROADCAST,(const char *)&bVal,sizeof(bVal))==SOCKET_ERROR)
		throw "SOCKET_ERROR.";
}
//////////////// SocketSelect ///////////////////////
void SocketSelect::Clear()
{
	FD_ZERO(&m_rset);
	FD_ZERO(&m_wset);
	FD_ZERO(&m_eset);
	m_sock_max = 0;
	m_timeout.tv_sec = 0;
	m_timeout.tv_usec = 0;
	m_bRead = m_bWrite = m_bExcept = m_bTimeout = 0;
}
int SocketSelect::Add(const Socket & sock)
{
	if(m_bRead)
		FD_SET(sock.m_socket,&m_rset);
	if(m_bWrite)
		FD_SET(sock.m_socket,&m_wset);
	if(m_bExcept)
		FD_SET(sock.m_socket,&m_eset);
	m_sock_max = (m_sock_max<sock.m_socket+1 ? sock.m_socket+1 : m_sock_max);
	return 1;
}
int SocketSelect::AddForSend(const Socket & sock)
{
	if(m_bWrite)
		FD_SET(sock.m_socket,&m_wset);
	m_sock_max = (m_sock_max<sock.m_socket+1 ? sock.m_socket+1 : m_sock_max);
	return 1;
}
int SocketSelect::AddForRecv(const Socket & sock)
{
	if(m_bRead)
		FD_SET(sock.m_socket,&m_rset);
	m_sock_max = (m_sock_max<sock.m_socket+1 ? sock.m_socket+1 : m_sock_max);
	return 1;
}
int SocketSelect::Select()
{
	fd_set *pr,*pw,*pe;
	struct timeval *pt;
	pr = pw = pe = NULL;
	pt = NULL;
	if(m_bRead)
		pr = &m_rset;
	if(m_bWrite)
		pw = &m_wset;
	if(m_bExcept)
		pe = &m_eset;
	if(m_bTimeout)
		pt = &m_timeout;
	return select(m_sock_max,pr,pw,pe,pt);
}
int SocketSelect::IsReadReady(const Socket & sock)
{
	return FD_ISSET(sock.m_socket,&m_rset);
}
int SocketSelect::IsWriteReady(const Socket & sock)
{
	return FD_ISSET(sock.m_socket,&m_wset);
}
int SocketSelect::IsExcept(const Socket & sock)
{
	return FD_ISSET(sock.m_socket,&m_eset);
}
///////////////// TCP /////////////////////
TCP::TCP()
{
}
TCP::~TCP()
{
	Close();
}
int TCP::Create()
{
	Close();
	return Socket::Create(SOCK_STREAM);
}

int TCP::Listen(int backlog)
{
	return listen(m_socket,backlog)!=SOCKET_ERROR;
}

int TCP::Accept(TCP & tcp)
{
	struct sockaddr addr;
	int    len;
	len = sizeof(addr);
	SOCKET socket = accept(m_socket,&addr,&len);
	if(socket == INVALID_SOCKET)
		return 0;
	else
	{
		//tcp.Close(); // 在客户端连接事务处理中关闭，不要在这里关闭，以免多任务，多线程时失控。
		tcp.m_socket = socket;
		tcp.m_is_client = 0;	// 服务器端
		return 1;
	}
}
int TCP::Connect(const char * hostaddr, unsigned int port)
{
	struct sockaddr_in addr;
	addr = GetSockAddr(hostaddr,port);
	m_is_client = 1;	// 客户端
	return connect(m_socket,(struct sockaddr *)&addr,sizeof(addr))!=SOCKET_ERROR;
}
int TCP::Receive(char * buff, int buf_size)
{
	return recv(m_socket,buff,buf_size,0);// if return SOCKET_ERROR , can call WSAGetLastError() to get errorcode
/*	int result;
        result = recv(m_socket,buff,buf_size,0);
	return result;
	switch(result)
	{
	case SOCKET_ERROR:
		throw "SOCKET_ERROR.";
	case 0:
#ifdef _MSC_VER
	case WSAECONNRESET:
#endif
		throw "SOCKET CLOSED."
	default:
		return result;
	}
*/
}
int TCP::Send(const char * buff, int len)
{
	return send(m_socket,buff,len,0);// if return SOCKET_ERROR , can call WSAGetLastError() to get errorcode
}
int TCP::IsSendReady()
{
	SocketSelect sock_sele;
	sock_sele.Clear();
	sock_sele.SetRead(0);
	sock_sele.SetWrite(1);
	sock_sele.SetExcept(0);
	sock_sele.SetTimeOut(1,0);
	sock_sele.Add(*this);
	sock_sele.Select();
	return sock_sele.IsWriteReady(*this);
}
int TCP::IsReceiveReady()
{
	SocketSelect sock_sele;
	sock_sele.Clear();
	sock_sele.SetRead(1);
	sock_sele.SetWrite(0);
	sock_sele.SetExcept(0);
	sock_sele.SetTimeOut(1,0);
	sock_sele.Add(*this);
	sock_sele.Select();
	return sock_sele.IsReadReady(*this);
}
int  TCP::CreateServer(unsigned int port, const char * hostaddr)
{
	if(!Create())
		return 0;
	this->SetReuseAddr();  // 使用reuseAddress，可以避免前面关闭的链接端口不能重复使用问题，提高端口利用率。
	if(!Bind(port,hostaddr))
		return 0;
	return Listen();
}
int  TCP::CreateClient(const char * hostaddr, unsigned int port)
{
	if(!Create())
		return 0;
	return Connect(hostaddr,port);
}
void TCP::Close(int soft_type)
{
	char buff[64];
	int len;
	if(m_socket!=INVALID_SOCKET)
	{
#ifdef _MSC_VER
		if(soft_type)
		{
			WSAEVENT NewEvent;
			NewEvent = WSACreateEvent();
			WSAEventSelect(m_socket, NewEvent, FD_CLOSE);
			shutdown(m_socket,SD_SEND);
			//Sleep(500);
			while((len=recv(m_socket,buff,64,0))>0)
				;
			closesocket(m_socket);
			WSACloseEvent(NewEvent);
		}
		else
		{
			closesocket(m_socket);
		}
#else
		if(soft_type)
		{
			shutdown(m_socket,SD_SEND);
			while((len=recv(m_socket,buff,64,0))>0)
				;
		}
		close(m_socket);
#endif
		m_socket = INVALID_SOCKET;
	}
}
/////////////////// UDP //////////////////
UDP::UDP()
{
}
UDP::~UDP()
{
	Close();
}
int UDP::Create()
{
	Close();
	return Socket::Create(SOCK_DGRAM);
}
int UDP::CreateServer(unsigned int port)
{
		int result = Create();
		result = result ? Bind(port):result;
		return result;
}
void UDP::Close()
{
	Socket::Close();
}
int UDP::SendTo(const struct sockaddr_in & addr,const char * buff, unsigned int len)
{
	return sendto(m_socket,buff,len,0,(const struct sockaddr *)&addr,sizeof(addr));
}
int UDP::SendTo(const char * host_addr,unsigned int port,const char * buff, unsigned int len)
{
	struct sockaddr_in svr_addr;
	svr_addr = GetSockAddr(host_addr,port);
	return SendTo(svr_addr,buff,len);
}
int UDP::ReceiveFrom(struct sockaddr_in & addr, char * buff, unsigned int len)
{
	int size;
	size = sizeof(addr);
	return recvfrom(m_socket,buff,len,0,(struct sockaddr *)&addr, &size);
}
int UDP::ReceiveFrom(const char * host_addr,unsigned int port,char * buff, unsigned int len)
{
	struct sockaddr_in svr_addr;
	svr_addr = GetSockAddr(host_addr,port);
	return ReceiveFrom(svr_addr,buff,len);
}
////////////////////////// TCPSeq ///////////////////
int TCPSeq::Receive(char * buff, int len)
{
	int n,r,msg_len,eff_len;
	n = 0;
	while(n<TCPSEQ_HEAD_LEN && (r=TCP::Receive(&m_pkg_head[n],TCPSEQ_HEAD_LEN-n))>0)
	{
		n += r;
	}
	if(r<=0)
		return 0;
	msg_len = GetHeadMsgLen();
	eff_len = (msg_len>len) ? len : msg_len;
	n = 0;
	while(n<eff_len && (r=TCP::Receive(&buff[n],eff_len-n))>0)
	{
		n += r;
	}
	if(r<=0)
		return 0;
	if(msg_len>eff_len)
	{
		char tmp[1024];
		while(n<msg_len && (r=TCP::Receive(tmp,(msg_len-n)>1024 ? 1024 : msg_len-n))>0)
		{
			n += r;
		}
	}
	if(r<=0)
		return 0;
	return eff_len;
}
int TCPSeq::Send(const char * buff, int len)
{
	int n,r;
	m_serial_number ++;
	SetHeadSerial(m_serial_number);
	SetHeadMsgLen(len);
	n = 0;
	while(n<TCPSEQ_HEAD_LEN && (r=TCP::Send(&m_pkg_head[n],TCPSEQ_HEAD_LEN-n))>0)
	{
		n += r;
	}
	if(r<=0)
		return 0;
	n = 0;
	while(n<len && (r=TCP::Send(&buff[n],len-n))>0)
	{
		n += r;
	}
	if(r<=0)
		return 0;
	return len;
}
unsigned long TCPSeq::GetHeadSerial()
{
	return ntohl(*(unsigned long *)&m_pkg_head[1]);
}
void TCPSeq::SetHeadSerial(unsigned long serial)
{
	*(unsigned long *)&m_pkg_head[1] = htonl(serial);
}
unsigned long TCPSeq::GetHeadMsgLen()
{
	return ntohl(*(unsigned long*)&m_pkg_head[TCPSEQ_HEAD_LEN-sizeof(unsigned long)]);
}
void TCPSeq::SetHeadMsgLen(unsigned long msg_len)
{
	*(unsigned long*)&m_pkg_head[TCPSEQ_HEAD_LEN-sizeof(unsigned long)] = htonl(msg_len);
}
////////////////////////// UDPSeq //////////////////
UDPSeq::~UDPSeq()
{
	if(m_buff!=NULL)
		delete [] m_buff;
}
int UDPSeq::ResetBuff(unsigned int len)
{
	if(m_buff_len<len)
	{
		m_buff_len=len;
		if(m_buff!=NULL)
			delete [] m_buff;
		m_buff = new char[m_buff_len];
		if(m_buff==NULL)
			throw "UDPSeq::ResetBuff,not enough memory.";
	}
	return 1;
}
char UDPSeq::ComputeCRC(const char * buff, unsigned int len)
{
	unsigned int i;
	char crc,bit0;//,bit8;
	crc = '\xFF';
	for(i=0;i<len;i++)
	{
		bit0 = crc & '\x01';
		crc >>=1;
		crc &= '\x7F';
		crc = (bit0 ? crc|'\x80' : crc);
		crc ^=buff[i];
	}
	return crc;
}
int UDPSeq::SendTo(const struct sockaddr_in & addr,unsigned long sn,const char * buff, unsigned int len)
{
	int send_len ;
	send_len = UDPSEQ_HEAD_LEN+len;
	ResetBuff(send_len);
	*(unsigned long *)&m_pkg_head[1] = htonl(sn);
	*(unsigned long*)&m_pkg_head[UDPSEQ_HEAD_LEN-sizeof(unsigned long)] = htonl(len);
	memcpy(m_buff,m_pkg_head,UDPSEQ_HEAD_LEN);
	memcpy(&m_buff[UDPSEQ_HEAD_LEN],buff,len);
	m_buff[0] = ComputeCRC(&m_buff[1],send_len-1);
	return UDP::SendTo(addr,m_buff,send_len)==send_len;
}
int UDPSeq::ReceiveFrom(struct sockaddr_in & addr,unsigned long &sn,char * buff, unsigned int &len)
{
	unsigned int recv_len ;
	recv_len = UDPSEQ_HEAD_LEN+len;
	ResetBuff(recv_len);
	recv_len = UDP::ReceiveFrom(addr,m_buff,recv_len);
	if(recv_len<=0)
		throw "UDP::ReceiveFrom,return error.";
	if(recv_len<UDPSEQ_HEAD_LEN)
		throw "UDPSeq::ReceiveFrom,mesg length too short.";
	memcpy(m_pkg_head,m_buff,UDPSEQ_HEAD_LEN);
	sn = ntohl(*(unsigned long *)&m_pkg_head[1]);
	len = ntohl(*(unsigned long*)&m_pkg_head[UDPSEQ_HEAD_LEN-sizeof(unsigned long)]);
	if(len!=recv_len-UDPSEQ_HEAD_LEN)
		throw "UDPSeq::ReceiveFrom,mesg length error.";
	if(m_buff[0] != ComputeCRC(&m_buff[1],recv_len-1))
		throw "UDPSeq::ReceiveFrom,CRC error.";
	memcpy(buff,&m_buff[UDPSEQ_HEAD_LEN],len);
	return 1;
}
////////////////////////// end //////////////////////

