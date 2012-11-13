#include "http.h"
#include <time.h>
#include <sys/stat.h>
#include <cstdio>
#include <iostream>
CriticalSection g_cs_content_type_dict; //全局的content_type字典的临界区锁
map<string,string> Http::m_content_type; // 静态变量声明及初始化

static int lua_myprint(lua_State *L)
{
	int n = lua_gettop(L);
	int i;
	string result;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to "
                           LUA_QL("print"));
		if (i>1) result += " ";//fputs("\t", stdout);
		result += s; //fputs(s, stdout);
		lua_pop(L, 1);  /* pop result */
	}
	result += "\r\n"; // fputs("\n", stdout);
	lua_getglobal(L,LUA_GLOB_HTTP);	// 取全局变量到堆栈顶
	//Http *p = (Http *)luaL_checkudata(L, -1,  "HTTP_CONNECTION");
	Http **p = (Http **)lua_touserdata(L, -1);
	if(!*p)
		fputs(result.c_str(),stdout);
	else
	{
		char buff[80];
		sprintf(buff,"%X\r\n",result.length());
		(*p)->Send(buff,strlen(buff));
		(*p)->Send(result.c_str(),result.length());
		(*p)->Send(CRLF,2);
	}
	return 0;
}
int escape(char * dest, int dlen, const char * src, int slen)
{
	int i,j;
	int c;
	char *hex="0123456789ABCDEF";
	for(i=0,j=0;i<slen;i++)
	{
		c = src[i];
		if(c==' ') {if(j<dlen) dest[j]='+'; j++;}
		else if('A'<=c && c<='Z') {if(j<dlen) dest[j]=c; j++;}
		else if('a'<=c && c<='z') {if(j<dlen) dest[j]=c; j++;}
		else if('0'<=c && c<='9') {if(j<dlen) dest[j]=c; j++;}
		else if(c=='-' || c=='_' || c=='.' || c=='!' || c=='~' || c=='*' || c=='\'' || c=='(' || c==')' )
		{	if(j<dlen) dest[j]=c; j++;	}
		else
		{
			if(j<dlen)dest[j]='%'; 
			j++;
			if(j<dlen-1){	dest[j]=hex[(c>>4)&0x0f];	dest[j+1]=hex[c&0x0f]; } 
			j+=2;
		}
	}
	if(j<dlen) dest[j] = '\0';
	return j;
}
int unescape(char * dest, int dlen, const char * src, int slen)
{
	int i,j;
	int c,d;
	char *hex="0123456789ABCDEF";
	i=0;j=0;
	while(i<slen)
	{
		c = src[i++];
		if(c=='+') {if(j<dlen) dest[j]=' '; j++;}
		else if(c=='%' && i<slen-1)
		{
			c = src[i++];
			if(c>='a' && c<='z') c=c-'a'+10;
			else if(c>='A' && c<='Z') c=c-'A'+10;
			else c=c-'0';
			c <<=4;
			d = src[i++];
			if(d>='a' && d<='z') d=d-'a'+10;
			else if(d>='A' && d<='Z') d=d-'A'+10;
			else d=d-'0';
			c = c+d;
			if(j<dlen) dest[j]=c; j++;
		}
		else
		{	if(j<dlen) dest[j]=c; j++;	}

	}
	if(j<dlen) dest[j] = '\0';
	return j;
}
static int lua_escape(lua_State * L)
{
	size_t slen,dlen;
	const char * src = luaL_checklstring(L,1,&slen);
	char * dest ;
	dest = new char [slen*3+2];
	dlen = escape(dest,slen*3+1,src,slen);
	lua_pushlstring(L,dest,dlen);
	delete [] dest;
	return 1;
}
static int lua_unescape(lua_State * L)
{
	size_t slen,dlen;
	const char * src = luaL_checklstring(L,1,&slen);
	char * dest ;
	dest = new char [slen+1];
	dlen = unescape(dest,slen,src,slen);
	lua_pushlstring(L,dest,dlen);
	delete [] dest;
	return 1;
}

