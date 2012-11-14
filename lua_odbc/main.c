#include "lua_odbc.h"

int odbc_connection(lua_State *L);
int odbc_statement(lua_State *L);
int odbc_date(lua_State *L);
int odbc_datetime(lua_State *L);
/*
 * library entry point
 */
LUA_EXPORT int luaopen_luaodbc(lua_State *L) {
    odbc_connection(L);
    odbc_statement(L);
	odbc_date(L);
	odbc_datetime(L);
    return 1;
}

/*--------------
ע�⣺
1.��connect��close֮ǰ��statement������ǰclose�����������α�ر����⡣���������Ҫ��һ�������
   �Ѿ������ͨ����statment����conn���ü���������⡣
2.prepare��������û����ɲ���,�Ѿ�����
3.foxpro��odbc����������û�гɹ���
   �Ѿ��ҵ�ԭ�򣬴�����ʱ����ᴴ����ִ���ļ�����Ŀ¼����û�а���ODBC��������Ŀ¼�����������Ҫ������
   ����ָ��Ŀ¼����Ҫ��Ϊָ�����Ŀ¼���﷨�磺
   create table [E:\\cvs_qgs\\read_oa\\ttt.dbf] (a char(10),b char(10))
4.BindParameter�Ѿ����ԣ������ϴ������ݡ�
5.date��datetime�����ֶλ�û�в���, �Ѿ�����˰����ڡ�ʱ�����Ͳ�����
   ��һ��selectȡ�������ӦҲҪת����lua�ı��ʽ����󶨲�����ʽ��Ӧ��
6.������ڣ�����ʱ�����͵İ󶨲��ԡ�
*/
