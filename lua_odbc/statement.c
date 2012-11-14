#include "lua_odbc.h"
#include <time.h>

const char * GetErrorMessage(statement_t *statement)
{
	return get_error(SQL_HANDLE_STMT,statement->stmt,statement->msg,SQL_MAX_MESSAGE_LENGTH);
}
const char * get_error(short type,HANDLE h,char * msg, short msgLen)
{
//	SQLSMALLINT i;
	SQLCHAR  sqlState[6];
	SQLINTEGER nativeError;
	SQLGetDiagRec(type, h, 1, sqlState, &nativeError, msg, SQL_MAX_MESSAGE_LENGTH, &msgLen);
	if(msgLen>=SQL_MAX_MESSAGE_LENGTH)
		msgLen = SQL_MAX_MESSAGE_LENGTH-1;
	msg[msgLen] = '\0';
	return msg;
}
/*
 * Converts SQLite types to Lua types
 */
static lua_push_type_t odbc_to_lua_push(short odbc_type, int null) {
    lua_push_type_t lua_type;

    if (null)
		return LUA_PUSH_NIL;

    switch(odbc_type) {
		case SQL_NUMERIC:
		case SQL_DECIMAL:
		case SQL_REAL:
		case SQL_FLOAT:
		case SQL_DOUBLE:
		case SQL_BIGINT:
			lua_type = LUA_PUSH_NUMBER;
			break;
		case SQL_INTEGER:
		case SQL_SMALLINT:
		case SQL_TINYINT:
			lua_type = LUA_PUSH_INTEGER;
			break;
		case SQL_BIT:  // 我增加的
			lua_type = LUA_PUSH_BOOLEAN;
			break;
//#define SQL_LONGVARCHAR                         (-1)
//#define SQL_BINARY                              (-2)
//#define SQL_VARBINARY                           (-3)
//#define SQL_LONGVARBINARY                       (-4)
		case SQL_LONGVARCHAR:
		case SQL_BINARY:
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY: // 这个也是我增加的
			lua_type = LUA_PUSH_BINARY;
			break;
		case SQL_DATE:
		case SQL_TYPE_DATE:
			lua_type = LUA_PUSH_DATE;
			break;
		case SQL_TIMESTAMP:
		case SQL_TYPE_TIMESTAMP:
			lua_type = LUA_PUSH_DATETIME;
			break;
		default:
			lua_type = LUA_PUSH_STRING;
    }

    return lua_type;
}
static const char * odbc_type_str(short odbc_type) {
    switch(odbc_type) {
		case SQL_NUMERIC:
			return "NUMERIC";
		case SQL_DECIMAL:
			return "DECIMAL";
		case SQL_REAL:
			return "REAL";
		case SQL_FLOAT:
			return "FLOAT";
		case SQL_DOUBLE:
			return "DOUBLE";
		case SQL_BIGINT:
			return "BIGINT";
		case SQL_INTEGER:
			return "INTEGER";
		case SQL_SMALLINT:
			return "SMALLINT";
		case SQL_TINYINT:
			return "TINYINT";
		case SQL_DATE:
		case SQL_TYPE_DATE:
			return "DATE";
		case SQL_TIMESTAMP:
		case SQL_TYPE_TIMESTAMP:
			return "TIMESTAMP";
		case SQL_BIT:  // 我增加的
			return "BOOL";
		case SQL_LONGVARCHAR:
		case SQL_BINARY:
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY: // 这个也是我增加的
			return "BINARY";
		default:
			return "VARCHAR";
    }
}

int lua_date_param( lua_State *L,int index,SQL_DATE_STRUCT *odbc_date )
{
	int top = lua_gettop(L);
	int ok = 1;
	int year,month,day;
	int days[13]={0,31,29,31,30,31,30,31,31,30,31,30,31};
	if( ! lua_istable(L,index))
		return 0;
	lua_pushstring(L,"year");
	lua_rawget(L,index);
	if(!lua_isnumber(L,-1))
		ok = 0;
	else
		year = luaL_checkint(L,-1);
	if(ok)
	{
		lua_pushstring(L,"month");
		lua_rawget(L,index);
		if(!lua_isnumber(L,-1))
			ok = 0;
		else
		{
			month = luaL_checkint(L,-1);
			if(month<1 || month >12)
				ok = 0;
		}
	}
	if(ok)
	{
		day = 0;
		lua_pushstring(L,"day");
		lua_rawget(L,index);
		if(lua_isnumber(L,-1))
			day = luaL_checkint(L,-1);
		else if(lua_isnil(L,-1))
			day = 1;
		else
			ok = 0;
		if(day<1 || day >days[month])
				ok = 0;
	}
	if(ok && odbc_date!=NULL)
	{
		odbc_date->year = year;
		odbc_date->month = month ;
		odbc_date->day = day;
	}
	lua_settop(L,top);
	return ok;
}