void http_initContentType()
{
	Http::m_content_type[".001"]="application/x-001";
 	Http::m_content_type[".301"]="application/x-301";
 	Http::m_content_type[".323"]="text/h323";
 	Http::m_content_type[".906"]="application/x-906";
 	Http::m_content_type[".907"]="drawing/907";
 	Http::m_content_type[".a11"]="application/x-a11";
 	Http::m_content_type[".acp"]="audio/x-mei-aac";
	Http::m_content_type[".ai"]="application/postscript";
	Http::m_content_type[".aif"]="audio/aiff";
	Http::m_content_type[".aifc"]="audio/aiff";
	Http::m_content_type[".aiff"]="audio/aiff";
	Http::m_content_type[".anv"]="application/x-anv";
	Http::m_content_type[".asa"]="text/asa";
	Http::m_content_type[".asf"]="video/x-ms-asf";
	Http::m_content_type[".asp"]="text/asp";
	Http::m_content_type[".asx"]="video/x-ms-asf";
	Http::m_content_type[".au"]="audio/basic";
	Http::m_content_type[".avi"]="video/avi";
	Http::m_content_type[".awf"]="application/vnd.adobe.workflow";
	Http::m_content_type[".biz"]="text/xml";
	Http::m_content_type[".bmp"]="application/x-bmp";
	Http::m_content_type[".bot"]="application/x-bot";
	Http::m_content_type[".c4t"]="application/x-c4t";
	Http::m_content_type[".c90"]="application/x-c90";
	Http::m_content_type[".cal"]="application/x-cals";
	Http::m_content_type[".cat"]="application/vnd.ms-pki.seccat";
	Http::m_content_type[".cdf"]="application/x-netcdf";
	Http::m_content_type[".cdr"]="application/x-cdr";
	Http::m_content_type[".cel"]="application/x-cel";
	Http::m_content_type[".cer"]="application/x-x509-ca-cert";
	Http::m_content_type[".cg4"]="application/x-g4";
	Http::m_content_type[".cgm"]="application/x-cgm";
	Http::m_content_type[".cit"]="application/x-cit";
	Http::m_content_type[".class"]="java/*";
	Http::m_content_type[".cml"]="text/xml";
	Http::m_content_type[".cmp"]="application/x-cmp";
	Http::m_content_type[".cmx"]="application/x-cmx";
	Http::m_content_type[".cot"]="application/x-cot";
	Http::m_content_type[".crl"]="application/pkix-crl";
	Http::m_content_type[".crt"]="application/x-x509-ca-cert";
	Http::m_content_type[".csi"]="application/x-csi";
	Http::m_content_type[".css"]="text/css";
	Http::m_content_type[".cut"]="application/x-cut";
	Http::m_content_type[".dbf"]="application/x-dbf";
	Http::m_content_type[".dbm"]="application/x-dbm";
	Http::m_content_type[".dbx"]="application/x-dbx";
	Http::m_content_type[".dcd"]="text/xml";
	Http::m_content_type[".dcx"]="application/x-dcx";
	Http::m_content_type[".der"]="application/x-x509-ca-cert";
	Http::m_content_type[".dgn"]="application/x-dgn";
	Http::m_content_type[".dib"]="application/x-dib";
	Http::m_content_type[".dll"]="application/x-msdownload";
	Http::m_content_type[".doc"]="application/msword";
	Http::m_content_type[".dot"]="application/msword";
	Http::m_content_type[".drw"]="application/x-drw";
	Http::m_content_type[".dtd"]="text/xml";
	Http::m_content_type[".dwf"]="Model/vnd.dwf";
	Http::m_content_type[".dwf"]="application/x-dwf";
	Http::m_content_type[".dwg"]="application/x-dwg";
	Http::m_content_type[".dxb"]="application/x-dxb";
	Http::m_content_type[".dxf"]="application/x-dxf";
	Http::m_content_type[".edn"]="application/vnd.adobe.edn";
	Http::m_content_type[".emf"]="application/x-emf";
	Http::m_content_type[".eml"]="message/rfc822";
	Http::m_content_type[".ent"]="text/xml";
	Http::m_content_type[".epi"]="application/x-epi";
	Http::m_content_type[".eps"]="application/x-ps";
	Http::m_content_type[".eps"]="application/postscript";
	Http::m_content_type[".etd"]="application/x-ebx";
	Http::m_content_type[".exe"]="application/x-msdownload";
	Http::m_content_type[".fax"]="image/fax";
	Http::m_content_type[".fdf"]="application/vnd.fdf";
	Http::m_content_type[".fif"]="application/fractals";
	Http::m_content_type[".fo"]="text/xml";
	Http::m_content_type[".frm"]="application/x-frm";
	Http::m_content_type[".g4"]="application/x-g4";
	Http::m_content_type[".gbr"]="application/x-gbr";
	Http::m_content_type["."]="application/x-";
	Http::m_content_type[".gif"]="image/gif";
	Http::m_content_type[".gl2"]="application/x-gl2";
	Http::m_content_type[".gp4"]="application/x-gp4";
	Http::m_content_type[".hgl"]="application/x-hgl";
	Http::m_content_type[".hmr"]="application/x-hmr";
	Http::m_content_type[".hpg"]="application/x-hpgl";
	Http::m_content_type[".hpl"]="application/x-hpl";
	Http::m_content_type[".hqx"]="application/mac-binhex40";
	Http::m_content_type[".hrf"]="application/x-hrf";
	Http::m_content_type[".hta"]="application/hta";
	Http::m_content_type[".htc"]="text/x-component";
	Http::m_content_type[".htm"]="text/html";
	Http::m_content_type[".html"]="text/html";
	Http::m_content_type[".htt"]="text/webviewhtml";
	Http::m_content_type[".htx"]="text/html";
	Http::m_content_type[".icb"]="application/x-icb";
	Http::m_content_type[".ico"]="image/x-icon";
	Http::m_content_type[".ico"]="application/x-ico";
	Http::m_content_type[".iff"]="application/x-iff";
	Http::m_content_type[".ig4"]="application/x-g4";
	Http::m_content_type[".igs"]="application/x-igs";
	Http::m_content_type[".iii"]="application/x-iphone";
	Http::m_content_type[".img"]="application/x-img";
	Http::m_content_type[".ins"]="application/x-internet-signup";
	Http::m_content_type[".isp"]="application/x-internet-signup";
	Http::m_content_type[".IVF"]="video/x-ivf";
	Http::m_content_type[".java"]="java/*";
	Http::m_content_type[".jfif"]="image/jpeg";
	Http::m_content_type[".jpe"]="image/jpeg";
	Http::m_content_type[".jpe"]="application/x-jpe";
	Http::m_content_type[".jpeg"]="image/jpeg";
	Http::m_content_type[".jpg"]="image/jpeg";
	Http::m_content_type[".jpg"]="application/x-jpg";
	Http::m_content_type[".js"]="application/x-javascript";
	Http::m_content_type[".jsp"]="text/html";
	Http::m_content_type[".la1"]="audio/x-liquid-file";
	Http::m_content_type[".lar"]="application/x-laplayer-reg";
	Http::m_content_type[".latex"]="application/x-latex";
	Http::m_content_type[".lavs"]="audio/x-liquid-secure";
	Http::m_content_type[".lbm"]="application/x-lbm";
	Http::m_content_type[".lmsff"]="audio/x-la-lms";
	Http::m_content_type[".ls"]="application/x-javascript";
	Http::m_content_type[".ltr"]="application/x-ltr";
	Http::m_content_type[".m1v"]="video/x-mpeg";
	Http::m_content_type[".m2v"]="video/x-mpeg";
	Http::m_content_type[".m3u"]="audio/mpegurl";
	Http::m_content_type[".m4e"]="video/mpeg4";
	Http::m_content_type[".mac"]="application/x-mac";
	Http::m_content_type[".man"]="application/x-troff-man";
	Http::m_content_type[".math"]="text/xml";
	Http::m_content_type[".mdb"]="application/msaccess";
	Http::m_content_type[".mdb"]="application/x-mdb";
	Http::m_content_type[".mfp"]="application/x-shockwave-flash";
	Http::m_content_type[".mht"]="message/rfc822";
	Http::m_content_type[".mhtml"]="message/rfc822";
	Http::m_content_type[".mi"]="application/x-mi";
	Http::m_content_type[".mid"]="audio/mid";
	Http::m_content_type[".midi"]="audio/mid";
	Http::m_content_type[".mil"]="application/x-mil";
	Http::m_content_type[".mml"]="text/xml";
	Http::m_content_type[".mnd"]="audio/x-musicnet-download";
	Http::m_content_type[".mns"]="audio/x-musicnet-stream";
	Http::m_content_type[".mocha"]="application/x-javascript";
	Http::m_content_type[".movie"]="video/x-sgi-movie";
	Http::m_content_type[".mp1"]="audio/mp1";
	Http::m_content_type[".mp2"]="audio/mp2";
	Http::m_content_type[".mp2v"]="video/mpeg";
	Http::m_content_type[".mp3"]="audio/mp3";
	Http::m_content_type[".mp4"]="video/mpeg4";
	Http::m_content_type[".mpa"]="video/x-mpg";
	Http::m_content_type[".mpd"]="application/vnd.ms-project";
	Http::m_content_type[".mpe"]="video/x-mpeg";
	Http::m_content_type[".mpeg"]="video/mpg";
	Http::m_content_type[".mpg"]="video/mpg";
	Http::m_content_type[".mpga"]="audio/rn-mpeg";
	Http::m_content_type[".mpp"]="application/vnd.ms-project";
	Http::m_content_type[".mps"]="video/x-mpeg";
	Http::m_content_type[".mpt"]="application/vnd.ms-project";
	Http::m_content_type[".mpv"]="video/mpg";
	Http::m_content_type[".mpv2"]="video/mpeg";
	Http::m_content_type[".mpw"]="application/vnd.ms-project";
	Http::m_content_type[".mpx"]="application/vnd.ms-project";
	Http::m_content_type[".mtx"]="text/xml";
	Http::m_content_type[".mxp"]="application/x-mmxp";
	Http::m_content_type[".net"]="image/pnetvue";
	Http::m_content_type[".nrf"]="application/x-nrf";
	Http::m_content_type[".nws"]="message/rfc822";
	Http::m_content_type[".odc"]="text/x-ms-odc";
	Http::m_content_type[".out"]="application/x-out";
	Http::m_content_type[".p10"]="application/pkcs10";
	Http::m_content_type[".p12"]="application/x-pkcs12";
	Http::m_content_type[".p7b"]="application/x-pkcs7-certificates";
	Http::m_content_type[".p7c"]="application/pkcs7-mime";
	Http::m_content_type[".p7m"]="application/pkcs7-mime";
	Http::m_content_type[".p7r"]="application/x-pkcs7-certreqresp";
	Http::m_content_type[".p7s"]="application/pkcs7-signature";
	Http::m_content_type[".pc5"]="application/x-pc5";
	Http::m_content_type[".pci"]="application/x-pci";
	Http::m_content_type[".pcl"]="application/x-pcl";
	Http::m_content_type[".pcx"]="application/x-pcx";
	Http::m_content_type[".pdf"]="application/pdf";
	Http::m_content_type[".pdf"]="application/pdf";
	Http::m_content_type[".pdx"]="application/vnd.adobe.pdx";
	Http::m_content_type[".pfx"]="application/x-pkcs12";
	Http::m_content_type[".pgl"]="application/x-pgl";
	Http::m_content_type[".pic"]="application/x-pic";
	Http::m_content_type[".pko"]="application/vnd.ms-pki.pko";
	Http::m_content_type[".pl"]="application/x-perl";
	Http::m_content_type[".plg"]="text/html";
	Http::m_content_type[".pls"]="audio/scpls";
	Http::m_content_type[".plt"]="application/x-plt";
	Http::m_content_type[".png"]="image/png";
	Http::m_content_type[".png"]="application/x-png";
	Http::m_content_type[".pot"]="application/vnd.ms-powerpoint";
	Http::m_content_type[".ppa"]="application/vnd.ms-powerpoint";
	Http::m_content_type[".ppm"]="application/x-ppm";
	Http::m_content_type[".pps"]="application/vnd.ms-powerpoint";
	Http::m_content_type[".ppt"]="application/vnd.ms-powerpoint";
	Http::m_content_type[".ppt"]="application/x-ppt";
	Http::m_content_type[".pr"]="application/x-pr";
	Http::m_content_type[".prf"]="application/pics-rules";
	Http::m_content_type[".prn"]="application/x-prn";
	Http::m_content_type[".prt"]="application/x-prt";
	Http::m_content_type[".ps"]="application/x-ps";
	Http::m_content_type[".ps"]="application/postscript";
	Http::m_content_type[".ptn"]="application/x-ptn";
	Http::m_content_type[".pwz"]="application/vnd.ms-powerpoint";
	Http::m_content_type[".r3t"]="text/vnd.rn-realtext3d";
	Http::m_content_type[".ra"]="audio/vnd.rn-realaudio";
	Http::m_content_type[".ram"]="audio/x-pn-realaudio";
	Http::m_content_type[".ras"]="application/x-ras";
	Http::m_content_type[".rat"]="application/rat-file";
	Http::m_content_type[".rdf"]="text/xml";
	Http::m_content_type[".rec"]="application/vnd.rn-recording";
	Http::m_content_type[".red"]="application/x-red";
	Http::m_content_type[".rgb"]="application/x-rgb";
	Http::m_content_type[".rjs"]="application/vnd.rn-realsystem-rjs";
	Http::m_content_type[".rjt"]="application/vnd.rn-realsystem-rjt";
	Http::m_content_type[".rlc"]="application/x-rlc";
	Http::m_content_type[".rle"]="application/x-rle";
	Http::m_content_type[".rm"]="application/vnd.rn-realmedia";
	Http::m_content_type[".rmf"]="application/vnd.adobe.rmf";
	Http::m_content_type[".rmi"]="audio/mid";
	Http::m_content_type[".rmj"]="application/vnd.rn-realsystem-rmj";
	Http::m_content_type[".rmm"]="audio/x-pn-realaudio";
	Http::m_content_type[".rmp"]="application/vnd.rn-rn_music_package";
	Http::m_content_type[".rms"]="application/vnd.rn-realmedia-secure";
	Http::m_content_type[".rmvb"]="application/vnd.rn-realmedia-vbr";
	Http::m_content_type[".rmx"]="application/vnd.rn-realsystem-rmx";
	Http::m_content_type[".rnx"]="application/vnd.rn-realplayer";
	Http::m_content_type[".rp"]="image/vnd.rn-realpix";
	Http::m_content_type[".rpm"]="audio/x-pn-realaudio-plugin";
	Http::m_content_type[".rsml"]="application/vnd.rn-rsml";
	Http::m_content_type[".rt"]="text/vnd.rn-realtext";
	Http::m_content_type[".rtf"]="application/msword";
	Http::m_content_type[".rtf"]="application/x-rtf";
	Http::m_content_type[".rv"]="video/vnd.rn-realvideo";
	Http::m_content_type[".sam"]="application/x-sam";
	Http::m_content_type[".sat"]="application/x-sat";
	Http::m_content_type[".sdp"]="application/sdp";
	Http::m_content_type[".sdw"]="application/x-sdw";
	Http::m_content_type[".sit"]="application/x-stuffit";
	Http::m_content_type[".slb"]="application/x-slb";
	Http::m_content_type[".sld"]="application/x-sld";
	Http::m_content_type[".slk"]="drawing/x-slk";
	Http::m_content_type[".smi"]="application/smil";
	Http::m_content_type[".smil"]="application/smil";
	Http::m_content_type[".smk"]="application/x-smk";
	Http::m_content_type[".snd"]="audio/basic";
	Http::m_content_type[".sol"]="text/plain";
	Http::m_content_type[".sor"]="text/plain";
	Http::m_content_type[".spc"]="application/x-pkcs7-certificates";
	Http::m_content_type[".spl"]="application/futuresplash";
	Http::m_content_type[".spp"]="text/xml";
	Http::m_content_type[".ssm"]="application/streamingmedia";
	Http::m_content_type[".sst"]="application/vnd.ms-pki.certstore";
	Http::m_content_type[".stl"]="application/vnd.ms-pki.stl";
	Http::m_content_type[".stm"]="text/html";
	Http::m_content_type[".sty"]="application/x-sty";
	Http::m_content_type[".svg"]="text/xml";
	Http::m_content_type[".swf"]="application/x-shockwave-flash";
	Http::m_content_type[".tdf"]="application/x-tdf";
	Http::m_content_type[".tg4"]="application/x-tg4";
	Http::m_content_type[".tga"]="application/x-tga";
	Http::m_content_type[".tif"]="image/tiff";
	Http::m_content_type[".tif"]="application/x-tif";
	Http::m_content_type[".tiff"]="image/tiff";
	Http::m_content_type[".tld"]="text/xml";
	Http::m_content_type[".top"]="drawing/x-top";
	Http::m_content_type[".torrent"]="application/x-bittorrent";
	Http::m_content_type[".tsd"]="text/xml";
	Http::m_content_type[".txt"]="text/plain";
	Http::m_content_type[".uin"]="application/x-icq";
	Http::m_content_type[".uls"]="text/iuls";
	Http::m_content_type[".vcf"]="text/x-vcard";
	Http::m_content_type[".vda"]="application/x-vda";
	Http::m_content_type[".vdx"]="application/vnd.visio";
	Http::m_content_type[".vml"]="text/xml";
	Http::m_content_type[".vpg"]="application/x-vpeg005";
	Http::m_content_type[".vsd"]="application/vnd.visio";
	Http::m_content_type[".vsd"]="application/x-vsd";
	Http::m_content_type[".vss"]="application/vnd.visio";
	Http::m_content_type[".vst"]="application/vnd.visio";
	Http::m_content_type[".vst"]="application/x-vst";
	Http::m_content_type[".vsw"]="application/vnd.visio";
	Http::m_content_type[".vsx"]="application/vnd.visio";
	Http::m_content_type[".vtx"]="application/vnd.visio";
	Http::m_content_type[".vxml"]="text/xml";
	Http::m_content_type[".wav"]="audio/wav";
	Http::m_content_type[".wax"]="audio/x-ms-wax";
	Http::m_content_type[".wb1"]="application/x-wb1";
	Http::m_content_type[".wb2"]="application/x-wb2";
	Http::m_content_type[".wb3"]="application/x-wb3";
	Http::m_content_type[".wbmp"]="image/vnd.wap.wbmp";
	Http::m_content_type[".wiz"]="application/msword";
	Http::m_content_type[".wk3"]="application/x-wk3";
	Http::m_content_type[".wk4"]="application/x-wk4";
	Http::m_content_type[".wkq"]="application/x-wkq";
	Http::m_content_type[".wks"]="application/x-wks";
	Http::m_content_type[".wm"]="video/x-ms-wm";
	Http::m_content_type[".wma"]="audio/x-ms-wma";
	Http::m_content_type[".wmd"]="application/x-ms-wmd";
	Http::m_content_type[".wmf"]="application/x-wmf";
	Http::m_content_type[".wml"]="text/vnd.wap.wml";
	Http::m_content_type[".wmv"]="video/x-ms-wmv";
	Http::m_content_type[".wmx"]="video/x-ms-wmx";
	Http::m_content_type[".wmz"]="application/x-ms-wmz";
	Http::m_content_type[".wp6"]="application/x-wp6";
	Http::m_content_type[".wpd"]="application/x-wpd";
	Http::m_content_type[".wpg"]="application/x-wpg";
	Http::m_content_type[".wpl"]="application/vnd.ms-wpl";
	Http::m_content_type[".wq1"]="application/x-wq1";
	Http::m_content_type[".wr1"]="application/x-wr1";
	Http::m_content_type[".wri"]="application/x-wri";
	Http::m_content_type[".wrk"]="application/x-wrk";
	Http::m_content_type[".ws"]="application/x-ws";
	Http::m_content_type[".ws2"]="application/x-ws";
	Http::m_content_type[".wsc"]="text/scriptlet";
	Http::m_content_type[".wsdl"]="text/xml";
	Http::m_content_type[".wvx"]="video/x-ms-wvx";
	Http::m_content_type[".xdp"]="application/vnd.adobe.xdp";
	Http::m_content_type[".xdr"]="text/xml";
	Http::m_content_type[".xfd"]="application/vnd.adobe.xfd";
	Http::m_content_type[".xfdf"]="application/vnd.adobe.xfdf";
	Http::m_content_type[".xhtml"]="text/html";
	Http::m_content_type[".xls"]="application/vnd.ms-excel";
	Http::m_content_type[".xls"]="application/x-xls";
	Http::m_content_type[".xlw"]="application/x-xlw";
	Http::m_content_type[".xml"]="text/xml";
	Http::m_content_type[".xpl"]="audio/scpls";
	Http::m_content_type[".xq"]="text/xml";
	Http::m_content_type[".xql"]="text/xml";
	Http::m_content_type[".xquery"]="text/xml";
	Http::m_content_type[".xsd"]="text/xml";
	Http::m_content_type[".xsl"]="text/xml";
	Http::m_content_type[".xslt"]="text/xml";
	Http::m_content_type[".xwd"]="application/x-xwd";
	Http::m_content_type[".x_b"]="application/x-x_b";
	Http::m_content_type[".x_t"]="application/x-x_t";
}

