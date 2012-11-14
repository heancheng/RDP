#include "lua_odbc.h"

int  odbc_statement_create(lua_State *L, connection_t *conn, const char *sql_query);
int odbc_datetime_new(lua_State *L);
int odbc_date_new(lua_State *L);

//get_error(SQL_HANDLE_DBC,conn->dbc,msg,msgLen);

static int commit(connection_t *conn) {
    int rc; //= OCITransCommit(conn->svc, conn->err, OCI_DEFAULT);
	SQLRETURN retcode;
	retcode = SQLEndTran(SQL_HANDLE_DBC,conn->dbc,SQL_COMMIT);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		SQLUSMALLINT cb;
		SQLSMALLINT cbsize;
		SQLGetInfo(conn->dbc,SQL_CURSOR_COMMIT_BEHAVIOR,
			&cb,sizeof(cb),	&cbsize);
		if(cb != SQL_CB_PRESERVE) // Cursor Behavior
		{
			//int s = m_arrayCursor.GetSize();
			//CMyCursor *dbCursor;
			//for(int i=s-1;i>=0;i--)
			//{
			//	dbCursor = (CMyCursor *)m_arrayCursor[i];
			//	m_arrayCursor.RemoveAt(i);
			//	dbCursor->Close();
			//}
		}
		SQLSetConnectAttr(conn->dbc,
			SQL_ATTR_AUTOCOMMIT,
			(SQLUSMALLINT *)SQL_AUTOCOMMIT_ON,
			SQL_IS_UINTEGER);
		rc =  TRUE;
	}
	else
		rc = FALSE;
    return rc;
}

static int rollback(connection_t *conn) {
    int rc; //= OCITransRollback(conn->svc, conn->err, OCI_DEFAULT);
	SQLRETURN retcode;
	retcode = SQLEndTran(SQL_HANDLE_DBC,conn->dbc,SQL_ROLLBACK);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		SQLUSMALLINT cb;
		SQLSMALLINT cbsize;
		SQLGetInfo(conn->dbc,SQL_CURSOR_ROLLBACK_BEHAVIOR,
			&cb,sizeof(cb),	&cbsize);
		if(cb != SQL_CB_PRESERVE)   // Cursor Behavior
		{
		//	int s = m_arrayCursor.GetSize();
		//	CMyCursor *dbCursor;
		//	for(int i=s-1;i>=0;i--)
		//	{
		//		dbCursor = (CMyCursor *)m_arrayCursor[i];
		//		m_arrayCursor.RemoveAt(i);
		//		dbCursor->Close();
		//	}
		}
		SQLSetConnectAttr(conn->dbc,
			SQL_ATTR_AUTOCOMMIT,
			(SQLUSMALLINT *)SQL_AUTOCOMMIT_ON,
			SQL_IS_UINTEGER);
		rc = TRUE;
	}
	else
		rc = FALSE;

    return rc;
}


/*
 * connection,err = DBD.ODBC.New(dsn)
 */