int lua_datetime_param( lua_State *L,int index,SQL_DATE_STRUCT *odbc_date,SQL_TIMESTAMP_STRUCT *odbc_datetime )
{
	SQL_DATE_STRUCT date;
	int top = lua_gettop(L);
	int ok1 = lua_date_param(L,index,&date);
	int ok = ok1;
	int hour,minute,second,fraction;
	hour=minute=second=fraction=0;
	if(ok)	// 一定要有小时，否则不通过。
	{
		if(odbc_date)
			*odbc_date = date;
		lua_pushstring(L,"hour");
		lua_rawget(L,index);
		if(lua_isnumber(L,-1))
			hour = luaL_checkint(L,-1);
		else
			ok = 0;
		if(hour<0 || hour >23)
				ok = 0;
	}
	if(ok)
	{
		lua_pushstring(L,"minute");
		lua_rawget(L,index);
		if(lua_isnumber(L,-1))
			minute = luaL_checkint(L,-1);
		else if(lua_isnil(L,-1))
			minute = 0;
		else
			ok = 0;
		if(minute<0 || minute >59)
				ok = 0;
	}
	if(ok)
	{
		lua_pushstring(L,"second");
		lua_rawget(L,index);
		if(lua_isnumber(L,-1))
			second = luaL_checkint(L,-1);
		else if(lua_isnil(L,-1))
			second = 0;
		else
			ok = 0;
		if(second<0 || second >59)
				ok = 0;
	}
	if(ok)
	{
		lua_pushstring(L,"fraction");
		lua_rawget(L,index);
		if(lua_isnumber(L,-1))
			fraction = luaL_checkint(L,-1);
		else if(lua_isnil(L,-1))
			fraction = 0;
		else
			ok = 0;
		if(fraction<0)
				ok = 0;
	}
	if(ok && odbc_datetime)
	{
		odbc_datetime->year = date.year;
		odbc_datetime->month = date.month;
		odbc_datetime->day = date.day;
		odbc_datetime->hour = hour;
		odbc_datetime->minute = minute;
		odbc_datetime->second = second;
		odbc_datetime->fraction = fraction;
	}
	lua_settop(L,top);
	return ok+ok1;
}
/*
 * Fetch metadata from the database
 */

static void statement_fetch_metadata(lua_State *L, statement_t *statement) {
    bindparams_t *bind;
    int i;
	SQLSMALLINT nameSize, type, decimal;
	SQLUINTEGER width;
	SQLSMALLINT nullable;
	SQLCHAR name[1024];
	SQLRETURN retcode;
	lua_push_type_t lua_push;

    if (statement->metadata)
		return;

	if(statement->num_columns<=0)
	{
		statement->bind = NULL;
		statement->metadata = 1;
		return ;
	}
    statement->bind = (bindparams_t *)malloc(sizeof(bindparams_t) * (statement->num_columns+1));
    memset(statement->bind, 0, sizeof(bindparams_t) * (statement->num_columns+1));
    bind = statement->bind;

	for(i=1;i<=statement->num_columns;i++)
	{
		if(SQLDescribeCol(statement->stmt,i,name,1023,	&nameSize,	&type,
			&width,		&decimal,		&nullable)!=SQL_ERROR)
		{
			bind[i].name = malloc(nameSize+1);
			bind[i].name_len = nameSize;
			bind[i].max_len = width;
			bind[i].decimal = decimal;
			bind[i].data_type = type;
			strlower(name);
			strcpy(bind[i].name,name);
			//printf("field name:%s\n",bind[i].name);
			lua_push = odbc_to_lua_push(bind[i].data_type, bind[i].null);
			if(lua_push!=LUA_PUSH_BINARY)
			{
				if(lua_push==LUA_PUSH_DATE){
					bind[i].data = malloc(sizeof(SQL_DATE_STRUCT));
					retcode = SQLBindCol(statement->stmt,i,SQL_C_DATE,(SQLPOINTER)(bind[i].data),sizeof(SQL_DATE_STRUCT),&(bind[i].id));
				}else if (lua_push == LUA_PUSH_DATETIME) {
					bind[i].data = malloc(sizeof(SQL_TIMESTAMP_STRUCT));
					retcode = SQLBindCol(statement->stmt,i,SQL_C_TIMESTAMP,(SQLPOINTER)(bind[i].data),sizeof(SQL_TIMESTAMP_STRUCT),&(bind[i].id));
				}else {
					bind[i].data = malloc(bind[i].max_len+1);
					retcode = SQLBindCol(statement->stmt,i,SQL_C_CHAR,(SQLPOINTER)(bind[i].data),width+1,&(bind[i].id));
				}
				if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
				{
					GetErrorMessage(statement);
					luaL_error(L, "param get %s", statement->msg);
				}
			}
			else
			{
				bind[i].data = NULL;
			}
		}
	}
    statement->metadata = 1;
}


/*
 * num_affected_rows = statement:affected()
 */
static int statement_affected(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1,  ODBC_STATEMENT);
    SQLINTEGER affected;
    SQLINTEGER rc;

    if (!statement->stmt) {
        luaL_error(L, DBI_ERR_INVALID_STATEMENT);
    }

    /*
     * get number of affected rows
     */
    rc = SQLRowCount(statement->stmt,&affected);

    lua_pushinteger(L, affected);

    return 1;
}

