/* Core functions
 *
 * (C) 2003-2011 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

/*************************************************************************/

#include "module.h"

class CommandHelp : public Command
{
 public:
	CommandHelp(Module *creator) : Command(creator, "generic/help", 0)
	{
		this->SetDesc(_("Displays this list and give information about commands"));
		this->SetFlag(CFLAG_STRIP_CHANNEL);
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		FOREACH_MOD(I_OnPreHelp, OnPreHelp(source, params));
	
		User *u = source.u;
		BotInfo *bi = source.owner;

		if (params.empty())
		{
			for (command_map::iterator it = bi->commands.begin(), it_end = bi->commands.end(); it != it_end; ++it)
			{
				// Smaller command exists
				Anope::string cmd = myStrGetToken(it->first, ' ', 0);
				if (cmd != it->first && bi->commands.count(cmd))
					continue;

				service_reference<Command> c(it->second);
				if (!c)
					continue;
				if (!Config->HidePrivilegedCommands || c->permission.empty() || u->HasCommand(c->permission))
				{
					source.command = it->first;
					c->OnServHelp(source);
				}
			}
		}
		else
		{
			bool helped = false;
			for (unsigned max = params.size(); max > 0; --max)
			{
				Anope::string full_command;
				for (unsigned i = 0; i < max; ++i)
					full_command += " " + params[i];
				full_command.erase(full_command.begin());

				std::map<Anope::string, Anope::string>::iterator it = bi->commands.find(full_command);
				if (it == bi->commands.end())
					continue;

				service_reference<Command> c(it->second);
				if (!c)
					continue;

				if (Config->HidePrivilegedCommands && !c->permission.empty() && !u->HasCommand(c->permission))
					continue;

				const Anope::string &subcommand = params.size() > max ? params[max] : "";
				source.command = full_command;
				if (!c->OnHelp(source, subcommand))
					continue;

				helped = true;
				source.Reply(" ");

				/* Inform the user what permission is required to use the command */
				if (!c->permission.empty())
					source.Reply(_("Access to this command requires the permission \002%s\002 to be present in your opertype."), c->permission.c_str());
				if (!c->HasFlag(CFLAG_ALLOW_UNREGISTERED) && !u->IsIdentified())
					source.Reply( _("You need to be identified to use this command."));
				/* User doesn't have the proper permission to use this command */
				else if (!c->permission.empty() && !u->HasCommand(c->permission))
					source.Reply(_("You cannot use this command."));
				/* User can use this command */
				else
					source.Reply(_("You can use this command."));

				break;
			}

			if (helped == false)
				source.Reply(_("No help available for \002%s\002."), params[0].c_str());
		}
	
		FOREACH_MOD(I_OnPostHelp, OnPostHelp(source, params));

		return;
	}
};

class Help : public Module
{
	CommandHelp commandhelp;

 public:
	Help(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, CORE),
		commandhelp(this)
	{
		this->SetAuthor("Anope");

		ModuleManager::RegisterService(&commandhelp);
	}
};

MODULE_INIT(Help)