int connection_new(lua_State *L) {
    int n = lua_gettop(L);

    int rc = 0;

    const char *dsn = NULL;
    const char *db = NULL;
    connection_t *conn = NULL;

    SQLHANDLE env = NULL;
    SQLHANDLE dbc = NULL;
    SQLRETURN retcode = 0;
	SQLCHAR conStrOut[1024+1];
	SQLSMALLINT conStrOutBytes,timeOut;
	SQLUSMALLINT driverCompletion ;
	char msg[1025];
    /* db, user, password */
    switch(n) {
		case 5:
		case 4:
		case 3:
		case 2:
			//if (lua_isnil(L, 2) == 0)
			//	dsn = luaL_checkstring(L, 2);
		case 1:
			/*
			 * db is the only mandatory parameter
			 */
			//db = luaL_checkstring(L, 1);
			dsn = luaL_checkstring(L, 1);
    }

	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
	{
		lua_pushnil(L);
		lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, get_error(SQL_HANDLE_DBC,conn->dbc,msg,1025));

		return 2;
	}
	/* Set the ODBC version environment attribute */
	retcode = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, SQL_IS_INTEGER);
	if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) )
	{
		lua_pushnil(L);
		lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, get_error(SQL_HANDLE_DBC,conn->dbc,msg,1025));
		SQLFreeHandle(SQL_HANDLE_ENV, env);

		return 2;
	}
	/* Allocate connection handle */
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
	if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) )
	{
		lua_pushnil(L);
		lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, get_error(SQL_HANDLE_ENV,env,msg,1025));
		SQLFreeHandle(SQL_HANDLE_ENV, env);

		return 2;
	}
	/* Set login timeout to 5 seconds. */
	timeOut = 5;
	SQLSetConnectAttr(dbc, SQL_LOGIN_TIMEOUT, (void *)&timeOut, SQL_IS_INTEGER);
	/* Connect to data source */
	//if(bPrompt)
	//	driverCompletion = SQL_DRIVER_COMPLETE || SQL_DRIVER_COMPLETE_REQUIRED || SQL_DRIVER_PROMPT;
	//else
	driverCompletion = SQL_DRIVER_NOPROMPT;

	retcode =  SQLDriverConnect(dbc, NULL  /* AfxGetMainWnd()->GetSafeHwnd()*/,
		(SQLCHAR*)dsn, strlen(dsn),conStrOut, 1024,	&conStrOutBytes,driverCompletion);
	if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
	{
		lua_pushnil(L);
		lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, get_error(SQL_HANDLE_DBC,dbc,msg,1025));
		 SQLFreeHandle(SQL_HANDLE_DBC, dbc);
		 SQLFreeHandle(SQL_HANDLE_ENV, env);

		return 2;
	}

    conn = (connection_t *)lua_newuserdata(L, sizeof(connection_t));
    conn->odbc = env;
	conn->dbc = dbc;
	conn->stmt_counter = 0;
    conn->err = FALSE;
    conn->autocommit = 0;
	conn->conn_string = malloc(strlen(conStrOut)+1);
	strcpy(conn->conn_string, conStrOut);
    //luaL_getmetatable(L,  ODBC_CONNECTION);
    //lua_setmetatable(L, -2);
    luaL_setmetatable(L, ODBC_CONNECTION);
    return 1;
}

static int connection_string(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1,  ODBC_CONNECTION);
	if(conn->dbc)
	{
		lua_pushstring(L,conn->conn_string);
	}
	else
		lua_pushstring(L,"没有连接数据库驱动");

	return 1;
}
/*
 * success = connection:autocommit(on)
 */
static int connection_autocommit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1,  ODBC_CONNECTION);
    int on = lua_toboolean(L, 2);
    int err = 1;
	SQLRETURN retcode;
	char msg[1025];
    if (conn->odbc) {
		if (on)
		    rollback(conn);
		if (on) {
			retcode = SQLSetConnectAttr(conn->dbc, SQL_ATTR_AUTOCOMMIT,
					(SQLPOINTER) SQL_AUTOCOMMIT_ON, 0);
		} else {
			retcode = SQLSetConnectAttr(conn->dbc, SQL_ATTR_AUTOCOMMIT,
					(SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0);
		}
		if(retcode!=SQL_SUCCESS){
			err = 1;
		}else{
			conn->autocommit = on;
			err = 0;
		}
    }

    lua_pushboolean(L, !err);
	if(conn->odbc && err)
		lua_pushstring(L,get_error(SQL_HANDLE_DBC,conn->dbc,msg,1024));
    return 1;
}


/*
 * success = connection:close()
 */
