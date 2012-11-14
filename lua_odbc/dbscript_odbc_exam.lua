require("odbcscript")
script = [=[
print(os.clock())
#begindb#
select 1
opendb odbc,dsn=oa_list
-- foxpro odbc 创建表有问题，原因未知。
--transaction
--sqlprepare create table aab (FD1 C(10),FD2 C(12))
--sqlexec
--print(aprdb[1].db,aprdb[1].ok,aprdb[1].msg)
--commit
--transaction
--insert into aab values ('akdf', 213.2)
--insert into aab values ('akdf', 213.2)
--select * from aab
--listtable
--   select * from oa_list
--   savedb to kadf.csv
--    select * from oa_page
--    savedb to oa_page.csv
--transaction
--loaddb oamyload from kadf.csv
--commit
select dcode 文件_代码,标题 文件_标题,拟稿人,拟稿日期,状态,文号 from oa_list where recno()<=1000
print("<html>")
print([[<style>
table thead tr {
  position:relative;
  top:expression((this.offsetParent.scrollTop>this.parentElement.parentElement.offsetTop?this.offsetParent.scrollTop-this.parentElement.parentElement.offsetTop-1:0)-1);
}
table thead tr th{
  position:relative;
  background: rgb(208,232,255);
}
</style>
]])
print("<body>")
htmltable rowspan {4} params class="lua_table" border="1" id="table_1"
--htmltable rowspan {head_raw=true,4,5} params class="lua_table" border="1" id="table_1"
print([[</body></html>]])
select * from oa_list
select 2
--sharedb from 1
opendb odbc,dsn=oa_list
sqlprepare select * from oa_list where dcode = ?
select 1
local i= 0
scan to mem1
	i = i +1
	--print("work1:",odbcscript.table_str(mem1))
	select 2
	sqlexec mem1.dcode
	scan to mem2
		--print("work2:",odbcscript.table_str(mem2))
	end
end
--commit
print(os.clock())
#enddb#
return i
]=]
-- 根据上面测试，切换数据库工作区后，需要重新连接数据库，这应该是apr.dbd的一个bug。
--print(pcall(dbscript.run,script,true))
ret,msg = odbcscript.run(script,--[[debug = ]]false)
print("row num:",ret,msg)
--print("row num:",dbscript.runfile("dbscript_ex2.lsql"))
