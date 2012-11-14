-- 2011��6��25�գ��ΰ��ɣ�Ƕ��SQL���,�ṩ����foxpro�ı����䣬��ߴ�����Ч��
-- 2011��6��26�գ�����sharedb from x������#begindb#ǰ����䲻����������⡣���odbc��prepare���������⡣��ʾ��dbscript_sharedb.lua
-- �ؼ��� select,opendb,sqlprepare,sqlexec,create,update,delete,drop,insert,select x,scan to memvar,listtable
--         transaction,commit,rollback,sharedb from x,savedb to csv_filename,loaddb tablename from csv_filename
-- �����Ա��#begindb#���棬��䰴foxproϰ�ߣ�ʹ�÷ֺ�(;)���С���ʹ�÷ֺ�����ʱ���������кŻ����ӣ��Ե��Դ���һЩ���㣬�������ܡ�
-- ע�͡����������﷨����ʹ��lua�ķ���൱�ڳ�����������Ĺؼ����⣬lua���������Լ������������������lua��foxpro���ŵ㡣
-- �������⣬foxpro��odbc�����ܴ�������sqlite3��postgresql��û�����⡣

module("odbcscript", package.seeall)
local function alltrim (s)
       return (string.gsub(s, "^%s*(.-)%s*$", "%1"))
end

-- ���溯���ǰ�lua�ı�����ת�����ַ�������listtable��Ҫ�õ���
-- tableת�����ַ�������ת�������Ͱ���table��boolean��number��string��Ҫ��keyΪ�ַ�����Ϣ�����ϱ�������ʽҪ��
function table_str(value)
	local tmp,k,v,len
	tmp = ""
	for k,v in pairs(value) do
		if type(v)=="table" then
			tmp = tmp .. table_str(v)
		else
			len = #tmp
			tmp = tmp ..(len>0 and "," or "").. k .. "="
			if type(v)=='string' then
				if not string.find(v,"\"") then
					tmp = tmp ..'\"'..v..'\"'
				elseif not string.find(v,"'") then
					tmp = tmp ..'\''..v..'\''
				else
					tmp = tmp ..'[['..v..']]'
				end
			elseif type(v)=='boolean' then
				tmp = tmp .. (v and 'true' or 'false')
			elseif type(v)=='number' then
				tmp = tmp .. v
			else --�����ı�����
				-- tmp = tmp..tostring(v)
			end
		end
	end
	tmp = "{"..tmp.."}"
	return tmp
end
function date_str(v)
	if type(v)~='table' then
		return ""
	end
	return ""..v.year .. "-" .. v.month .. "-" .. v.day
end

function datetime_str(v)
	if type(v)~='table' then
		return ""
	end
	return ""..v.year .. "-" .. v.month .. "-" .. v.day .." " .. v.hour ..":" .. v.minute .. ":" .. v.second