/*
 * success = statement:close()
 */
int statement_close(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1,  ODBC_STATEMENT);
    int ok = 0;

	if(statement)
	{
	    if (statement->stmt!=SQL_NULL_HSTMT) {
			int rc = SQLFreeHandle(SQL_HANDLE_STMT, statement->stmt);    /* Free handles */
			statement->stmt = SQL_NULL_HSTMT;
		}

		if (statement->bind && statement->num_columns>0) {
			int i;
			for(i=1;i<=statement->num_columns;i++){
				free(statement->bind[i].name);
				if(statement->bind[i].data!=NULL) free(statement->bind[i].data);
			}
			free(statement->bind);
			statement->bind = NULL;
		}
		statement->conn->stmt_counter --;
		ok = 1;
	}
	//printf("Close odbc statement\n");
    lua_pushboolean(L, ok);
    return 1;
}

/*
 *  column_names = statement:columns()
 */
static int statement_columns(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1,  ODBC_STATEMENT);

    int i;
    int d = 1;

    if (!statement->stmt) {
        luaL_error(L, DBI_ERR_INVALID_STATEMENT);
        return 0;
    }

    //statement_fetch_metadata(L, statement);

    lua_newtable(L);
    for (i = 1; i <= statement->num_columns; i++) {
		const char *name = statement->bind[i].name;

		LUA_PUSH_ARRAY_STRING(d, name);
    }

    return 1;
}

static int statement_column_info(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1,  ODBC_STATEMENT);

    int i = luaL_checkint(L,2);
    int d = 1;

    if (!statement->stmt) {
        luaL_error(L, DBI_ERR_INVALID_STATEMENT);
        return 0;
    }
	if(i<1 || i>statement->num_columns){
        luaL_error(L, "字段序号超界");
        return 0;
	}
    //statement_fetch_metadata(L, statement);

    lua_newtable(L);
	LUA_PUSH_ATTRIB_STRING("name",statement->bind[i].name);
	LUA_PUSH_ATTRIB_STRING("type",odbc_type_str(statement->bind[i].data_type));
	LUA_PUSH_ATTRIB_INT("width",statement->bind[i].max_len);
	LUA_PUSH_ATTRIB_INT("decimal",statement->bind[i].decimal);
	LUA_PUSH_ATTRIB_BOOL("nullable",statement->bind[i].null);

    return 1;
}

/*
 * success,err = statement:execute(...)
 */
int statement_execute(lua_State *L) {
    int n = lua_gettop(L);	//sql语句参数个数
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1,  ODBC_STATEMENT);
    int i,p,type;
    const char *errstr = NULL;
    SQLSMALLINT num_columns;