int http_find_content_type(const string & ext, string & content_type)
{
	int result = 0;
	g_cs_content_type_dict.Lock();
	map<string,string>::iterator ci;
	if(ext.length()>0){
		ci = Http::m_content_type.find(ext);
		if(ci!=Http::m_content_type.end()){
			content_type = ci->second;
			result = 1;
		}
	}
	g_cs_content_type_dict.Unlock();
	return result;
}

int Http::ReceiveHead()
{
	int ok,dlen;
	//string line;
	char line[4096];
	m_request.clear();
	ok = 0;
	dlen=4095;
	new_swin(3);
	while(getRequestSplit(line,dlen,CRLF)/*getRequestLine(line)*/)
	{
		if(dlen==0){ok = 1; break;}
		headLineParse(line,dlen);
		dlen=4095;
	}
	if(ok && m_request["Content-Length"].length()>0)
		ok = getPostParam();
	map<string,string>::iterator mi;
/*	this->m_pCS->Lock();
	cout << "___________ HTTP REQUEST ____________" << endl;
	for(mi=m_request.begin();mi!=m_request.end();mi++)
	{
		cout << ">" << mi->first << "= " << mi->second << endl;
	}
	cout << "___________ HTTP GET PARAMS ____________" << endl;
	for(mi=m_get_params.begin();mi!=m_get_params.end();mi++)
	{
		cout << ">" << mi->first << "= " << mi->second << endl;
	}
	cout << "___________ HTTP POST PARAMS ____________" << endl;
	for(mi=m_post_params.begin();mi!=m_post_params.end();mi++)
	{
		cout << ">" << mi->first << "= " << mi->second << endl;
	}
	this->m_pCS->Unlock();   -----------*/
	return ok;
}