end
function count_(field_name)
	local n,word;
	n = 0;
	for word in string.gmatch(field_name,"([^_]*)_?") do
		n = n + ((#word>0) and 1 or 0 );
	end
	return n;
end
-- mem_html_table({�ֶ����б�}��{���ݾ���}�� ���������� [{��ͬ������Ҫ�鲢���б�}]��[html��Ĳ���]��
-- ���{��ͬ������Ҫ�鲢���б�}�а���"head_raw",����head_rawҪ����,��{head_raw=true,1,2} ��ʾ��ͷ��Ҫ����ԭ״�����úϲ�
function mem_html_table(cols_name,tab_data,tab_data_rows,dat_span,table_params)
	local i,j,n,word
	local fld_i,tmp,k,v
	local html_table
	local tab_head, tab_head_rows,tab_head_cols
	local tab_ctrl, tab_head_str, tab_data_str
	dat_span = dat_span and dat_span or {}
	tab_head = {}
	tab_head_cols = 0
	tab_head_rows = 1
	fld_i = 1
	for k=1,#cols_name do
		tab_head[fld_i] = cols_name[k]
		tab_head_cols = fld_i
		tmp = count_(cols_name[k])
		if tmp>tab_head_rows then tab_head_rows = tmp; end
		fld_i = fld_i +1
	end
	i = 0
	-- ��ͷ�Ŀ��ƴ���
	tab_ctrl = {}
	if dat_span.head_raw then
		tab_head_rows = 1
		tab_ctrl[1] = {}
		for i=1,tab_head_cols do tab_ctrl[1][i] = tab_head[i]; end
	else
		for j = 1,tab_head_rows do tab_ctrl[j] = {}; end -- ��ʼ��ÿ�еĸ�������
		for i =1,tab_head_cols do  -- ��ָ��ֶεĵ���
			j = 1
			for word in string.gmatch(tab_head[i],"([^_]*)_?") do
				if #word>0 then tab_ctrl[j][i] = word; end
				j = j+1
			end
		end
	end
	for i = 1, tab_head_cols do -- ����ϲ�����
		n = 1
		for j=tab_head_rows,1,-1 do
			if tab_ctrl[j][i] then
				if n>1 then	tab_ctrl[j][i] = [[<th rowspan="]]..n..[[">]] .. tab_ctrl[j][i] .. [[</th>]]; end
				break
			else
				n = n +1
			end
		end
	end
	for j = 1, tab_head_rows do -- ����ϲ�����
		n = 1
		for i=tab_head_cols,1,-1 do
			if i>1 and tab_ctrl[j][i]==tab_ctrl[j][i-1] then
				n = n +1
				tab_ctrl[j][i] = nil
			else
				if tab_ctrl[j][i] and string.sub(tab_ctrl[j][i],1,3)~="<th" then
					if n >1 then
						tab_ctrl[j][i] = [[<th colspan="]]..n..[[">]] .. tab_ctrl[j][i] .. [[</th>]]
					else
						tab_ctrl[j][i] = [[<th>]] .. tab_ctrl[j][i] .. [[</th>]]
					end
				end
				n = 1
			end
		end
	end
	-- �����ͷ
	tab_head_str = "<thead> \n"
	for j=1,tab_head_rows do
		tab_head_str = tab_head_str ..[[<tr>]]
		for i=1, tab_head_cols do
			if tab_ctrl[j][i] then
				if string.sub(tab_ctrl[j][i],1,3)=="<th" then
					tab_head_str = tab_head_str .. tab_ctrl[j][i]
				else
					tab_head_str = tab_head_str .. [[<th>]]..tab_ctrl[j][i]..[[</th>]]
				end
			end
		end
		tab_head_str = tab_head_str .."</tr>\n"
	end
	tab_head_str = tab_head_str .. "</thead>\n"
	-- ����������Ϣ
	for _,i in pairs(dat_span) do
		if type(i)=="number" and i>=1 and i<= tab_head_cols and string.sub(tab_data[1][i],1,3)~="<td" then
			j = 2
			tmp = 1
			while j<=tab_data_rows do
				if tab_data[j][i]==tab_data[tmp][i] then
					tab_data[j][i] = nil
				else
					if j-tmp>1 then
						tab_data[tmp][i] = [[<td rowspan="]]..(j-tmp)..[[">]] .. tab_data[tmp][i] .. [[</td>]]
					else
						tab_data[tmp][i] = [[<td>]] .. tab_data[tmp][i] .. [[</td>]]
					end
					tmp = j
				end
				j = j +1
			end
			if j-tmp>1 then
				tab_data[tmp][i] = [[<td rowspan="]]..(j-tmp)..[[">]] .. tab_data[tmp][i] .. [[</td>]]
			end
		end
	end
	for i=1, tab_head_cols do  -- ��û��<td>��ǩ�Ĵ��ϱ�ǩ
		for j=1,tab_data_rows do
			if tab_data[j][i] and string.sub(tab_data[j][i],1,3)~="<td" then
				tab_data[j][i] =  [[<td>]] .. tab_data[j][i] .. [[</td>]]
			end
		end
	end
	tab_data_str = "<tbody>\n"   -- ��װ����
	for j=1,tab_data_rows do  -- ��û��<td>��ǩ�Ĵ��ϱ�ǩ
		tab_data_str = tab_data_str .. [[<tr>]]
		for i=1, tab_head_cols do
			if tab_data[j][i] then
				tab_data_str = tab_data_str .. tab_data[j][i]
			end
		end
		tab_data_str = tab_data_str .. "</tr>\n"
	end
	tab_data_str = tab_data_str .. "</tbody>\n"
	-- ��װ������html���
	--return '<table class="lua_table"' ..(type(table_params)=="string" and (" "..table_params) or "").. '>\n'..tab_head_str..tab_data_str.."</table>\n"
	return '<table' ..(type(table_params)=="string" and (" "..table_params) or "").. '>\n'..tab_head_str..tab_data_str.."</table>\n"
end
-- db_html_table(prepare_statement_obj,[{������Ҫ�鲢����ID���б�}],[html����������]}
function db_html_table(prepare,dat_span,table_params)
	local cols_name,tab_data,tab_data_rows
	local i,fld_i,k,v
	tab_head = {}
	tab_data = {}
	tab_head_cols = 0
	tab_head_rows = 1
	tab_data_rows = 0
	cols_name = prepare:columns()
	i = 0
	for row in prepare:rows(true) do
		i = i +1
		tab_data[i] = {}
		fld_i = 1
		for k=1,#cols_name do
			v = row[cols_name[k]]
			tab_data[i][fld_i] = (type(v)=='string' and v or tostring(v))
			fld_i = fld_i +1
		end
		tab_data_rows = tab_data_rows +1
	end
	return mem_html_table(cols_name,tab_data,tab_data_rows,dat_span,table_params)
end

function savedb_csv(filename,prepare)
	local i = 0
	local line = ""
	local row
	local handle,msg = io.open(filename,"w")
	if not handle then error("savedb to "..filename..":���ܴ��ļ�."..msg,2) end
	for row in prepare:rows(true) do
		if i == 0 then
			line = ""
			for k,v in pairs(row) do
				line = line .. (#line>0 and "," or "") .. k
			end
			handle:write(line .. "\n")
		end
		line = ""
		for k,v in pairs(row) do
			line = line ..(#line>0 and "," or "")..'"' .. string.gsub((type(v)=='string' and v or tostring(v)),'"','""') .. '"'
		end
		handle:write(line .. "\n")
		i = i +1
	end
	handle:close()
end
function loaddb_csv(filename,db,tablename,trans)
	local handle = io.open(filename,"r")
	local i = 0
	local line = ""
	local row,prepare,msg
	local fields = {},fi
	local values = {}
	local mins = {}
	local maxs = {}
	i = 0
	for line in handle:lines() do
		if i == 0 then
			-- ����ȡ���ֶ���
			fi = 0
			for v in string.gmatch(line,"([^,]*),?") do
				if #v>0 then
					fi = fi+1
					fields[fi] = v
				end
			end
		else
			-- ���������ֶο��
			fi = 0
			vs = ""
			for v,s in string.gmatch(line,[[([^,]*)(,?)]]) do
				vv = string.match(v,'^"(.*)"$')
				v = vv and vv or v
				v = string.gsub(v,'""','"')
				if s=="," then
					fi = fi+1
					values[fi] = vs .. v
					len = string.len(values[fi])
					if not mins[fi] then mins[fi] = len end
					if not maxs[fi] then maxs[fi] = len end
					if len < mins[fi] then mins[fi] = len end
					if len > maxs[fi] then maxs[fi] = len end
					vs = ""
				else
					vs = vs .. v
				end
			end
			if #vs>0 then
				fi = fi +1
				values[fi] = vs
				len = string.len(values[fi])
				if not mins[fi] then mins[fi] = len end
				if not maxs[fi] then maxs[fi] = len end
				if len < mins[fi] then mins[fi] = len end
				if len > maxs[fi] then maxs[fi] = len end
			end
		end
		i = i + 1
		if i>100 then break end
	end
	handle:seek("set",0)
	-- ����Ƿ���ڱ�
	prepare,msg = db:prepare("select * from ".. tablename)
	if prepare then ok,msg = prepare:execute() end
	if not prepare or not ok then
		--������
		sqlstr = "create table " .. tablename .. " ("
		for fi,v in ipairs(fields) do
			if v and #v>0 then
				--print(fi,v,mins[fi],maxs[fi])  -- for debug
				sqlstr = sqlstr ..(fi==1 and "" or ",").. v .. " char(" .. (mins[fi]==maxs[fi] and maxs[fi] or maxs[fi]+20) .. ")"
			end
		end
		sqlstr = sqlstr .. ")"
		print(sqlstr)  -- for debug
		prepare,msg = db:prepare(sqlstr)
		ok,msg = prepare:execute()
		if not ok then error("loaddb "..tablename ..":�������ʧ��."..msg,2) end
	end
	-- �����������
	if not trans then db:autocommit(false) end

	sqlstr = "insert into ".. tablename .. " ("
	sqlval = ""
	for fi,v in ipairs(fields) do
		sqlstr = sqlstr .. (fi==1 and "" or ",") .. v
		sqlval = sqlval .. (fi==1 and "" or ",") .. "?"
	end
	sqlstr = sqlstr .. ") values (" .. sqlval .. ")"
	prepare,msg = db:prepare(sqlstr)
	--print(sqlstr)  -- for debug
	if not prepare then error("loaddb "..tablename ..":insert����prepareʧ��"..msg,2) end
	i = 0
	for line in handle:lines() do
		if i == 0 then
			-- ��һ�У��ֶ���������
		else
			-- ������ֶ�����
			fi = 0
			vs = ""
			continue = false
			for v,s in string.gmatch(line,[[([^,]*)(,?)]]) do
				v = vs .. v
				cstart = string.sub(v,1,1)
				csecond = string.sub(v,2,2)
				cend = string.sub(v,-1,-1)
				if cstart~='"' and cend~='"' then
					fi = fi+1
					v = string.gsub(v,'""','"')
					values[fi] = alltrim(v)
					vs = ""
				elseif cstart == '"' and cend=='"' and #v>2 then
					v = string.sub(v,2,-2)
					fi = fi+1
					v = string.gsub(v,'""','"')
					values[fi] = alltrim(v)
					vs = ""
				elseif cend=='"' then
					fi = fi+1
					v = string.gsub(v,'""','"')
					values[fi] = alltrim(v)
					vs = ""
				else   -- cstart = '"'
					-- �ֶα��ֶΣ���Ҫ����ȡ�����ֵ
					vs = v..(s and s or "")
				end
			end
			if fi<#fields then
				fi = fi +1
				values[fi] = alltrim(vs)
			end
			val_str = ""
			for fi=1,#fields do val_str = val_str ..(#val_str>0 and "," or "") .."values["..fi.."]" end
			ok,msg = prepare:execute(loadstring("return "..val_str)())
			if not ok then
				print(table_str(values))
				error("loaddb "..tablename ..":insert����query�ύ����ʧ��"..msg,2)
			end
		end
		i = i + 1
		if (i%5000)==0 then
			db:commit()
			db:autocommit(false)
		end
	end
	db:commit()
	if trans then db:autocommit(false)
	else db:autocommit(true) end
	handle:close()
end
-- ��ε���ʱ����ʹ��loaddbscript��һ�α�����ʹ�ã����Ч��
function loaddbscript(source,ds_debug)
	local i = 0
	local dbstat = false
	local dbarea = "1"
	local script = [[require("luaodbc"); local _odbc = odbc.connection;]]
	local script_env = {}
	local continue = false
	local k,k_save,k_tmp,sqlkey,params
	k_save = ""
	for k in string.gmatch(source,"([^\n]*)[\n]?") do
		i = i +1
		k = alltrim(k)
		k_tmp,continue = string.match(k,"(.*)(;)$")
		k = k_save .. (#k_save>0 and "\n" or "") .. k
		if continue then
			k_save = k_save .. (#k_save>0 and "\n" or "") .. k_tmp
		elseif k == "#begindb#" then
			dbstat = true
			script = script .. "--" .. k .. "\n"
		elseif k == "#enddb#" then
			dbstat = false
			script = script .. "--" .. k .. "\n"
		elseif dbstat then
			keyword = string.match(k,"^(%a+)%s*")
			if keyword=="select" then
				keyword,area = string.match(k,"(select)%s+(%d+)$")
				if keyword then
					dbarea = area
					script = script .. "if not aprdb then aprdb={} end ; if not aprdb["..dbarea.."] then aprdb["..dbarea.."]= {} end" .. "\n"
				else
					script = script .. "aprdb["..dbarea.."].prepare, aprdb["..dbarea.."].msg = aprdb["..dbarea.."].db:prepare([["..k.."]]);" ..
									"aprdb["..dbarea.."].ok, aprdb["..dbarea.."].msg = aprdb["..dbarea.."].prepare:execute();" ..
									   "if not aprdb["..dbarea.."].ok then error([[("..tostring(i)..")"..k..":]] ..aprdb["..dbarea.."].msg,2) end\n"
				end
			elseif keyword =="opendb" then
				keyword,driver,connect = string.match(k,"(opendb)%s*(%w+),%s*(.*)")
				script = script .. "aprdb["..dbarea.."].db,aprdb["..dbarea.."].msg = _odbc.new([["..connect.."]]);"..
								  "if not aprdb["..dbarea.."].db then error([[("..tostring(i)..")"..k..":]] ..aprdb["..dbarea.."].msg,2) end\n"
				if not script_env[dbarea] then script_env[dbarea] = {} end
				script_env[dbarea].db = k -- ��¼ k ���ݿ��Ѿ���
			elseif keyword =="sharedb" then
				memvar = string.match(k,"^sharedb%s*from%s*(%d+)")
				if memvar then
					script = script .. "if not aprdb["..memvar.."] or not aprdb["..memvar.."].db then error([[("..tostring(i)..")"..k..":Դ���ݿ�û�����Ӻ�]],2) end;"..
							"aprdb["..dbarea.."].db = aprdb["..memvar.."].db \n"
					if not script_env[dbarea] then script_env[dbarea] = {} end
					script_env[dbarea].db = k -- ��¼ k ���ݿ��Ѿ���
				else
					script = script .. k .. "\n"
				end
			elseif keyword =="create" or keyword=="insert" or keyword=="delete" or keyword=="update" or keyword == 'drop' then
				if not script_env[dbarea] or not script_env[dbarea].db then error("("..tostring(i)..")"..k..":ʹ��sqlǰ���ȴ����ݿ⡣",2) end
				script = script ..  -- "if not aprdb["..dbarea.."].db then error([["..k..":��ͼʹ��û�д򿪵����ݿ�]],2) end;" ..
									"aprdb["..dbarea.."].ok, aprdb["..dbarea.."].msg = aprdb["..dbarea.."].db:query([["..k.."]]);" ..
									"if not aprdb["..dbarea.."].ok then error([[("..tostring(i)..")"..k..":]] ..aprdb["..dbarea.."].msg,2) end\n"
			elseif keyword =="sqlprepare" then
				sqlkey,sql = string.match(k,"sqlprepare%s*(%a+)%s*(.*)")
				if not script_env[dbarea] then script_env[dbarea] = {} end
				script_env[dbarea].sqlkey = sqlkey -- ��¼sql�Ĺؼ��֣���execute���ж�ʹ��query����select
				if not script_env[dbarea] or not script_env[dbarea].db then error("("..tostring(i)..")"..k..":ʹ��sqlprepareǰ���ȴ����ݿ⡣",2) end
				script = script ..  --"if not aprdb["..dbarea.."].db then error([["..k..":��ͼʹ��û�д򿪵����ݿ�]],2) end;" ..
								"aprdb["..dbarea.."].prepare, aprdb["..dbarea.."].msg = aprdb["..dbarea.."].db:prepare([["..sqlkey..' ' ..sql.."]]);" ..
								"if not aprdb["..dbarea.."].prepare then error([[("..tostring(i)..")"..k..":]] ..aprdb["..dbarea.."].msg,2) end\n"
			elseif keyword =="sqlexec" then
				params = string.match(k,"sqlexec%s*(.*)")
				if not script_env[dbarea] or not script_env[dbarea].sqlkey then error("("..tostring(i)..")"..k..":ʹ��sqlexecǰ��ʹ��sqlprepare��",2) end
				sqlkey = script_env[dbarea].sqlkey
				-- ������Ϊ���sqlexecЧ�ʣ�������䡣��������򣬶Ե�����ʾ��Ϣ�����Ѻá������ֵͼ�����Ӧ�ò����������Ǿ������
				--script = script .. "if not aprdb["..dbarea.."].prepare then error([["..k..":ʹ��sqlexecǰ��ʹ��sqlprepare��]],2) end;"
				if sqlkey == "create" or sqlkey=="insert" or sqlkey=="update" or sqlkey=="delete" or sqlkey=='drop' then
					script = script .. "aprdb["..dbarea.."].ok, aprdb["..dbarea.."].msg = aprdb["..dbarea.."].prepare:execute(".. params .. ");" ..
									 "if not aprdb["..dbarea.."].ok then error([["..k..":]] ..aprdb["..dbarea.."].msg,2) end\n"
				else
					script = script .. "aprdb["..dbarea.."].ok, aprdb["..dbarea.."].msg = aprdb["..dbarea.."].prepare:execute(".. params .. ");" ..
									 "if not aprdb["..dbarea.."].ok then error([["..k..":]] ..aprdb["..dbarea.."].msg,2) end\n"
				end
			elseif keyword =="transaction" or keyword=="transbegin" then
				if not script_env[dbarea] or not script_env[dbarea].db then error(k..":ʹ��transactionǰ���ȴ����ݿ⡣",2) end
				script = script .. "aprdb["..dbarea.."].trans, aprdb["..dbarea.."].msg = aprdb["..dbarea.."].db:autocommit(false);" ..
								   "if not aprdb["..dbarea.."].trans then error([[("..tostring(i)..")"..k..":]] ..aprdb["..dbarea.."].msg,2) end\n"
			elseif keyword =="commit" then
				if not script_env[dbarea] or not script_env[dbarea].db then error(k..":ʹ��commitǰ���ȴ����ݿ⡣",2) end
					script = script .. "aprdb["..dbarea.."].ok, aprdb["..dbarea.."].msg = aprdb["..dbarea.."].db:commit();" ..
									   "if not aprdb["..dbarea.."].ok then error([[("..tostring(i)..")"..k..":]] ..aprdb["..dbarea.."].msg,2) end\n"
			elseif keyword =="rollback" then
				if not script_env[dbarea] or not script_env[dbarea].db then error(k..":ʹ��rollbackǰ���ȴ����ݿ⡣",2) end
					script = script .. "aprdb["..dbarea.."].ok, aprdb["..dbarea.."].msg = aprdb["..dbarea.."].db:rollback();" ..
									   "if not aprdb["..dbarea.."].ok then error([[("..tostring(i)..")"..k..":]] ..aprdb["..dbarea.."].msg,2) end\n"
			elseif keyword =="listtable" then
				script = script .. "for row in aprdb["..dbarea.."].prepare:rows(true) do print(odbcscript.table_str(row)) end" .."\n"

			elseif keyword =="scan" then
				memvar = string.match(k,"scan%s*to%s*(%a%w*)")
				if memvar then
					script = script .."for "..memvar.." in aprdb["..dbarea.."].prepare:rows(true)  do\n"
				else
					script = script .. k .."\n"
				end
			elseif keyword == "savedb" then
				memvar = string.match(k,"^savedb%s*to%s*(.*)")
				if memvar then
					script = script .."odbcscript.savedb_csv([["..memvar.."]],aprdb["..dbarea.."].prepare)\n"
				else
					script = script .. k .."\n"
				end
			elseif keyword == "loaddb" then
				tablename,memvar = string.match(k,"^loaddb%s*(.-)%s*from%s*(.*)")
				if memvar then
					script = script .."odbcscript.loaddb_csv([["..memvar.."]],aprdb["..dbarea.."].db,[["..tablename.."]],aprdb["..dbarea.."].trans)\n"
				else
					script = script .. k .."\n"
				end
			elseif keyword == "htmltable" then
				params = nil
				if k==keyword then
					memvar="{}"
				else
					memvar,params = string.match(k,"^htmltable%s*rowspan%s*([^%s]*)%s*params%s*(.*)")  -- �﷨��htmltable [rowspan {1,2} [params border="1" id='23']]
					if not memvar then
						memvar = string.match(k,"^htmltable%s*rowspan%s*(.*)")
					end
					if not memvar then
						params = string.match(k,"^htmltable%s*params%s*(.*)")  -- �﷨��htmltable [rowspan {1,2} [params border="1" id='23']]
						if params then memvar = "{}" end
					end
				end
				if memvar then
					script = script .."print(odbcscript.db_html_table(aprdb["..dbarea.."].prepare,"..memvar..(params and ",[["..params.."]]" or "").."))\n"
				else
					script = script ..k .. "\n"
				end
			else
				script = script .. k .. "\n"
			end
		else  -- not dbstat
			continue = false
			k_save = ""
			script = script .. k .. "\n"
		end
		if not continue then k_save = "" end
	end
	local dbfun,msg = loadstring(script)
	if not dbfun then
		print(script)
		error("�������:"..msg,2)
	end
	if ds_debug then
		print(script)
	end
	return dbfun,msg
end
--�ṩ�����һ��ʹ�õ���
function run(source,ds_debug)
	local dbfun = loaddbscript(source,ds_debug)
	return dbfun()
end
--�ṩ���ļ�����һ���Ե���
function runfile(filename,dsdebug)
	local file,msg = io.open(filename)
	if not file then error(msg,2) end
	local script_str = file:read("*a")
	file:close()
	return run(script_str,dsdebug)
end