//	SQLINTEGER StrLenOrInd;
	SQLINTEGER *sid;
    SQLRETURN retcode =SQL_SUCCESS;
	SQL_DATE_STRUCT date;
	SQL_TIMESTAMP_STRUCT datetime;
	char err[64];
	const void *value;
	size_t s;
	PTR pParamID;
	struct putdat_s {
		const char *v;
		SQLINTEGER len;
	} * put_dat_info;

    if (!statement->stmt) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, DBI_ERR_EXECUTE_INVALID);
		return 2;
    }
	/*绑定输入参数*/
	//StrLenOrInd = SQL_CURSOR_FORWARD_ONLY;
	//retcode = SQLSetStmtAttr(statement->stmt,SQL_ATTR_CURSOR_TYPE,&StrLenOrInd,sizeof(SQLINTEGER));
	//SQLPrepare函数准备好的SQL语句可以被SQLExecute函数多次执行。在每次执行时，可使用不同的语句参数。
	//但是，当调用SQLExecute函数再次执行一个SELECT语句时，应用程序必须先使用SQL＿CLOSE参数调用SQLFreeStmt函数，关闭与该语句句柄相关联的游标，废除它正在处理的结果集合，然后再执行SELECT语句，生成新的结果集合。
	SQLFreeStmt(statement->stmt,SQL_CLOSE);
	sid = malloc(sizeof(SQLINTEGER)*(n+1));
	put_dat_info = malloc(sizeof(struct putdat_s)*(n+1));

    for (p = 2; p <= n; p++) {
		i = p - 1;
		type = lua_type(L, p);
		put_dat_info[i].v = NULL;
		put_dat_info[i].len = 0;
		switch(type) {
			case LUA_TNIL:
				sid[i] = SQL_NULL_DATA;
				retcode = SQLBindParameter(statement->stmt,i,SQL_PARAM_INPUT,SQL_C_SSHORT,SQL_INTEGER,0,0,NULL,0,&(sid[i]));
				break;
			case LUA_TNUMBER:
				value = lua_tostring(L, p);
				sid[i] = SQL_NTS;
				retcode = SQLBindParameter(statement->stmt,i,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_CHAR,strlen(value),0,(SQLPOINTER)value,0,&(sid[i]));
				break;
			case LUA_TSTRING:
				value = luaL_checklstring(L, p, &s);
				if(s < 65535){
					sid[i] = s;
					retcode = SQLBindParameter(statement->stmt,i,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_CHAR,s,0,(SQLPOINTER)value,0,&(sid[i]));
				} else {
					//PTR pParmID;
					sid[i] = SQL_LEN_DATA_AT_EXEC(0);//SQL_DEFAULT_PARAM ;
					retcode = SQLBindParameter (statement->stmt,           // hstmt
                               i,                // ipar
                               SQL_PARAM_INPUT,  // fParamType
                               SQL_C_BINARY,       // fCType
                               SQL_LONGVARCHAR,  // FSqlType
                               0,           // cbColDef
                               0,                // ibScale
                               (VOID *)value,        // rgbValue
                               0,                // cbValueMax
                               &(sid[i]));     // pcbValue
					sid[i] = SQL_LEN_DATA_AT_EXEC(0);
					put_dat_info[i].v = value;
					put_dat_info[i].len = s;
					//if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					//{
					//	pParmID = i;
					//	retcode = SQLParamData(statement->stmt,&pParmID);
					//	if(retcode==SQL_NEED_DATA)
					//	{
					//		retcode = SQLPutData(statement->stmt,(SQLPOINTER)value,s);
					//		retcode = SQLPutData(statement->stmt,(SQLPOINTER)value,0);
					//	}
					//	retcode = SQLParamData(statement->stmt,&pParmID);
					//}
				}
				break;
			case LUA_TBOOLEAN:
				*(short int*)value = lua_toboolean(L, p);
				sid[i] = sizeof(short int);
				retcode = SQLBindParameter(statement->stmt,i,SQL_PARAM_INPUT,SQL_C_SSHORT,SQL_SMALLINT,sizeof(short int),0,(SQLPOINTER)value,sizeof(short int),&(sid[i]));
				break;
			case LUA_TUSERDATA: // 日期，日期时间
				value = lua_touserdata(L,p);
				retcode = 0;
				if (lua_getmetatable(L, p)) {  /* does it have a metatable? */
					lua_getfield(L, LUA_REGISTRYINDEX, ODBC_DATE);  /* get correct metatable */
					if (lua_rawequal(L, -1, -2))   /* does it have the correct mt? */
						retcode = 1;
					else{
						lua_pop(L,1);
						lua_getfield(L, LUA_REGISTRYINDEX, ODBC_DATETIME);  /* get correct metatable */
						if(lua_rawequal(L,-1,-2)) retcode = 2;
					}
				}
				lua_pop(L, 2);  /* remove both metatables */
				if(retcode==1) // date
				{
					sid[i] = sizeof(SQL_DATE_STRUCT);
					date = *(SQL_DATE_STRUCT *)value;
					retcode = SQLBindParameter(statement->stmt,i,SQL_PARAM_INPUT,SQL_C_DATE,SQL_DATE,SQL_DATE_LEN,0,(SQLPOINTER)&date,sizeof(SQL_DATE_STRUCT),&(sid[i]));
				}
				else if(retcode==2)
				{
					sid[i] = sizeof(SQL_TIMESTAMP_STRUCT);
					datetime = *(SQL_TIMESTAMP_STRUCT *)value;
					retcode = SQLBindParameter(statement->stmt,i,SQL_PARAM_INPUT,SQL_C_TIMESTAMP,SQL_TIMESTAMP,SQL_TIMESTAMP_LEN,0,(SQLPOINTER)&datetime,sizeof(SQL_TIMESTAMP_STRUCT),&(sid[i]));
				}
				else
				{
					retcode = SQL_ERROR;
					snprintf(err, sizeof(err)-1, DBI_ERR_BINDING_TYPE_ERR, lua_typename(L, type));
					errstr = err;
				}
				break;
			default:
				/*
				 * Unknown/unsupported value type
				 */
				retcode = SQL_ERROR;
				snprintf(err, sizeof(err)-1, DBI_ERR_BINDING_TYPE_ERR, lua_typename(L, type));
				errstr = err;
		}

		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
			break;
    }
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
	{
		lua_pushboolean(L, 0);
		if (errstr)
			lua_pushfstring(L, DBI_ERR_BINDING_PARAMS, errstr);
		else {
			GetErrorMessage(statement);
			lua_pushfstring(L, DBI_ERR_BINDING_PARAMS, statement->msg);
		}
		free(sid);
		free(put_dat_info);
		return 2;
	}
    /*
     * execute statement
     */
   retcode = SQLExecute(statement->stmt);
	if(retcode==SQL_NEED_DATA){
		while(SQLParamData(statement->stmt,&pParamID)==SQL_NEED_DATA)
		{
			for(i=1;i<n;i++)
			{
				if(put_dat_info[i].v == pParamID)
				{
					retcode = SQLPutData(statement->stmt,(SQLPOINTER)pParamID,put_dat_info[i].len);
					retcode = SQLPutData(statement->stmt,(SQLPOINTER)pParamID,0);
				}
			}
			retcode = SQLParamData(statement->stmt,&pParamID);
		}
	}else if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		 GetErrorMessage(statement);
		lua_pushboolean(L, 0);
		lua_pushfstring(L, DBI_ERR_BINDING_PARAMS, statement->msg);
		free(sid);
		free(put_dat_info);

		return 2;
	}
	free(sid);
	free(put_dat_info);
    /*
     * get number of columns
     */
	num_columns = 0;
	retcode = SQLNumResultCols(statement->stmt,&num_columns);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		 GetErrorMessage(statement);
		lua_pushboolean(L, 0);
		lua_pushfstring(L, DBI_ERR_BINDING_PARAMS, statement->msg);

		return 2;
    }
	//if(num_columns<=0)		// 非select语句，如update，create等语句
	//{
	//}

    statement->num_columns = num_columns;
    statement_fetch_metadata(L, statement);

    lua_pushboolean(L, 1);
    return 1;
}

