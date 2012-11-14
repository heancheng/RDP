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
注意：
1.在connect被close之前，statement必须提前close，否则会出现游标关闭问题。这个问题需要进一步解决。
   已经解决，通过在statment增加conn引用计数解决问题。
2.prepare参数问题没有完成测试,已经测试
3.foxpro的odbc创建表问题没有成功。
   已经找到原因，创建表时，表会创建在执行文件所在目录，并没有按照ODBC配置所在目录创建。如果需要创建在
   驱动指定目录，需要人为指定表的目录，语法如：
   create table [E:\\cvs_qgs\\read_oa\\ttt.dbf] (a char(10),b char(10))
4.BindParameter已经测试，可以上传大数据。
5.date，datetime类型字段还没有测试, 已经解决了绑定日期、时间类型参数，
   下一步select取数结果对应也要转换成lua的表格式，与绑定参数格式对应。
6.完成日期，日期时间类型的绑定测试。
*/