int Http::getRequestSplit(char * dest, int &dlen, const char * separator)
{
	int di,plen;
	char c;
	plen = strlen(separator);
	di=0;

	while(!m_buff.read_max || (m_buff.read_count<m_buff.read_max))
	{
		if(m_buff.start>=m_buff.count){
			m_buff.count = Receive(m_buff.buff,1024);
			if(m_buff.count==0 || m_buff.count==SOCKET_ERROR) return 0;
			m_buff.start = 0;
			m_buff.buff[m_buff.count] = '\0';
		//	m_pCS->Lock();				// for debug
		//	cout << m_buff.buff;		// for debug
		//	m_pCS->Unlock();			// for debug
		}
		/*----- BS算法，存在缓冲满时的临界问题
		dest[di++] = m_buff.buff[m_buff.start++];  m_buff.read_count++;
		if(di>=plen){
			for(i=di-1,j=plen-1;j>=0 && dest[i]==separator[j];i--,j--) ;
			if(j<0){ dlen =di-plen; return 1;} // 匹配成功，返回1
		}
		if(di==dlen) return 2; // 结果缓冲满，返回
		-------*/
		c = m_buff.buff[m_buff.start++]; m_buff.read_count++;
		if(c==separator[m_buff.wlen]){
			m_buff.swin[m_buff.wlen++]=c; 
			if(m_buff.wlen==plen){dlen = di; m_buff.wlen=0; return 1;} // 判断是否匹配分隔符
		} 
		else if(m_buff.wlen>0){
			m_buff.swin[m_buff.wlen++] = c;
			while(m_buff.wlen>0){
				dest[di++] = m_buff.swin[0];
				m_buff.wlen--;
				memmove(m_buff.swin,&m_buff.swin[1],m_buff.wlen);
				if(memcmp(separator,m_buff.swin,m_buff.wlen)==0) break; //剩下的可以匹配
				if(di==dlen) return 2; // 缓冲满
			}
		}else{
			dest[di++] = c;
			if(di==dlen) return 2; // 缓冲满
		}
	}
	dlen = di;
	m_buff.wlen=0;
	return 1; // src 完成匹配
}
int Http::getPostParam()
{
	int i,len;
	int content_length = atoi(m_request["Content-Length"].c_str());
	string content_type = m_request["Content-Type"];
	string content;
	char line[4096];
	len = content_type.length();
	if(len>0){
		string mp_type,boundary;
		for(i=0;i<len && content_type[i]!=' ';i++) ;
		mp_type = content_type.substr(0,i);
		if(mp_type=="multipart/form-data;"){
			for( ;i<len && content_type[i]!= '=' ;i++) ;
			boundary = content_type.substr(i+1);
			return getBoundaryParam(content_length,string("--")+boundary);
		}
	}
	m_buff.read_count = 0;
	m_buff.read_max = content_length;
	new_swin(3);
	while(m_buff.read_count<m_buff.read_max)
	{
		len = 4095;
		if(!getRequestSplit(line,len)/*!getRequestLine(content)*/) return 0;
		line[len] = '\0';
		content += line;
	}
	lineParamParse(content,m_post_params);
	return 1;
}	
int Http::getBoundaryParam(int content_length, const string & boundary)
{
	int i,len,ret;
	char cc[2],line[4096];
	string name,filename,value,tmp;
	cc[1] = '\0';
	m_buff.read_count = 0;
	m_buff.read_max = content_length;
	len = 4095;
	new_swin(boundary.length()+10);
	if(!getRequestSplit(line,len,boundary.c_str())) return 0; // -----------------------------7db46030622
	while(m_buff.read_count<m_buff.read_max)
	{
		name = filename = value = "";
		len = 4095;
		if(!getRequestSplit(line,len,CRLF)) return 0;     // 去掉boundary后面的CRLF，或'--CRLF'
		len = 4095;
		if(!getRequestSplit(line,len,CRLF)) return 0;     // Content-Disposition: form-data; name="message"
		line[len] = '\0';
		tmp = line;
		i = tmp.find("name=\"");
		if(i>0){
			for(i+=6;i>0 && i<tmp.length() && tmp[i]!='\"';i++){
				cc[0] = tmp[i]; name += cc;
			}
		}
		i = tmp.find("filename=\"");
		if(i>0){
			for(i+=10;i>0 && i<tmp.length() && tmp[i]!='\"';i++){
				cc[0] = tmp[i]; filename += cc;
			}
			if(name.length()>0) m_post_files[name] = filename;
		}
		len = 4095;
		while(len!=0){
			len = 4095;
			if(!getRequestSplit(line,len,CRLF)) return 0;
		}
		while(m_buff.read_count<m_buff.read_max)
		{
			len = 4095;
			ret = getRequestSplit(line,len,(string(CRLF)+boundary).c_str());
			if(!ret) return 0;
			value.append(line,&line[len]);
			if(ret==1) break;
		}
		if(name.length()>0) m_post_params[name] = value;
	}
	return 1;
}
void Http::lineParamParse(const string & line, map<string,string> & params)
{
	int s,i,j,len;
	string pair,name,value;
	i=0;
	len=line.length();
	while(i<len)
	{
		for(s=i;i<len && line[i]!='&';i++) ;
		pair = line.substr(s,i-s);
		i++;
		for(j=0;j<pair.length();j++){
			if(pair[j]=='='){
				name = pair.substr(0,j); value = pair.substr(j+1);
				if(name.length()>0) params[name] = value;
				break;
			}
		}
	}
}
void Http::headLineParse(const char * line, int len)
{
	int k;
	string key,value;
	for(k=0;k<len && line[k]!=' ';k++) ;
	if(k>len-1) return ;// 有错误
	if(k==0) return ;// 空格开头的行，不是合法的request命令行
	if(line[k-1]==':')
	{
		key.append(line,&line[k-1]);//=line.substr(0,k-1);value=line.substr(k+1);
		value.append(&line[k+1],&line[len]);
		m_request[key]=value;
	}
	else
	{
		key.append(line,&line[k]);//key=line.substr(0,k); value=line.substr(k+1);
		value.append(&line[k+1],&line[len]);
		if(key=="GET" || key=="POST" || key=="HEAD")
		{
			m_request["Method"]=key;
			len = value.length();
			for(k=0;k<len && value[k]!=' ';k++) ;
			if(k==0 || k>len-1) return ;
			m_request["Uri"]=value.substr(0,k);
			m_request["Version"] = value.substr(k+1);
			uriParse(m_request["Uri"],m_request["Path"],m_request["Get-Params"]);
			if(m_request["Get-Params"].length()) lineParamParse(m_request["Get-Params"],m_get_params);
		}
	}
}
int Http::SendHead(const char * answer, unsigned int len, const char * type, const char * encoding)
{
	char buff[100];
	Send(answer,strlen(answer));
	setHttpDate();
	sprintf(buff,"Date: %s\r\n",m_http_date);
	Send(buff,strlen(buff));
	if(type && strlen(type)>0)  {
		sprintf(buff,"Content-Type: %s\r\n",type);
		Send(buff,strlen(buff));
	}
	if(encoding && strlen(encoding)>0)  {
		sprintf(buff,"Content-Encoding: %s\r\n",encoding);
		Send(buff,strlen(buff));
	}
	if(len>0 || IsKeepAlive())  {
		sprintf(buff,"Content-Length: %d\r\n",len);
		Send(buff,strlen(buff));
	}
	if(m_response.length()>0)
		Send(m_response.c_str(),m_response.length());
	Send(HTTP_SERVER,strlen(HTTP_SERVER));
	Send(CRLF,2);
	m_response = "";
	return 1;
}
void Http::uriParse(const string & uri, string & path, string & params)
{
	int i,len;
	len = uri.length();
	if(len==0) return ;
	for(i=0;i<len && uri[i]!='?'; i++) ;
	if(i==len){path = uri; params="";}
	else{path = uri.substr(0,i); params=uri.substr(i+1);}
}
int Http::SendFile(const char * filename)
{
	string ext,content_type,fname;
	char buff[1024];

	if(filename==NULL)
		fname = m_root+m_request["Path"];
	else if(filename[0]=='/')
		fname = m_root+filename;
	else
		fname = m_root+"/"+filename;
	if(fname[fname.length()-1]=='/')
		fname += "index.html";
	for(int i=0;i<fname.length();i++){
		if(fname[i]=='/') fname[i] = '\\';
	}
	;
	if(getFileInfo(fname.c_str()) && m_file_info.is_dir)
		fname += "\\index.html";
	if(!getFileInfo(fname.c_str()))
	{
		SendHead(HTTP_NOT_FOUND);
		//SendHead(HTTP_OK,m_root.length(),"text/html"); // 用于测试，返回Root信息
		//Send(m_root.c_str(),m_root.length());
		return 1;
	}
	if(m_file_info.is_file)
	{
		getFileExt(fname.c_str(),ext,content_type);
		if(!m_file_info.can_read){
			SendHead(HTTP_FORBIDDEN);
			return 1;
		}
		if(ext==".hlua")
		{
			m_response = "Transfer-Encoding: chunked\r\n";
			SendHead(HTTP_OK,0,"text/html");
			// ---- 执行.hlua网页
			HttpLua lua(this);
			lua.SetTable("http_request",m_request);
			lua.SetTable("http_get_params",m_get_params);
			lua.SetTable("http_post_params",m_post_params);
			lua.SetTable("http_post_files",m_post_files);
			lua.DoHLUA(fname.c_str());
			Send("0\r\n",3); // 结束chunked
			Send(CRLF,2);	// 结束chunked,footer
			return 1;
		}else if(ext==".lua"){
			m_response = "Transfer-Encoding: chunked\r\n";
			SendHead(HTTP_OK,0,"text/html");
			// ---- 执行.hlua网页
			HttpLua lua(this);
			lua.SetTable("http_request",m_request);
			lua.SetTable("http_get_params",m_get_params);
			lua.SetTable("http_post_params",m_post_params);
			lua.SetTable("http_post_files",m_post_files);
			lua.DoFile(fname.c_str());
			Send("0\r\n",3); // 结束chunked
			Send(CRLF,2);	// 结束chunked,footer
			return 1;
		}
		setHttpDate(&m_file_info.time);
		m_response = "Last-Modified: ";
		m_response += m_http_date;
		m_response += CRLF;
		if(!m_request["If-Modified-Since"].empty() && cmpHttpDate(m_http_date,m_request["If-Modified-Since"].c_str())<=0)
		{	// 文件没有更新，不必重新传文件
			m_response +="If-Modified-Since: ";
			m_response += m_request["If-Modified-Since"];
			m_response += CRLF;
			SendHead(HTTP_NOT_MODIFIED,0);
			return 1;
		}
		else
		{
			SendHead(HTTP_OK,m_file_info.size,content_type.c_str());
			FILE * fp;
			int n;
			fp = fopen(fname.c_str(),"rb");
			while((n=fread(buff,1,1024,fp))>0)
				Send(buff,n);
			fclose(fp);
			return 1;
		}
	}
	else
	{
		SendHead(HTTP_NOT_FOUND);
		return 1;
	}
}