/*
 * must be called after an execute
 */
static int statement_fetch_impl(lua_State *L, statement_t *statement, int named_columns) {
    SQLRETURN rc;
    bindparams_t *bind;
	lua_push_type_t lua_push;

    if (!statement->stmt) {
		luaL_error(L, DBI_ERR_FETCH_INVALID);
		return 0;
    }

    bind = statement->bind;

    rc = SQLFetch(statement->stmt);

    if (rc == SQL_NO_DATA) {
		/* No more rows */
		SQLCloseCursor(statement->stmt);
        lua_pushnil(L);
        return 1;
    } else if (rc == SQL_ERROR) {
		GetErrorMessage(statement);
		luaL_error(L, DBI_ERR_FETCH_FAILED, statement->msg);
		return 2;
    }
	//printf("Fetch %d,num_counts:%d\n",rc,statement->num_columns);
    if (statement->num_columns>0) {
		int i;
		int d = 1;

		lua_newtable(L);
		for (i = 1; i <= statement->num_columns; i++) {
			const char *name = bind[i].name;
			const char *data = bind[i].data;
			//printf("field name:%s, value:%s\n",name,data);
			bind[i].null = (SQL_NULL_DATA==bind[i].id);
			lua_push = odbc_to_lua_push(bind[i].data_type, bind[i].null);

			if (lua_push == LUA_PUSH_NIL) {
					if (named_columns) {
						LUA_PUSH_ATTRIB_NIL(name);
					} else {
						LUA_PUSH_ARRAY_NIL(d);
					}
			} else if (lua_push == LUA_PUSH_INTEGER) {
					int val = atoi(data);

					if (named_columns) {
						LUA_PUSH_ATTRIB_INT(name, val);
					} else {
						LUA_PUSH_ARRAY_INT(d, val);
					}
			} else if (lua_push == LUA_PUSH_NUMBER) {
					double val = strtod(data, NULL);

					if (named_columns) {
						LUA_PUSH_ATTRIB_FLOAT(name, val);
					} else {
						LUA_PUSH_ARRAY_FLOAT(d, val);
					}
			} else if (lua_push == LUA_PUSH_STRING) {
					if (named_columns) {
						LUA_PUSH_ATTRIB_STRING(name, data);
					} else {
						LUA_PUSH_ARRAY_STRING(d, data);
					}
			} else if (lua_push == LUA_PUSH_DATE) {
					SQL_DATE_STRUCT * date ;
					if(named_columns){ lua_pushstring(L,name);} // push name
					date = (SQL_DATE_STRUCT *)lua_newuserdata(L,sizeof(SQL_DATE_STRUCT )); // push value
					*date = *(SQL_DATE_STRUCT *)data;
					luaL_getmetatable(L,  ODBC_DATE);
					lua_setmetatable(L, -2);
					if(named_columns) lua_rawset(L,-3);
					else {lua_rawseti(L,-2,d); d++;}
					/*SQL_DATE_STRUCT date = *(SQL_DATE_STRUCT *)data;
					if (named_columns) {
						LUA_PUSH_ATTRIB_DATE(name, date.year,date.month,date.day);
					} else {
						LUA_PUSH_ARRAY_DATE(d, date.year,date.month,date.day);
					}		--*/
			} else if (lua_push == LUA_PUSH_DATETIME) {
					SQL_TIMESTAMP_STRUCT * date ;
					if(named_columns){ lua_pushstring(L,name);} // push name
					date = (SQL_TIMESTAMP_STRUCT *)lua_newuserdata(L,sizeof(SQL_TIMESTAMP_STRUCT )); // push value
					*date = *(SQL_TIMESTAMP_STRUCT *)data;
					luaL_getmetatable(L,  ODBC_DATETIME);
					lua_setmetatable(L, -2);
					if(named_columns) lua_rawset(L,-3);
					else {lua_rawseti(L,-2,d); d++;}
					/*SQL_TIMESTAMP_STRUCT datetime = *(SQL_TIMESTAMP_STRUCT *)data;
					if (named_columns) {
						LUA_PUSH_ATTRIB_DATETIME(name, datetime.year,datetime.month,datetime.day,datetime.hour,datetime.second,datetime.minute,datetime.fraction);
					} else {
						LUA_PUSH_ARRAY_DATETIME(d,  datetime.year,datetime.month,datetime.day,datetime.hour,datetime.second,datetime.minute,datetime.fraction);
					}		*/
			} else if (lua_push == LUA_PUSH_BINARY) {
					SQLINTEGER got = 0;
					char *buffer,tmp[2];
					SQLRETURN rc;
					int isnull = 0;
					short stype = SQL_C_BINARY;
					rc = SQLGetData(statement->stmt, i, stype, tmp, 0, &got);
					if(rc == SQL_ERROR){
							GetErrorMessage(statement);
							lua_pushnil(L);
							lua_pushfstring(L, DBI_ERR_FETCH_FAILED, statement->msg);
							return 2;
					}else if (got == SQL_NULL_DATA) {
						if (named_columns) {
							LUA_PUSH_ATTRIB_NIL(name);
						} else {
							LUA_PUSH_ARRAY_NIL(d);
						}
					}else { /* concat intermediary chunks */
						buffer = malloc(got+1);
						rc = SQLGetData(statement->stmt, i, stype, buffer,
							got, &got);
						if(rc==SQL_ERROR){
							free(buffer);
							GetErrorMessage(statement);
							lua_pushnil(L);
							lua_pushfstring(L, DBI_ERR_FETCH_FAILED, statement->msg);
							return 2;
						}else{
							if (named_columns) {
								LUA_PUSH_ATTRIB_BUFFER(name, buffer,got);
							} else {
								LUA_PUSH_ARRAY_BUFFER(d,buffer,got);
							}
							free(buffer);
						}
					}
					//if (!isnull && rc == SQL_ERROR) return fail(L, hSTMT, hstmt);
					/* return everything we got */
					//if(!isnull){ }
					//else{}
			} else if (lua_push == LUA_PUSH_BOOLEAN) {
					int val = atoi(data);

					if (named_columns) {
						LUA_PUSH_ATTRIB_BOOL(name, val);
					} else {
						LUA_PUSH_ARRAY_BOOL(d, val);
					}
			} else {
					lua_pushnil(L);
					lua_pushstring(L, DBI_ERR_UNKNOWN_PUSH);
					return 2;
			}
		}
    } else {
		/*
			 * no columns returned by statement?
			 */
		lua_pushnil(L);
    }

    return 1;
}

