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

/* $ModDesc: Provides /DELUSER functionality */



class DelUserQuery : public SQLQuery
{
 public:
	const std::string uid;
	bool verbose;
	DelUserQuery(Module* me, const std::string& u)
		: SQLQuery(me), uid(u)
	{
	}
	
	void OnResult(SQLResult& res)
	{
		SQLEntries row;
		User* src = ServerInstance->FindNick(uid);
		ServerInstance->Logs->Log("DELUSER",DEFAULT,"OnResult Query");
		src->WriteServ("NOTICE %s :*** Deleted user",src->nick.c_str());
	
/*
	while (res.GetRow(row))
		{
			ServerInstance->Logs->Log("DELUSER",DEFAULT,"OnResult Query %s for %s",row[0].value.c_str(),src->nick.c_str());
			src->WriteServ("NOTICE %s :*** Deleted user %s",src->nick.c_str(), row[0].value.c_str());			
		}
		*/
	}

	void OnError(SQLerror& error)
	{
		ServerInstance->SNO->WriteGlobalSno('a', "Forbidden connection (SQL query failed: %s)", error.Str());
	}
};


/** Handle /DELUSER
 */
class CommandDelUser : public SplitCommand
{
 	std::string query;
	dynamic_reference<SQLProvider> SQL;
	Module* module;


 public:
	/* Command 'deluser', needs operator */
	CommandDelUser(Module* me) : SplitCommand(me,"DELUSER",1,1), SQL(me, "SQL")
	{
		syntax = "<username>";
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
		
		query = "DELETE FROM users where username= \""+username+"\"";
		
		ServerInstance->Logs->Log("DELUSER",DEFAULT,"Submit Query is: %s", query.c_str());
		// SQL->submit()
		SQL->submit(new DelUserQuery(module, src->uuid), query);

		return CMD_SUCCESS;
	}


};

class ModuleDelUser : public Module
{
	CommandDelUser cmd;
 
 public:
	ModuleDelUser()
		: cmd(this)
	{
	}

	void init()
	{
		ServerInstance->Modules->AddService(cmd);
	}


	virtual ~ModuleDelUser()
	{
	}

	virtual Version GetVersion()
	{
		return Version("Provides /DELUSER functionality", VF_VENDOR);
	}
};

MODULE_INIT(ModuleDelUser)