int Http::getFileExt(const char * filename, string & ext, string & content_type)
{
	int i;
	int len = strlen(filename);
	for(i=len-1;i>1 && filename[i]!='\\' && filename[i]!='/' && filename[i]!='.';i--) ;
	if(filename[i]=='.'){
		ext = &filename[i];
		if(!http_find_content_type(ext,content_type))
				content_type = "application/download";
		return 1;
	}
	else 
		return 0;
}
int Http::getFileInfo(const char * filename)
{
	struct stat finfo; 
	if(::stat(filename, &finfo)!=0)
		return 0;
	m_file_info.time = finfo.st_mtime;
	m_file_info.size = finfo.st_size;
	m_file_info.is_file = (finfo.st_mode & S_IFREG ? 1 :0);
	m_file_info.is_dir = (finfo.st_mode & S_IFDIR ? 1: 0);
	m_file_info.can_read = (finfo.st_mode & S_IREAD ? 1: 0);
	return 1;
}
void Http::setHttpDate(const time_t * ptime)
{
	time_t t;
	struct tm *gmt;
	char * wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	char * month[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	if(ptime)
		t = *ptime;
	else
		time(&t);
	gmt = gmtime(&t);
	sprintf(m_http_date,"%s, %.2d %s %d %.2d:%.2d:%.2d GMT", wday[gmt->tm_wday],gmt->tm_mday,month[gmt->tm_mon],1900+gmt->tm_year, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
}
int Http::cmpHttpDate(const char * http_date1, const char * http_date2)
{
	char * month[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	int  date1[6],date2[6]; // 0-5 对应year，month，day，hour，min，sec
	int i;
	char s_month[10];
	sscanf(http_date1,"%*s%d%s%d%d:%d:%d",&date1[2],&s_month,&date1[0],&date1[3],&date1[4],&date1[5]);
	for(i=0;i<12;i++){
		if(strcmp(month[i],s_month)==0){
			date1[1] = i;
			break;
		}
	}
	sscanf(http_date2,"%*s%d%s%d%d:%d:%d",&date2[2],&s_month,&date2[0],&date2[3],&date2[4],&date2[5]);
	for(i=0;i<12;i++){
		if(strcmp(month[i],s_month)==0){
			date2[1] = i;
			break;
		}
	}
	for(i=0;i<6;i++){
		if(date1[i]!=date2[i]) return date1[i]-date2[i];
	}
	return 0;
}
//-------------  HttpLua ----------------------
void HttpLua::RegisterCFunction(const char * fun_name, int(*cfun)(lua_State *))
{
	lua_pushstring(m_L,fun_name);
	lua_pushcfunction(m_L,cfun);
	lua_settable(m_L,LUA_GLOBALSINDEX);
}
HttpLua::HttpLua(Http *p):m_pHttp(p)
{
		m_L = lua_open(); if(!m_L) throw "加载LUA环境失败。";
		luaL_openlibs(m_L);
		//lua_pushstring(m_L,"print");			// 在Lua中的函数名，替换原系统的print函数
		//lua_pushcfunction(m_L,lua_myprint);		// C函数入堆栈
		//lua_settable(m_L, LUA_GLOBALSINDEX);	// 注册为全局函数
		RegisterCFunction("print",lua_myprint);
		RegisterCFunction("escape",lua_escape);
		RegisterCFunction("unescape",lua_unescape);

		luaL_newmetatable(m_L,LUA_HTTP_CONN);	// 定义一个新的用户自定义类型
		Http ** pHttp = (Http **)lua_newuserdata(m_L,sizeof(Http *));
		*pHttp = p;
		luaL_getmetatable(m_L,  LUA_HTTP_CONN);
		lua_setmetatable(m_L, -2);					// 设置类型为用户自定义的"HTTP_CONNECTION"
		lua_setglobal(m_L,LUA_GLOB_HTTP);		// 定义为全局变量
		// 测试全局变量
		lua_getglobal(m_L,LUA_GLOB_HTTP);	// 取全局变量到堆栈顶
		Http **q = (Http **)lua_touserdata(m_L, -1);	
		string def_fun = "function httplua_check_run(script,blocknum)\r\n";
		def_fun += "	local fun,msg = loadstring(script,'block'..blocknum)\r\n";
		def_fun += "	if fun then\r\n";
		def_fun += "		result,msg = pcall(fun)\r\n";
		def_fun += "		if not result then\r\n";
		def_fun += "			print('<br>执行时错误:',msg)\r\n";
		def_fun += "		end\r\n";
		def_fun += "	else\r\n";
		def_fun += "		print('<br>编译时错误:',msg)\r\n";
		def_fun += "	end\r\n";
		def_fun += "end\r\n";
		luaL_dostring(m_L,def_fun.c_str());
}

int HttpLua::DoHLUA(const char * filename)
{
	int stat,i,hi,n,block_no;
	char buff[1024],html[1024],c,last,cc[2];
	string script;
	FILE * fp = fopen(filename,"rt");
	if(!fp){
		DoScript("print('<html><body>Open file fail.</body></html>')");
		return 0;
	}
	stat = 0;
	last = '\0';
	hi = 0;
	cc[1] = '\0';
	block_no = 0;
	while(!feof(fp))
	{
		n = fread(buff,1,1024,fp);
		i = 0;
		while(i<n){
			c = buff[i++];
			if(last=='<' && c=='%' && !stat){ 
				stat=1;c=buff[i++];
				SendHtml(html,hi);
				hi = 0;
			}else if(last=='%' && c=='>' && stat){
				stat = 0; c=buff[i++];
				ExecScript(script.c_str(),script.length(),++block_no);
				script = "";
			}else{
				if(stat){
					cc[0] = last;
					script += cc;
				}else{
					if(last) html[hi++] = last;
				}
			}
			last = c;
		}
		if(hi)	{
			SendHtml(html,hi);  
			hi=0;
		}
	}
	if(stat){
		cc[0] = last;
		script += cc;
	}
	else if(last!=' ' && last!='\t' && last!='\0' && last!='\r' && last!='\n')
	{
		html[hi++] = last;
	}
	if(hi){
		SendHtml(html,hi); 
		hi=0;
	}
	if(script.length()) ExecScript(script.c_str(),script.length(),++block_no);
	fclose(fp);
	return 1;
}
void HttpLua::SendHtml(const char *html,int len)
{
	char chunk_head[20];
	if(len){
		sprintf(chunk_head,"%X\r\n",len);
		m_pHttp->Send(chunk_head,strlen(chunk_head));
		m_pHttp->Send(html,len);
		m_pHttp->Send(CRLF,2);
	}
}
void HttpLua::ExecScript(const char *script,int len,int block_no)
{
	if(!len)
		return ;
	lua_getglobal(m_L,"httplua_check_run");
	lua_pushstring(m_L,script);
	lua_pushinteger(m_L,block_no);
	lua_pcall(m_L,2,0,0);
	//DoScript(script);
}
void HttpLua::SetTable(const char * varname, map<string,string> & value)
{
	lua_newtable(m_L);
	map<string,string>::iterator mi;
	for(mi=value.begin(); mi!=value.end(); mi++)
	{
		lua_pushlstring(m_L,mi->first.c_str(),mi->first.length());
		lua_pushlstring(m_L,mi->second.c_str(),mi->second.length());
		lua_rawset(m_L,-3);
	}
	lua_setglobal(m_L,varname);
}