static int next_iterator(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, lua_upvalueindex(1),  ODBC_STATEMENT);
    int named_columns = lua_toboolean(L, lua_upvalueindex(2));

    return statement_fetch_impl(L, statement, named_columns);
}

/*
 * table = statement:fetch(named_indexes)
 */
int statement_fetch(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1,  ODBC_STATEMENT);
    int named_columns = lua_toboolean(L, 2);

    return statement_fetch_impl(L, statement, named_columns);
}

/*
 * num_rows = statement:rowcount()
 */
static int statement_rowcount(lua_State *L) {
    luaL_error(L, DBI_ERR_NOT_IMPLEMENTED,  ODBC_STATEMENT, "rowcount");

    return 0;
}

/*
 * iterfunc = statement:rows(named_indexes)
 */
static int statement_rows(lua_State *L) {
    if (lua_gettop(L) == 1) {
        lua_pushvalue(L, 1);
        lua_pushboolean(L, 0);
    } else {
        lua_pushvalue(L, 1);
        lua_pushboolean(L, lua_toboolean(L, 2));
    }

    lua_pushcclosure(L, next_iterator, 2);
    return 1;
}

/*
 * __gc
 */
static int statement_gc(lua_State *L) {
    /* always free the handle */
    statement_close(L);
    return 0;
}

/*
 * __tostring
 */
static int statement_tostring(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1,  ODBC_STATEMENT);

    lua_pushfstring(L, "%s: %p",  ODBC_STATEMENT, statement);

    return 1;
}