static int connection_close(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1,  ODBC_CONNECTION);
    int disconnect = 0;

    if (conn->odbc!=SQL_NULL_HENV) {
		if(conn->stmt_counter >0 )
				return luaL_error (L, "there are open cursors");  /*通过这个中断gc执行，等stmt完成后执行，保证内存安全*/
		 SQLDisconnect(conn->dbc);
		 SQLFreeHandle(SQL_HANDLE_DBC,conn->dbc);
		 SQLFreeHandle(SQL_HANDLE_ENV,conn->odbc);
		conn->dbc = SQL_NULL_HDBC;
		conn->odbc = SQL_NULL_HENV;
		free(conn->conn_string);
		conn->conn_string = NULL;
		disconnect = 1;
    }
	//printf("Close odbc connection.");
    lua_pushboolean(L, disconnect);
    return 1;
}

/*
 * success = connection:commit()
 */
static int connection_commit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1,  ODBC_CONNECTION);
    int err = 1;
	char msg[1025];
    if (conn->odbc) {
		err = commit(conn);
    }

    lua_pushboolean(L, !err);
	if(conn->odbc && err)
		lua_pushstring(L,get_error(SQL_HANDLE_DBC,conn->dbc,msg,1024));
	return 1;
}

/*
 * ok = connection:ping()
 */
static int connection_ping(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1,  ODBC_CONNECTION);
    int ok = 0;

    if (conn->odbc) {
		ok = 1;
    }

    lua_pushboolean(L, ok);
    return 1;
}

/*
 * statement,err = connection:prepare(sql_str)
 */
int connection_prepare(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1,  ODBC_CONNECTION);

    if (conn->odbc) {
		return  odbc_statement_create(L, conn, luaL_checkstring(L, 2));
    }

    lua_pushnil(L);
    lua_pushstring(L, DBI_ERR_DB_UNAVAILABLE);
    return 2;
}

/*
 * quoted = connection:quote(str)
 */
static int connection_quote(lua_State *L) {
    luaL_error(L, DBI_ERR_NOT_IMPLEMENTED,  ODBC_CONNECTION, "quote");
    return 0;
}

/*
 * success = connection:rollback()
 */
static int connection_rollback(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1,  ODBC_CONNECTION);
    int err = 1;
	char msg[1025];
    if (conn->odbc) {
			err = rollback(conn);
    }

    lua_pushboolean(L, !err);
	if(conn->odbc && err)
		lua_pushstring(L,get_error(SQL_HANDLE_DBC,conn->dbc,msg,1024));
    return 1;
}

/*
 * __gc
 */
static int connection_gc(lua_State *L) {
    /* always close the connection */
    connection_close(L);
    return 0;
}

/*
 * __tostring
 */
static int connection_tostring(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1,  ODBC_CONNECTION);

    lua_pushfstring(L, "%s: %p",  ODBC_CONNECTION, conn);

    return 1;
}

int  odbc_connection(lua_State *L) {
    /*
     * instance methods
     */
    static const luaL_Reg connection_methods[] = {
		{"autocommit", connection_autocommit},
		{"close", connection_close},
		{"commit", connection_commit},
		{"ping", connection_ping},
		{"prepare", connection_prepare},
		{"quote", connection_quote},
		{"rollback", connection_rollback},
		{"connection_string",connection_string},
		{NULL, NULL}
    };

    /*
     * class methods
     */
    static const luaL_Reg connection_new_methods[] = {
		{"new", connection_new},
		{"new_connection", connection_new},
		{"new_date",odbc_date_new},
		{"new_datetime",odbc_datetime_new},
		{NULL, NULL}
	};

    /*
    luaL_newmetatable(L,  ODBC_CONNECTION);
    //luaL_register(L, 0, connection_methods);
    luaL_newlib(L,connection_methods); // 在lua 5.2中，使用此函数代替luaL_register函数，并强制不能使用全局变量

    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, connection_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, connection_tostring);
    lua_setfield(L, -2, "__tostring");
    */
    createmeta(L,ODBC_CONNECTION,connection_methods,connection_gc, connection_tostring);

    //luaL_register(L,  ODBC_CONNECTION, connection_class_methods);
    luaL_newlib(L,connection_new_methods);

    return 1;
}
