/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2007 Dennis Friis <peavey@inspircd.org>
 *   Copyright (C) 2007 Carsten Valdemar Munk <carsten.munk+inspircd@gmail.com>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "inspircd.h"
#include "sql.h"

/* $ModDesc: Provides /ADDIP functionality */


class GetIpQuery : public SQLQuery
{
 public:
	const std::string uid;
	const std::string addip;
	bool verbose;
	GetIpQuery(Module* me, const std::string& u, const std::string& ip)
		: SQLQuery(me), uid(u), addip(ip)
	{
	}
	
	void OnResult(SQLResult& res)
	{
		SQLEntries row;
		User* src = ServerInstance->FindNick(uid);
		
		if (res.GetRow(row))
		{
			std::string ip = row[0].value;
			vector<string> ips;
			istringstream f(ip);
			string s;    
			while (getline(f, s, ';')) {
				src->SendText(":%s %03d %s : %s", ServerInstance->Config->ServerName.c_str(), RPL_ADMINLOC1, src->nick.c_str(), s.c_str());
				ips.push_back(s);
			}
			
					
			query = "update users set (ip = \""+addip+"\" where username = \""+username+"\";";
			
			ServerInstance->Logs->Log("ADDIP",DEFAULT,"Submit Query is: %s", query.c_str());
			// SQL->submit()
			SQL->submit(new AddIpQuery(module, src->uuid), query);
		}
	}

	void OnError(SQLerror& error)
	{
		ServerInstance->SNO->WriteGlobalSno('a', "Forbidden connection (SQL query failed: %s)", error.Str());
	}
};


class AddIpQuery : public SQLQuery
{
 public:
	const std::string uid;
	bool verbose;
	AddIpQuery(Module* me, const std::string& u)
		: SQLQuery(me), uid(u)
	{
	}
	
	void OnResult(SQLResult& res)
	{
		SQLEntries row;
		User* src = ServerInstance->FindNick(uid);
		ServerInstance->Logs->Log("ADDIP",DEFAULT,"OnResult Query");
		src->WriteServ("NOTICE %s :*** Added IP",src->nick.c_str());
	}

	void OnError(SQLerror& error)
	{
		ServerInstance->SNO->WriteGlobalSno('a', "Forbidden connection (SQL query failed: %s)", error.Str());
	}
};


/** Handle /ADDIP
 */
class CommandAddIp : public SplitCommand
{
 	std::string query;
	dynamic_reference<SQLProvider> SQL;
	Module* module;


 public:
	/* Command 'ADDIP', needs operator */
	CommandAddUser(Module* me) : SplitCommand(me,"ADDIP",2,2), SQL(me, "SQL")
	{
		syntax = "<username> <ip>";
	    flags_needed = 'o'; 
		module = me;
		ConfigTag* tag = ServerInstance->Config->ConfValue("sqlauth");

		std::string dbid = tag->getString("dbid");
		if (dbid.empty())
			SQL.SetProvider("SQL");
		else
			SQL.SetProvider("SQL/" + dbid);
    }

	CmdResult Handle (const std::vector<std::string> &parameters, User *src)
	{
		std::string username = parameters[0];
		std::string ip = parameters[1];
		
		query = "select ip from users where username = \""+username+"\";";
		
		ServerInstance->Logs->Log("ADDIP",DEFAULT,"Submit Query is: %s", query.c_str());
		// SQL->submit()
		SQL->submit(new GetIpQuery(module, src->uuid, ip), query);

		return CMD_SUCCESS;
	}
};

class ModuleAddUser : public Module
{
	CommandAddUser cmd;
 
 public:
	ModuleAddUser()
		: cmd(this)
	{
	}

	void init()
	{
		ServerInstance->Modules->AddService(cmd);
	}


	virtual ~ModuleAddUser()
	{
	}

	virtual Version GetVersion()
	{
		return Version("Provides /ADDIP functionality", VF_VENDOR);
	}
};

MODULE_INIT(ModuleAddUser)