int  odbc_statement_create(lua_State *L, connection_t *conn, const char *sql_query) {
    //int rc;
    statement_t *statement = NULL;
    SQLHANDLE stmt;
	SQLRETURN retcode;
    //char *new_sql;

    /*
     * convert SQL string into a ODBC API compatible SQL statement
     */
    //new_sql = replace_placeholders(L, '?', sql_query);

    //rc = OCIHandleAlloc((dvoid *)conn->odbc, (dvoid **)&stmt, OCI_HTYPE_STMT, 0, (dvoid **)0);
    //rc = OCIStmtPrepare(stmt, conn->err, new_sql, strlen(new_sql), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT);
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, conn->dbc, &stmt);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		SQLSetStmtAttr(stmt, SQL_ATTR_ASYNC_ENABLE, SQL_ASYNC_ENABLE_OFF, 0);
		retcode = SQLPrepare(stmt,(SQLCHAR *)sql_query,SQL_NTS);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			statement_t s;
			s.stmt = stmt;
			GetErrorMessage(&s);
			lua_pushnil(L);
			lua_pushfstring(L, "Prepare ERROR:%s[%s]", s.msg,sql_query);
			return 2;
		}
	}
    //free(new_sql);
	conn->stmt_counter ++;
    statement = (statement_t *)lua_newuserdata(L, sizeof(statement_t));
	statement->conn = conn;
    statement->stmt = stmt;
    statement->num_columns = 0;
    statement->bind = NULL;
    statement->metadata = 0;

    luaL_getmetatable(L,  ODBC_STATEMENT);
    lua_setmetatable(L, -2);

    return 1;
}
//-------date----------
int odbc_date_new(lua_State *L){
	SQL_DATE_STRUCT * date = (SQL_DATE_STRUCT *)lua_newuserdata(L,sizeof(SQL_DATE_STRUCT ));
	time_t t;
	struct tm *tt;
	time(&t);
	tt = localtime(&t);
	date->year = tt->tm_year+1900;
	date->month = tt->tm_mon+1;
	date->day = tt->tm_mday;

    luaL_getmetatable(L,  ODBC_DATE);
    lua_setmetatable(L, -2);
	return 1;
}
static int odbc_date_gc(lua_State *L) {
    //SQL_TIMESTAMP_STRUCT * date = (connection_t *)luaL_checkudata(L, 1,  ODBC_DATE);
    // 没有需要手工清除的内存
    return 0;
}
static int odbc_date_set(lua_State *L){
    SQL_DATE_STRUCT *date = (SQL_DATE_STRUCT *)luaL_checkudata(L, 1,  ODBC_DATE);
	if(!lua_istable(L,2))
	{
		lua_pushnil(L);
		lua_pushstring(L,"需要以表为参数");
		return 2;
	}
	LUA_GET_ATTRIB_INT(2,date->year,	"year",1);
	LUA_GET_ATTRIB_INT(2,date->month,	"month",1);
	LUA_GET_ATTRIB_INT(2,date->day,		"day",1);
	lua_pushboolean(L,1);
	return 1;
}
static int odbc_date_get(lua_State *L){
    SQL_DATE_STRUCT *date = (SQL_DATE_STRUCT *)luaL_checkudata(L, 1,  ODBC_DATE);
	lua_newtable(L);
	LUA_PUSH_ATTRIB_INT("year",date->year);
	LUA_PUSH_ATTRIB_INT("month",date->month);
	LUA_PUSH_ATTRIB_INT("day",date->day);
	return 1;
}
static int odbc_date_tostring(lua_State *L){
	SQL_DATE_STRUCT *date = (SQL_DATE_STRUCT *)luaL_checkudata(L, 1,  ODBC_DATE);
	char buff[1024];
	sprintf(buff,"%d-%.2d-%.2d",  date->year,date->month,date->day);
	lua_pushstring(L, buff);
	return 1;
}
static int odbc_date_type(lua_State *L){
	SQL_DATE_STRUCT *date = (SQL_DATE_STRUCT *)luaL_checkudata(L, 1,  ODBC_DATE);
	lua_pushstring(L, "odbc_date");
	return 1;
}
//----------datetime-----------
int odbc_datetime_new(lua_State *L){
	SQL_TIMESTAMP_STRUCT * date = (SQL_TIMESTAMP_STRUCT *)lua_newuserdata(L,sizeof(SQL_TIMESTAMP_STRUCT ));
	time_t t;
	struct tm *tt;
	time(&t);
	tt = localtime(&t);
	date->year = tt->tm_year+1900;
	date->month = tt->tm_mon+1;
	date->day = tt->tm_mday;
	date->hour = tt->tm_hour;
	date->minute = tt->tm_min;
	date->second = tt->tm_sec;
	date->fraction = 0;
    luaL_getmetatable(L,  ODBC_DATETIME);
    lua_setmetatable(L, -2);
	return 1;
}
static int odbc_datetime_gc(lua_State *L) {
    //SQL_TIMESTAMP_STRUCT * date = (connection_t *)luaL_checkudata(L, 1,  ODBC_DATETIME);
    // 没有需要手工清除的内存
    return 0;
}
static int odbc_datetime_set(lua_State *L){
    SQL_TIMESTAMP_STRUCT *date = (SQL_TIMESTAMP_STRUCT *)luaL_checkudata(L, 1,  ODBC_DATETIME);
	int must = 1;
	if(!lua_istable(L,2))
	{
		lua_pushnil(L);
		lua_pushstring(L,"需要以表为参数");
		return 2;
	}
	LUA_GET_ATTRIB_INT(2,date->year,	"year",must);
	LUA_GET_ATTRIB_INT(2,date->month,	"month",must);
	LUA_GET_ATTRIB_INT(2,date->day,		"day",must);
	LUA_GET_ATTRIB_INT(2,date->hour,	"hour",0);
	LUA_GET_ATTRIB_INT(2,date->minute,	"minute",0);
	LUA_GET_ATTRIB_INT(2,date->second,	"second",0);
	LUA_GET_ATTRIB_INT(2,date->fraction,"fraction",0);
	lua_pushboolean(L,1);
	return 1;
}
static int odbc_datetime_get(lua_State *L){
    SQL_TIMESTAMP_STRUCT *date = (SQL_TIMESTAMP_STRUCT *)luaL_checkudata(L, 1,  ODBC_DATETIME);
	lua_newtable(L);
	LUA_PUSH_ATTRIB_INT("year",date->year);
	LUA_PUSH_ATTRIB_INT("month",date->month);
	LUA_PUSH_ATTRIB_INT("day",date->day);
	LUA_PUSH_ATTRIB_INT("hour",date->hour);
	LUA_PUSH_ATTRIB_INT("minute",date->minute);
	LUA_PUSH_ATTRIB_INT("second",date->second);
	LUA_PUSH_ATTRIB_INT("fracton",date->fraction);
	return 1;
}
static int odbc_datetime_tostring(lua_State *L){
	SQL_TIMESTAMP_STRUCT *date = (SQL_TIMESTAMP_STRUCT *)luaL_checkudata(L, 1,  ODBC_DATETIME);
	char buff[1024];
	sprintf(buff,"%d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d",  date->year,date->month,date->day,date->hour,date->minute,date->second,date->fraction);
	lua_pushstring(L, buff);
	return 1;
}
static int odbc_datetime_type(lua_State *L){
	SQL_TIMESTAMP_STRUCT *date = (SQL_TIMESTAMP_STRUCT *)luaL_checkudata(L, 1,  ODBC_DATETIME);
	lua_pushstring(L, "odbc_datetime");
	return 1;
}
//-----------------------------
int odbc_date(lua_State *L){
    static const luaL_Reg odbc_date_methods[] = {
	{"set",odbc_date_set},
	{"get",odbc_date_get},
	{"type",odbc_date_type},
	{NULL, NULL}
    };

//    static const luaL_Reg odbc_date_class_methods[] = {
//	{"new",odbc_date_new},
//	{NULL, NULL}
//    };
//
//    luaL_newmetatable(L,  ODBC_DATE);
//    //luaL_register(L, 0, odbc_date_methods);
//    luaL_newlib(L,odbc_date_methods);
//
//    lua_pushvalue(L,-1);
//    lua_setfield(L, -2, "__index");
//
//    //lua_pushcfunction(L, odbc_date_gc);
//    //lua_setfield(L, -2, "__gc");
//
//    lua_pushcfunction(L, odbc_date_tostring);
//    lua_setfield(L, -2, "__tostring");
//
//    //luaL_register(L,  ODBC_DATE, odbc_date_class_methods);
//    luaL_newlib(L,odbc_date_class_methods);

    createmeta(L,ODBC_DATE,odbc_date_methods,odbc_date_gc, odbc_date_tostring);
    return 1;
}
int odbc_datetime(lua_State *L){
    static const luaL_Reg odbc_datetime_methods[] = {
	{"set",odbc_datetime_set},
	{"get",odbc_datetime_get},
	{"type",odbc_datetime_type},
	{NULL, NULL}
    };

//    static const luaL_Reg odbc_datetime_class_methods[] = {
//	{"new",odbc_datetime_new},
//	{NULL, NULL}
//    };
//
//    luaL_newmetatable(L,  ODBC_DATETIME);
//    //luaL_register(L, 0, odbc_datetime_methods);
//    luaL_newlib(L,odbc_datetime_methods);
//
//    lua_pushvalue(L,-1);
//    lua_setfield(L, -2, "__index");
//
//    //lua_pushcfunction(L, odbc_date_gc);
//    //lua_setfield(L, -2, "__gc");
//
//    lua_pushcfunction(L, odbc_datetime_tostring);
//    lua_setfield(L, -2, "__tostring");
//
//    //luaL_register(L,  ODBC_DATETIME, odbc_datetime_class_methods);
//    luaL_newlib(L,odbc_datetime_class_methods);
    createmeta(L,ODBC_DATETIME,odbc_datetime_methods,odbc_datetime_gc, odbc_datetime_tostring);
    return 1;
}

int  odbc_statement(lua_State *L) {
    static const luaL_Reg statement_methods[] = {
	{"affected", statement_affected},
	{"close", statement_close},
	{"columns", statement_columns},
	{"column_info",statement_column_info},
	{"execute", statement_execute},
	{"fetch", statement_fetch},
	{"rowcount", statement_rowcount},
	{"rows", statement_rows},
	{NULL, NULL}
    };

//    static const luaL_Reg statement_class_methods[] = {
//	{NULL, NULL}
//    };

//    luaL_newmetatable(L,  ODBC_STATEMENT);
//    //luaL_register(L, 0, statement_methods);
//    luaL_newlib(L,statement_methods);
//
//    lua_pushvalue(L,-1);
//    lua_setfield(L, -2, "__index");
//
//    lua_pushcfunction(L, statement_gc);
//    lua_setfield(L, -2, "__gc");
//
//    lua_pushcfunction(L, statement_tostring);
//    lua_setfield(L, -2, "__tostring");
//
//    //luaL_register(L,  ODBC_STATEMENT, statement_class_methods);
//    luaL_newlib(L,statement_class_methods);
    createmeta(L,ODBC_STATEMENT,statement_methods,statement_gc, statement_tostring);

    return 1;
}
