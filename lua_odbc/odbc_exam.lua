odbc = require("luaodbc")
--db = odbc.connection;
conn,msg = odbc.new("dsn=oa_list")
print(conn,msg);
prepare,msg = conn:prepare("select dcode,拟稿人,标题 from oa_list")
print(prepare,msg);
if prepare:execute() then
	n = 1
	for row in prepare:rows(true) do
		for k,v in pairs(row) do io.write(k.."="..tostring(v).."\t") end
		print("")
		n = n +1
		if n>20 then
			break
		end
	end
end
	stmt,msg = conn:prepare([[select * from test2]])
	print('prepare:',stmt,msg)
	print(stmt:execute())
--获取字段信息
	cols = stmt:columns();
	for k,v in pairs(cols) do print(k,v,type(v)); info,msg=stmt:column_info(k); for k,v in pairs(info) do print('\t',k,v); end; end
--获取字段内容
	row,msg = stmt:fetch(true)
	print('fetch:',row,msg)
	for k,v in pairs(row) do print(k,v,type(v)) end
	print('fd4:----------------------')
	print(row.fd4,"type:"..type(row.fd4))
	print('fd5:----------------------')
	print(row.fd5,"type:"..type(row.fd5))
--测试各种类型参数
	stmt,msg = conn:prepare([[select * from test2 where fd5=?]])
	print('prepare:',stmt,msg)
	date = odbc.date.new();
	date:set{year=2011,month=5,day=23}
	print(stmt:execute(date))
	row,msg = stmt:fetch(true)
	print('fetch:',row,msg)
	for k,v in pairs(row) do print(k,v) end
	date = odbc.datetime.new();date:set{year=2011,month=5,day=23,hour=0}
	print(stmt:execute(date))
	row,msg = stmt:fetch(true)
	print('fetch:',row,msg)
	for k,v in pairs(row) do print(k,v) end
--test date
	date = odbc.date.new()
	print(date,date:type())
	tt,msg = odbc.datetime.new()
	print(tt,tt:type())
	print('kasdfasdf')
	--print(conn:close())
	conn = nil

