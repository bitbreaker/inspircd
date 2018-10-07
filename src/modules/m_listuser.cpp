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
#include <sstream>

/* $ModDesc: Provides /LISTUSER functionality */



class ListUserQuery : public SQLQuery
{
 public:
	const std::string uid;
	bool verbose;
	ListUserQuery(Module* me, const std::string& u)
		: SQLQuery(me), uid(u)
	{
	}
	
	void WriteText(std::string notice)
	{
		User* src = ServerInstance->FindNick(uid);
		src->SendText(":%s %03d %s : %s", ServerInstance->Config->ServerName.c_str(), RPL_ADMINLOC1, src->nick.c_str(), notice.c_str());
	}
	
	void OnResult(SQLResult& res)
	{
		SQLEntries row;
		std::stringstream ss;
		ServerInstance->Logs->Log("LISTUSER",DEFAULT,"OnResult Query");
		WriteText("List of authentificated users");			
		WriteText("+-----------------+---------------------------------+--------------------------+------------------+");	
		WriteText("| User            | Password                        | Host                     | IP               |");		
		WriteText("+-----------------+---------------------------------+--------------------------+------------------+");	
				
			
		while (res.GetRow(row))
		{
			std::string username = row[0].value;
			std::string password = row[1].value;
			std::string host = row[2].value;
			std::string ip = row[3].value;
			
			username.append(15 - username.length(), ' ');
			password.append(31 - password.length(), ' ');
			host.append(24 - host.length(), ' ');
			ip.append(16 - ip.length(), ' ');
			
			
			WriteText("| "+username+" | "+password+" | "+host+" | "+ip+" |");
			WriteText("+-----------------+---------------------------------+--------------------------+------------------+");
		}
	}

	void OnError(SQLerror& error)
	{
		ServerInstance->SNO->WriteGlobalSno('a', "Forbidden connection (SQL query failed: %s)", error.Str());
	}
};


/** Handle /LISTUSER
 */
class CommandListUser : public Command
{
 	std::string query;
	dynamic_reference<SQLProvider> SQL;
	Module* module;


 public:
	/* Command 'listuser', needs operator */
	CommandListUser(Module* me) : Command(me,"LISTUSER", 0), SQL(me, "SQL")
	{
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
		query = "SELECT username, password, host, ip from users";
		
		ServerInstance->Logs->Log("LISTUSER",DEFAULT,"Submit Query is: %s", query.c_str());
		// SQL->submit()
		SQL->submit(new ListUserQuery(module, src->uuid), query);

		return CMD_SUCCESS;
	}


};

class ModuleListUser : public Module
{
	CommandListUser cmd;
 
 public:
	ModuleListUser()
		: cmd(this)
	{
	}

	void init()
	{
		ServerInstance->Modules->AddService(cmd);
	}


	virtual ~ModuleListUser()
	{
	}

	virtual Version GetVersion()
	{
		return Version("Provides /LISTUSER functionality", VF_VENDOR);
	}
};

MODULE_INIT(ModuleListUser)

