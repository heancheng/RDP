// include standard SQL/ODBC "C" APIs
#include <windows.h>
#include <odbcinst.h>
#include <sql.h>        // core
#include <sqlext.h>     // extensions
#include <sqltypes.h>
#include <sqlucode.h>
/////////////////////////////////////////////////////////////////////////////
// Win32 libraries
#include <common.h>

#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#define  ODBC_CONNECTION	"odbc_connection"
#define  ODBC_STATEMENT		"odbc_statement"
#define  ODBC_DATE			"odbc_date"
#define  ODBC_DATETIME		"odbc_datetime"

const char * get_error(short type,HANDLE h,char * msg, short strLen);

typedef struct _bindparams {
    unsigned char *name;
    unsigned int name_len;
    short data_type;
    unsigned short max_len;
	unsigned short decimal;
	SQLINTEGER  id;
    char *data;
    short null;
} bindparams_t;

/*
 * connection object
 */
typedef struct _connection {
    SQLHANDLE odbc;
    SQLHANDLE dbc;
    SQLRETURN err;
    //OCIServer *srv;
    //OCISession *auth;
	int stmt_counter;
	char * conn_string;
    int autocommit;
} connection_t;

/*
 * statement object
 */
typedef struct _statement {
	connection_t * conn;
    SQLHANDLE stmt;
    int num_columns;
    bindparams_t *bind;

    int metadata;
	SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH];
} statement_t;
