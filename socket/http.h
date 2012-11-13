#ifndef _HTTP_HPP
#define _HTTP_HPP
#include "socket.h"
#include "lock.h" // for cout << kad  debug
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}
#include <string>
#include <map>
using namespace std;

#define HTTP_OK			"HTTP/1.1 200 OK\r\n"
#define HTTP_CREATED	"HTTP/1.1 201 Created\r\n"		// Post����Դ�Ѿ�������Ӧ�����һ���µ�URI
#define HTTP_ACCEPTED	"HTTP/1.1 202 Accepted\r\n"		// �����Ѿ������ܴ�������û�����
#define HTTP_NO_CONTENT	"HTTP/1.1 204 No Content\r\n"	// ��������������󣬵���û���µ����ݷ��ͻ�ȥ
#define HTTP_NOT_MODIFIED	"HTTP/1.1 304 Not Modified\r\n" 
// ����ͻ�����һ��If-Modified-Sinceͷ�귢��GET���󣬶�����Դָ��������������û���޸ģ��㷵�ش�״̬��Ӧ��Ӧ�ð���Ӧ��ͷ�꣬������ڡ�
#define HTTP_FORBIDDEN	"HTTP/1.1 403 Forbidden\r\n"	// ��ִֹ��
#define HTTP_NOT_FOUND	"HTTP/1.1 404 Not Found\r\n"	// û���ҵ��ļ�
#define HTTP_PAUSE		"HTTP/1.1 503 Service Unavailable OK\r\n"	// ��������ʱ���ܴ���������Ϊ���ڽ���ά������ʱ����

#define HTTP_SERVER		"Server: Hac web server.\r\n"
#define CRLF "\r\n"

class Http: public TCP
{
public:
	Http(const string & root,CriticalSection *pCS):m_root(root),m_pCS(pCS){/*initContentType();*/initBuff();memcpy(m_hex,"0123456789ABCDEF",16);}
	~Http(){if(m_buff.swin) delete [] m_buff.swin;}
	int ReceiveHead();
	int SendHead(const char * answer, unsigned int content_len=0, const char * type=NULL, const char * encoding=NULL);
	int SendFile(const char * filename=NULL);
	int SendContent(const char * msg);
	int IsKeepAlive(){return (m_request["Connection"]=="Keep-Alive" || (m_request["Connection"]!="Close" && m_request["Version"]=="HTTP/1.1"));}
private:
	void headLineParse(const char * line, int len);
	void lineParamParse(const string & line, map<string,string> & params);
	void uriParse(const string & uri, string & path, string & params);
	void setHttpDate(const time_t * ptime=NULL);
	int  cmpHttpDate(const char * http_date1, const char * http_date2);
	int  getFileExt(const char * filename, string & ext, string & content_type);
	int  getFileInfo(const char * filename);
	//int  getRequestLine(string & line);
	int  getRequestSplit(char * dest, int &dlen, const char * separator=CRLF);
	int  getPostParam();
	int  getBoundaryParam(int content_length, const string & boundary);
	void initBuff(){m_buff.start=m_buff.count=m_buff.newline = m_buff.read_count =m_buff.read_max= 0; m_buff.swin=NULL;}
	void new_swin(int len){if(m_buff.swin) delete [] m_buff.swin; m_buff.swin = new char[len]; m_buff.wlen = 0;}
	map<string,string> m_request;		// Http��������Ϣ
	map<string,string> m_get_params;
	map<string,string> m_post_params;
	map<string,string> m_post_files;
	char m_http_date[40];	// Http��ʽ��Date���ݣ�ͨ��setHttpDate(time)����
	char m_hex[16];
	struct file_info{
		unsigned long size;
		time_t time;
		char is_file, is_dir, can_read;
	} m_file_info;						// �����ļ�ȡ����http��ע����Ϣ
	struct tcp_buff{
		int start,count,newline,read_count,read_max,wlen;
		char buff[1025];
		char *swin;
	} m_buff;							// socket�Ļ�����Ϣ����Ҫ����Keep-Alive�У�����������Ϣ����socket���յ����ݿ��У����ڿ��������Ϣ��
	string m_root;						// �������ı��ظ�Ŀ¼
	string m_response;		// �������Ҫ���ؿͻ��˵���Ӧ��Ϣ
	CriticalSection *m_pCS;	// for debug ,ֻ������������ݵ���Ļ��������̳߳�ͻ������Ϣ���������Կ��ǹ����ڴ�ʹ������
public:
	static map<string,string> m_content_type;	// ͨ���ļ���չ���������ķ���������������
};
void http_initContentType();	// ��ʼ����̬����Http::m_content_type

#define LUA_HTTP_CONN "HTTP_CONNECTION"
#define LUA_GLOB_HTTP "_http_connect"
class HttpLua
{
public:
	HttpLua(Http *p);
	~HttpLua(){lua_close(m_L);}
	int DoScript(const char * script){	return luaL_dostring(m_L,script);	}
	int DoBuffer(const char * script, int len,const char * name){ return !(luaL_loadbuffer(m_L,script,len,name) || lua_pcall(m_L, 0, LUA_MULTRET, 0));}
	int DoFile(const char * filename){return luaL_dofile(m_L,filename);	}
	int DoHLUA(const char * filename);
	void SetTable(const char * varname, map<string,string> & value); 
	void RegisterCFunction(const char * fun_name, int(*cfun)(lua_State *));
private:
	void SendHtml(const char *html,int len);
	void ExecScript(const char *script,int len,int block_no);
	lua_State *m_L;
	Http *m_pHttp;
};

#endif
