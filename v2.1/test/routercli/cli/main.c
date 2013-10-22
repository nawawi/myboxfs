/* 
 * RouterCli 0.4 
 *
 * Copyright (c) 2004 Alexander Eremin <xyzyx@rambler.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../lib/ext.h"
#define HELPID (17)
char banner[100];
char enpass[50];
char string[200];
char nsname[50];
int cli_status;
int numtokens;
char host[40];
char dns[40];
char *hname[40];
char *arg;
char intf[5];
char **chargv;
char *ping_cmd[4];
char *trace_cmd[4];
char *telnet_cmd[2];
char *route_cmd[9];
char *ipadr_cmd[6];
char *acl_cmd[7];
char *rip_cmd[2];
char *cli_input;
char *wild[2];
char wc[20];
char aclstring[250];
char cmd[100];

int cli_ping PARAMS((char *));
int cli_trace PARAMS((char *));
int cli_help PARAMS((char *));
int cli_exit PARAMS((char *));
int cli_show PARAMS((char *));
int cli_enable PARAMS((char *));
int cli_hostname PARAMS((char *));
int cli_configure PARAMS((char *));
int cli_conf_line PARAMS((char *));
int cli_clear_route PARAMS ((char *));
int cli_iface PARAMS ((char *));
int cli_ip PARAMS ((char *));
int cli_route PARAMS ((char *));
int cli_ns PARAMS ((char *));
int cli_domain PARAMS ((char *));
int cli_up_iface PARAMS ((char *));
int cli_down_iface PARAMS ((char *));
int cli_ok_iface PARAMS ((char *));
int cli_write PARAMS ((char *));
int cli_password PARAMS ((char *));
int cli_tel PARAMS ((char *));
int cli_copy PARAMS ((char *));
int cli_banner PARAMS ((char *));
int cli_acl PARAMS ((char *));
int cli_acl_class PARAMS ((char *));
int cli_access PARAMS ((char *));
int cli_conf_no PARAMS ((char *));
int cli_no_acl PARAMS ((char *));
int cli_shell PARAMS ((char *));
int cli_router PARAMS ((char *));
int cli_network PARAMS ((char *));
int cli_clock PARAMS ((char *));

int ns=0;
int no_shut=0;
int versa=0;
  
typedef struct {
  char *name;                   
  rl_icpfunc_t *func;           
  char *name2;
  char *doc;
  char *doc2;
} CLI_COMMAND;

CLI_COMMAND commands[] = {
{"enable",cli_enable, NULL,"Turn on privileged commands",NULL},
{"help",cli_help, NULL,"Description of the interactive help system",NULL},
{"exit",cli_exit, NULL,"Exit from the EXEC",NULL},
{"ping",cli_ping,"ip",  "Send echo messages","IP echo"},
{"trace",cli_trace,"ip","Tracing route to destination","Traceroute destination address"},
{"show",cli_show, NULL,"Show running system information",NULL},
{"telnet",cli_tel,"WORD","Open a telnet connection","IP address or hostname of a remote system"},
{(char *)NULL,0,(char *)NULL,(char *)NULL }
};

typedef struct {
  char *name;
  rl_icpfunc_t *func;           
  char *name2;
  char *doc;
  char *doc2;
} CLI_COMMAND_PRIV;

CLI_COMMAND_PRIV commands_priv[] = {
{"configure",cli_configure,"terminal","Enter configuration mode","Configure from terminal"},
{"clock",cli_clock,"MMDDHHSSYYYY","Manage the system clock","Set the time and date"},
{"copy",cli_copy,"startup-config running-config","Copy configuration or image data","Copy from startup configuration"},
{"disable",cli_exit,NULL,"Turn off privileged commands",NULL},
{"help",cli_help, NULL,"Description of the interactive help system",NULL},
{"exit",cli_exit, NULL,"Exit from the EXEC",NULL},
{"ping",cli_ping,"ip",  "Send echo messages","IP echo"},
{"trace",cli_trace,"ip","Tracing route to destination","Traceroute destination address"},
{"show",cli_show, NULL,"Show running system information",NULL},
{"shell",cli_shell, NULL,"Execute system shell",NULL},
{"write",cli_write, "memory","Write running configuration to memory","Write to memory"},
{(char *)NULL,0,(char *)NULL,(char *)NULL }
};

typedef struct {
  char *name;
  rl_icpfunc_t *func;           
  char *name2;
  char *doc;
  char *doc2;
} CLI_COMMAND_CONF;

CLI_COMMAND_CONF commands_conf[] = {
{"access-list",cli_acl,NULL,"Add an access list entry",NULL},
{"banner",cli_banner,"login","Define a login banner","Set banner login"},
{"clear",cli_clear_route,"ip route A.B.C.D MASK A.B.C.D","Reset functions","Reset static routes"},
{"help",cli_help,NULL,"Description of the interactive help system",NULL},
{"hostname",cli_hostname,"WORD","Set system's network name","This system's network name"},
{"exit",cli_exit,NULL,"Exit from the EXEC",NULL},
{"line",cli_conf_line,"vty","Configure a terminal line","Virtual terminal"},
{"router",cli_router,"rip","Enable a routing process","Routing Information Protocol (RIP)"},
{"interface",cli_iface,"ethernet <0-5> (ethernet <0-5.0-5>)","Select an interface to configure","Ethernet interface number"},
{"ip",cli_ip,NULL,"Global IP configuration subcommands",NULL},
{"no",cli_conf_no,NULL,"Negate a command or set its default",NULL},
{"enable",cli_password,"password (secret)","Modify enable password parameters","Assign the privileged level password"},
{(char *)NULL,0,(char *)NULL,(char *)NULL }
};

typedef struct {
  char *name;
  rl_icpfunc_t *func;           
  char *name2;
  char *doc;
  char *doc2;
} CLI_COMMAND_RIP;

CLI_COMMAND_RIP commands_rip[] = {
{"access-list",cli_acl,NULL,"Add an access list entry",NULL},
{"banner",cli_banner,"STRING","Define a login banner","Set banner login"},
{"clear",cli_clear_route,"ip route A.B.C.D MASK A.B.C.D","Reset functions","Reset static routes"},
{"help",cli_help,NULL,"Description of the interactive help system",NULL},
{"hostname",cli_hostname,"WORD","Set system's network name","This system's network name"},
{"exit",cli_exit,NULL,"Exit from the EXEC",NULL},
{"line",cli_conf_line,"vty","Configure a terminal line","Virtual terminal"},
{"interface",cli_iface,"ethernet <0-5> {ethernet <0-5.0-5>}","Select an interface to configure","Ethernet interface number"},
{"ip",cli_ip,NULL,"Global IP configuration subcommands",NULL},
{"network",cli_network,"A.B.C.D A.B.C.D number {active|passive|external}","Enable routing on  an IP network","Network gw metric state"},
{"no",cli_conf_no,NULL,"Negate a command or set its default",NULL},
{"enable",cli_password,"password (secret)","Modify enable password parameters","Assign the privileged level password"},
{(char *)NULL,0,(char *)NULL,(char *)NULL }
};


typedef struct {
  char *name;
  rl_icpfunc_t *func;           
  char *name2;
  char *doc;
  char *doc2;
} CLI_COMMAND_LINE;

CLI_COMMAND_LINE commands_line[] = {
{"access-class",cli_access,NULL,"Filter connections based on an IP access-list",NULL},
{"help",cli_help,NULL,"Description of the interactive help system",NULL},
{"hostname",cli_hostname,"WORD","Set system's network name","This system's network name"},
{"exit",cli_exit,NULL,"Exit from the EXEC",NULL},
{"interface",cli_iface,"ethernet <0-5> (ethernet <0-5.0-5>)","Select an interface to configure","Ethernet interface number"},
{"ip",cli_ip,NULL,"Global IP configuration subcommands",NULL},
{"enable",cli_password,"password (secret)","Modify enable password parameters","Assign the privileged level password"},
{"no",cli_no_acl,NULL,"Negate a command or set its default",NULL},
{(char *)NULL,0,(char *)NULL,(char *)NULL }
};

typedef struct {
  char *name;
  rl_icpfunc_t *func;           
  char *name2;
  char *doc;
  char *doc2;
} CLI_COMMAND_IFACE;

CLI_COMMAND_IFACE commands_iface[] = {
{"help",cli_help,NULL,"Description of the interactive help system",NULL},
{"exit",cli_exit,NULL,"Exit from the EXEC",NULL},
{"ip",cli_up_iface,NULL,"Global IP configuration subcommands"},
{"no",cli_ok_iface,NULL,"Negate a command or set its default",NULL},
{"shutdown",cli_down_iface,NULL,"Shutdown the selected interface",NULL},
{(char *)NULL,0,(char *)NULL,(char *)NULL }
};


    typedef struct {
    char *name;
    char *doc;
} CLI_SHOW;    

CLI_SHOW show[] = {
{"version","System hardware and software status"},
{"clock","Display the system clock"},
{"history","Display the session command history"},
{(char *)NULL,(char *)NULL}
};

    typedef struct {
    char *name;
    char *doc;
} CLI_SHOW_PRIV;    

CLI_SHOW_PRIV show_priv[] = {
{"access-lists","List access-lists"},
{"arp","ARP table"},
{"version","System hardware and software status"},
{"clock","Display the system clock"},
{"history","Display the session command history"},
{"interfaces","Interface status and configuration"},
{"ip route","IP routing table"},
{"running-config","Current operating configuration"},
{"startup-config","Contents of startup configuration"},
{(char *)NULL,(char *)NULL}
};

    typedef struct {
    char *name;
    char *doc;
} CLI_SHOW_IP;    

CLI_SHOW_IP show_ip[] = {
{"default-gateway","Specify default gateway (if not routing IP)"},
{"domain-name","Define the default domain name"},
{"name-server","Specify address of name server to use"},
{"route","Establish static routes"},
{(char *)NULL,(char *)NULL}
};
    typedef struct {
    char *name;
    char *doc;
} CLI_SHOW_IP_IF;    

CLI_SHOW_IP_IF show_ip_if[] = {
{"access-group","Specify access control for packets"},
{"address","Set the IP address of an interface"},
{(char *)NULL,(char *)NULL}
};

    typedef struct {
    char *name;
    char *doc;
} CLI_CONF;    

CLI_CONF show_conf[] = {
{"terminal","Specify address of name server to use"},
{(char *)NULL,(char *)NULL}
};
    typedef struct {
    char *name;
    char *doc;
} CLI_COPY;    

CLI_COPY show_copy[] = {
{"startup-config running-config","Copy from startup configuration"},
{(char *)NULL,(char *)NULL}
};

    typedef struct {
    char *name;
    char *doc;
} CLI_FRW;    

CLI_FRW cli_frw[] = {
{"<1-99> {deny|permit} source [source-wildcard]","IP standard access list"},
{"<100-199> {deny|permit} {ip|tcp|udp} src src-wildcard dest dest-wildcard [operator port [port]]","IP extended access list"},
{(char *)NULL,(char *)NULL}
};

typedef struct {
    char *name;
    char *doc;
} CLI_NO_IFACE;    

CLI_NO_IFACE cli_no_iface[] = {
{"ip access-group","Specify access-control for packets"},
{"shutdown","Change state to up"},  
{(char *)NULL,(char *)NULL}
};

typedef struct {
    char *name;
    char *doc;
} CLI_NO_LINE;    

CLI_NO_LINE cli_no_line[] = {
{"access-class","Filter connections based on an IP access-list"},
{(char *)NULL,(char *)NULL}
};

typedef struct {
    char *name;
    char *doc;
} CLI_NO_CONF;    

CLI_NO_CONF cli_no_conf[] = {
{"access-list","Reset numbered access-list"},
{"router rip","Disable a routing process"},
{(char *)NULL,(char *)NULL}
};


char *cli_stripwhite ();
CLI_COMMAND *cli_search_command ();
CLI_COMMAND_PRIV *cli_search_command_priv ();
CLI_COMMAND_CONF *cli_search_command_conf ();
CLI_COMMAND_RIP *cli_search_command_rip ();
CLI_COMMAND_IFACE *cli_search_command_iface ();
CLI_COMMAND_LINE *cli_search_command_line ();

int done;

int main(int argc,char **argv)

{
	if(argc==2) 
	{
	    if(!(strcmp(argv[1], "-v")) || !(strcmp(argv[1], "--version"))){
		printf("RouterCLI (tm) 2 Software (Version %s) (c) 2004 Alexander Eremin <xyzyx@rambler.ru>\n",VERSION); exit(0);}
	    else if(!(strcmp(argv[1], "-h")) || !(strcmp(argv[1], "--help"))){
		printf("Usage:\n"); 
		printf("\t-s [--startup] Initialize the kept configuration and exit\n");
		printf("\t-v [--version] Show version\n");
		printf("\tWithout arguments - usual start\n");exit(0); }
	    else if(!(strcmp(argv[1], "-s")) || !(strcmp(argv[1], "--startup"))){
		cli_conf_init(); exit(0); }
	    else if(strchr(argv[1], '-')) { 
			printf("RouterCli: too few arguments\nTry `RouterCli --help`\n");exit(1);}
	    else {printf("RouterCli: too few arguments\nTry `RouterCli --help`\n");exit(1);}
	    }
	else if (argc==1)
	    cli_main();
	else
	    printf("RouterCli: too few arguments\nTry `RouterCli --help`\n");exit(1);
	return 0;
}       

int cli_main (void)
    
{
    char *s;
    gethostname(host,sizeof(host));
    printf("Router %s is now available\n\n",host);
    startup();
    if (banner)
	printf("%s\n\n",banner);
    cli_readline_init();        
    signal (SIGINT,SIG_IGN);
    signal (SIGQUIT,SIG_IGN);
    cli_status=1;
    for ( ; done == 0; )
    {
	gethostname(host,sizeof(host));
	if (cli_status==1)
	    strcat(host,"> ");
	else if (cli_status==2)
	    strcat(host,"# ");
	else if (cli_status==3)
	    strcat(host,"(config)# ");
	else if (cli_status==4)
	{
	    if (strchr(intf, ':')==0)
		strcat(host,"(config-if)# ");
	    else strcat(host,"(config-subif)# ");
	}  
	else if (cli_status==5)
	    strcat(host,"(config-line)# ");
	else if (cli_status==6)
	    strcat(host,"(config-router)# ");
	                                        
	cli_input=readline(host);
	if (!cli_input)
	break;
	s = cli_stripwhite (cli_input);
	if (*s)
	{
	  add_history (s);
	  cli_run_command (s);
	}
	free (cli_input);
    }
    exit (0);
}

int cli_run_command (cli_input)
    char *cli_input;
{
    register int i;
    CLI_COMMAND *command;
    CLI_COMMAND_PRIV *command_priv;
    CLI_COMMAND_CONF *command_conf;
    CLI_COMMAND_RIP  *command_rip;
    CLI_COMMAND_IFACE *command_iface;
    CLI_COMMAND_LINE *command_line;

    char *word;
    i = 0;
    while (cli_input[i] && whitespace (cli_input[i]))
	i++;
	word = cli_input + i;
    while (cli_input[i] && !whitespace (cli_input[i]))
	i++;
    if (cli_input[i])
	cli_input[i++] = '\0';
    if (cli_status==1)
    {   
	command = cli_search_command (word);
	if (!command)
	{
	    cli_telnet(word);
	    return (-1);
	}
	while (whitespace (cli_input[i]))
		i++;
	    word = cli_input + i;
	return ((*(command->func)) (word));
    }
    else if (cli_status==2)
    {   
	command_priv = cli_search_command_priv (word);
	if (!command_priv)
	{
	    cli_telnet(word);
	    return (-1);
	}
	while (whitespace (cli_input[i]))
		i++;
	    word = cli_input + i;
	return ((*(command_priv->func)) (word));
    }
    else if (cli_status==3)
    {   
	command_conf = cli_search_command_conf (word);
	if (!command_conf)
	{
	    fprintf (stderr, "%% Ambiguous command\r\n"); 
	    return (-1);
	}
	while (whitespace (cli_input[i]))
		i++;
	    word = cli_input + i;
	return ((*(command_conf->func)) (word));
    }
    else if (cli_status==4)
    {   
	command_iface = cli_search_command_iface (word);
	if (!command_iface)
	{
	    fprintf (stderr, "%% Ambiguous command\r\n"); 
	    return (-1);
	}
	while (whitespace (cli_input[i]))
		i++;
	    word = cli_input + i;
	return ((*(command_iface->func)) (word));
    }
    else if (cli_status==5)
    {   
	command_line = cli_search_command_line (word);
	if (!command_line)
	{
	    fprintf (stderr, "%% Ambiguous command\r\n"); 
	    return (-1);
	}
	while (whitespace (cli_input[i]))
		i++;
	    word = cli_input + i;
	return ((*(command_line->func)) (word));
    }
    else if (cli_status==6)
    {   
	command_rip = cli_search_command_rip (word);
	if (!command_rip)
	{
	    fprintf (stderr, "%% Ambiguous command\r\n"); 
	    return (-1);
	}
	while (whitespace (cli_input[i]))
		i++;
	    word = cli_input + i;
	return ((*(command_rip->func)) (word));
    }
    return (0);
}

CLI_COMMAND *cli_search_command (name)
    char *name;
{
    register int i;
    for (i = 0; commands[i].name; i++)
	if (strstr(commands[i].name,name) && (strlen(name)>=2))
	return (&commands[i]);
    return ((CLI_COMMAND *)NULL);
}

CLI_COMMAND_PRIV *cli_search_command_priv (name)
    char *name;
{
    register int i;
    for (i = 0; commands_priv[i].name; i++)
	if (strstr(commands_priv[i].name,name) && (strlen(name)>=2))
	return (&commands_priv[i]);
    return ((CLI_COMMAND_PRIV *)NULL);
}

CLI_COMMAND_CONF *cli_search_command_conf (name)
    char *name;
{
    register int i;
    for (i = 0; commands_conf[i].name; i++)
	if (strstr(commands_conf[i].name,name) && (strlen(name)>=2))
	return (&commands_conf[i]);
    return ((CLI_COMMAND_CONF *)NULL);
}

CLI_COMMAND_RIP *cli_search_command_rip (name)
    char *name;
{
    register int i;
    for (i = 0; commands_rip[i].name; i++)
	if (strstr(commands_rip[i].name,name) && (strlen(name)>=2))
	return (&commands_rip[i]);
    return ((CLI_COMMAND_RIP *)NULL);
}

CLI_COMMAND_LINE *cli_search_command_line (name)
    char *name;
{
    register int i;
    for (i = 0; commands_line[i].name; i++)
	if (strstr(commands_line[i].name,name) && (strlen(name)>=2))
	return (&commands_line[i]);
    return ((CLI_COMMAND_LINE *)NULL);
}

CLI_COMMAND_IFACE *cli_search_command_iface (name)
    char *name;
{
    register int i;
    for (i = 0; commands_iface[i].name; i++)
	if (strstr(commands_iface[i].name,name) && (strlen(name)>=2))
	return (&commands_iface[i]);
    return ((CLI_COMMAND_IFACE *)NULL);
}


char *cli_stripwhite (string)
     char *string;
{
  register char *s, *t;
  for (s = string; whitespace (*s); s++)
    ;
  if (*s == 0)
    return (s);
  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';
  return s;
}
char *command_generator PARAMS((const char *, int));

void cli_readline_init ()
{
    rl_terminal_name = getenv("TERM"); 
    setenv("TERM", rl_terminal_name, 0);
    rl_bind_key ('?', (Function *)vopros);
    rl_attempted_completion_function =  (CPPFunction *)word_completion;
    rl_completion_append_character = '\0';
}
		      
void vopros ()
{
  cli_input=makeargv(rl_line_buffer, BLANK_STRING, &chargv);
    /* ? */
    if (cli_input == NULL)
    {
	printf ("\n");
	cli_help_vopros();
	rl_delete_text (0, rl_end);
	rl_point = rl_end = 0;
	rl_on_new_line ();
    }
    else if (rl_end && isspace ((int) rl_line_buffer[rl_end - 1]))
    {
	printf ("\n");
	cli_chomp(rl_line_buffer);
	strcpy(cmd,rl_line_buffer);
	
	if (strcmp(rl_line_buffer,"show")==0 || strcmp(rl_line_buffer,"sh")==0)  
	    cli_show_vopros();
	else if (strcmp(rl_line_buffer,"access-list")==0 && (cli_status==3 || cli_status==6))
	    cli_acl_help();
	else if (strcmp(rl_line_buffer,"ip")==0 && (cli_status==3 || cli_status==4 || cli_status==6))
	    cli_show_ip();
	else if (strcmp(rl_line_buffer,"no")==0 && (cli_status==3 || cli_status==4 || cli_status==5 || cli_status==6))
	    cli_show_no();
	else  
	    cli_help_second(rl_line_buffer);
	rl_delete_text (0, rl_end);
	rl_point = rl_end = 0;
	rl_on_new_line ();
	rl_insert_text (cmd);
	rl_insert_text (" ");
    } 
}
	     
char *command_generator (text, state)
    const char *text;
    int state;
{
    static int len;
    static char *name = NULL;
    static int index = 0;
    if (! state)
    {
	index = 0;
	len = strlen (text);
    }
    cli_input= makeargv(rl_line_buffer, BLANK_STRING, &chargv);      
    if (cli_input == NULL)
	return NULL;
    if (cli_status==1)
    {
	if (strcmp(chargv[0],"show")==0 || strcmp(chargv[0],"sh")==0)
	{
	    while  ((name = show[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	} 
	else if (!chargv[1])
	{       
	    while  ((name = commands[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else return NULL;
    }   
    else if (cli_status==2)
    {
	if (strcmp(chargv[0],"show")==0 || strcmp(chargv[0],"sh")==0) 
	{
	    while  ((name = show_priv[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	} 
	else if (strcmp(chargv[0],"config")==0 || strcmp(chargv[0],"configure")==0) 
	{
	    while  ((name = show_conf[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }   
	}
	else if (strcmp(chargv[0],"copy")==0)
	{
	    while  ((name = show_copy[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }   
	}
	else if (!chargv[1])
	{
	    while  ((name = commands_priv[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else return NULL;
    }
    else if (cli_status==3)
    {
	if (strcmp(chargv[0],"access-list")==0)
	{
	    while  ((name = cli_frw[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else if (strcmp(chargv[0],"no")==0)
	{
	    while  ((name = cli_no_conf[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else if (strcmp(chargv[0],"ip")==0)
	{
	    while  ((name = show_ip[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else if (!chargv[1])
	{
	    while  ((name = commands_conf[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else return NULL;
    }
    else if (cli_status==4)
    {
	if (strcmp(chargv[0],"ip")==0)
	{
	    while  ((name = show_ip_if[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else if (strcmp(chargv[0],"no")==0)
	{
	    while  ((name = cli_no_iface[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else if (!chargv[1])
	{
	    while  ((name = commands_iface[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else return NULL;
    }
    else if (cli_status==5)
    {
	if (strcmp(chargv[0],"ip")==0)
	{
	    while  ((name = show_ip[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else if (strcmp(chargv[0],"no")==0)
	{
	    while  ((name = cli_no_line[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	
	else if (!chargv[1])
	{
	    while  ((name = commands_line[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else return NULL;
    }
    else if (cli_status==6)
    {
	if (strcmp(chargv[0],"access-list")==0)
	{
	    while  ((name = cli_frw[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else if (strcmp(chargv[0],"ip")==0)
	{
	    while  ((name = show_ip[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else if (strcmp(chargv[0],"no")==0)
	{
	    while  ((name = cli_no_conf[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	
	else if (!chargv[1])
	{
	    while  ((name = commands_rip[index].name))
	    {
		index++;
		if (strncmp (name, text, len) == 0)
		return (dupstr(name));
	    }
	}
	else return NULL;
    }
    return NULL;
}										      


char **word_completion (char *text, int start, int end)
{
    char **matches;
    matches = (char **)NULL;
    matches = rl_completion_matches (text, command_generator);
    rl_attempted_completion_over = 1;
    
/*    
    rl_beg_of_line(0,0);
    rl_delete_text(0, strlen(rl_line_buffer)+1);
    rl_redisplay();
*/
    return (matches);
}

int cli_help (name)
char *name;
{
    register int i;
    int found = 0;
    printf("Exec commands:\n");
    if (cli_status==1)
    {
	for (i = 0; commands[i].name; i++)
	{
	    if (!*name || (strcmp (name, commands[i].name) == 0))
	    {
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands[i].name, commands[i].doc);
		found++;
	    }
	}
    }
    else if (cli_status==2)
    {
	for (i = 0; commands_priv[i].name; i++)
	{
	    if (!*name || (strcmp (name, commands_priv[i].name) == 0))
	    {
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_priv[i].name, commands_priv[i].doc);
		found++;
	    }
	}
    }
    else if (cli_status==3)
    {
	for (i = 0; commands_conf[i].name; i++)
	{
	    if (!*name || (strcmp (name, commands_conf[i].name) == 0))
	    {
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_conf[i].name, commands_conf[i].doc);
		found++;
	    }
	}
    }
    else if (cli_status==4)
    {
	for (i = 0; commands_iface[i].name; i++)
	{
	    if (!*name || (strcmp (name, commands_iface[i].name) == 0))
	    {
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_iface[i].name, commands_iface[i].doc);
		found++;
	    }
	}
    }
    else if (cli_status==5)
    {
	for (i = 0; commands_line[i].name; i++)
	{
	    if (!*name || (strcmp (name, commands_line[i].name) == 0))
	    {
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_line[i].name, commands_line[i].doc);
		found++;
	    }
	}
    }
    else if (cli_status==6)
    {
	for (i = 0; commands_rip[i].name; i++)
	{
	    if (!*name || (strcmp (name, commands_rip[i].name) == 0))
	    {
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_rip[i].name, commands_rip[i].doc);
		found++;
	    }
	}
    }
    if (!found)
	printf ("%% Ambiguous command: %s\r\n", name);
    return(0);
}

void cli_help_vopros()
{
    register int i;
    printf("Exec commands:\n");
    if (cli_status==1)
    {
	for (i = 0; commands[i].name; i++)
	    printf (" %-*s\t\t%s\r\n",(int)HELPID,commands[i].name, commands[i].doc);
    }
    else if (cli_status==2)
    {
	for (i = 0; commands_priv[i].name; i++)
	    printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_priv[i].name, commands_priv[i].doc);
    }
    else if (cli_status==3)
    {
	for (i = 0; commands_conf[i].name; i++)
	    printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_conf[i].name, commands_conf[i].doc);
    }
    else if (cli_status==4)
    {
	for (i = 0; commands_iface[i].name; i++)
	    printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_iface[i].name, commands_iface[i].doc);
    }
    else if (cli_status==5)
    {
	for (i = 0; commands_line[i].name; i++)
	    printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_line[i].name, commands_line[i].doc);
    }
    else if (cli_status==6)
    {
	for (i = 0; commands_rip[i].name; i++)
	    printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_rip[i].name, commands_rip[i].doc);
    }
    
}

void cli_help_second (char *name)
{
  register int i;
    int found=0;
    if (cli_status==1)
    {
	for (i = 0; commands[i].name; i++)
	{
	    if (strcmp(name,commands[i].name) == 0 && commands[i].name2!=NULL)
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands[i].name2, commands[i].doc2);
	    found++;
	}
    }
    else if (cli_status==2)
    {
	for (i = 0; commands_priv[i].name; i++)
	{
	    if (strcmp(name,commands_priv[i].name) == 0 && commands_priv[i].name2!=NULL)
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_priv[i].name2, commands_priv[i].doc2);
	    found++;
	}
    }   
    else if (cli_status==3)
    {
	for (i = 0; commands_conf[i].name; i++)
	{
	    if (strcmp(name,commands_conf[i].name) == 0 && commands_conf[i].name2!=NULL)
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_conf[i].name2, commands_conf[i].doc2);
	    found++;
	}
    }
    else if (cli_status==4)
    {
	for (i = 0; commands_iface[i].name; i++)
	{
	    if (strcmp(name,commands_iface[i].name) == 0 && commands_iface[i].name2!=NULL)
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_iface[i].name2, commands_iface[i].doc2);
	    found++;
	}
    }
    else if (cli_status==5)
    {
	for (i = 0; commands_line[i].name; i++)
	{
	    if (strcmp(name,commands_line[i].name) == 0 && commands_line[i].name2!=NULL)
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_line[i].name2, commands_line[i].doc2);
	    found++;
	}
    }
    else if (cli_status==6)
    {
	for (i = 0; commands_rip[i].name; i++)
	{
	    if (strcmp(name,commands_rip[i].name) == 0 && commands_rip[i].name2!=NULL)
		printf (" %-*s\t\t%s\r\n",(int)HELPID,commands_rip[i].name2, commands_rip[i].doc2);
	    found++;
	}
    }
    	
    if (!found)
    printf ("\n");
}

int cli_exit (name)
     char *name;
{
    if (cli_status==1)
	done = 1;
    else if (cli_status==2)
	cli_status=1;
    else if (cli_status==3 || cli_status==5 || cli_status==6)
    {
	tm();printf(" %%SYS-5-CONFIG_I:Configured from console by console\r\n");
	cli_status=2;
    }
    else if (cli_status==4)
	cli_status=3;
    return(0);
}

int cli_need_arg (caller, name)
    char *caller, *name;
{
    if (!name || !*name)
    {
      fprintf (stderr, "%% Incomplete command\r\n");
      return (0);
    }
    return (1);
}

int cli_show(name)
char *name;
{
    int returncode = 0;
    if (!cli_need_arg ("show", name))
    	returncode = 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if (strcmp(chargv[0],"clock")==0 && (cli_status==1 || cli_status==2)) 
		showtime();
	    else if (strcmp(chargv[0],"history")==0 && (cli_status==1 || cli_status==2)) 
		showhist();
	    else if (strcmp(chargv[0],"version")==0 && (cli_status==1 || cli_status==2)) 
		showversion();
	    else if (strcmp(chargv[0],"ip")==0  && cli_status==2) 
		printf ("%% Incomplete command\r\n");
	    else if (strcmp(chargv[0],"arp")==0 && cli_status==2) 
		 cli_arp_show();
	    else if (strcmp(chargv[0],"access-lists")==0 && cli_status==2) 
		 cli_acl_show();
	    else if ((strcmp(chargv[0],"running-config")==0 || strcmp(chargv[0],"run")==0) && cli_status==2) 
		 cli_show_run();
	    else if ((strcmp(chargv[0],"startup-config")==0 || strcmp(chargv[0],"star")==0) && cli_status==2) 
		 cli_show_start();
	    else if ((strcmp(chargv[0],"interfaces")==0 || strcmp(chargv[0],"int")==0) && cli_status==2) 
		ethshowall();
	    else printf("%% Invalid input detected\r\n");       
	}
	else if (numtokens==2)
	{
	    if (strcmp(chargv[0],"ip")==0 && strcmp(chargv[1],"route")==0 && cli_status==2) 
		showroute();
	    else if ((strcmp(chargv[0],"interfaces")==0 || strcmp(chargv[0],"int")==0) && chargv[1] && cli_status==2) 
		ethshow(chargv[1]);
	    else printf("%% Invalid input detected\r\n");
	}
	else printf ("%% Invalid input detected\r\n");
    } 
    return returncode;
}


void cli_show_vopros()
{
    register int i;
    if (cli_status==1)
    {
	for (i=0;show[i].name;i++)
	    printf(" %-*s\t\t%s\r\n", (int)HELPID,show[i].name, show[i].doc);
    }
    else if (cli_status==2)
    {
	for (i=0;show_priv[i].name;i++)
	    printf(" %-*s\t\t%s\r\n", (int)HELPID,show_priv[i].name, show_priv[i].doc);
    }
}

void cli_show_ip()
{
    register int i;
	if (cli_status==3 || cli_status==6)
	{
	    for (i=0;show_ip[i].name;i++)
		printf(" %-*s\t\t%s\r\n", (int)HELPID,show_ip[i].name, show_ip[i].doc);
	}
	else if (cli_status==4)
	{
	    for (i=0;show_ip_if[i].name;i++)
		printf(" %-*s\t\t%s\r\n", (int)HELPID,show_ip_if[i].name, show_ip_if[i].doc);
	}
}
void cli_show_no()
{
    register int i;
		
	if (cli_status==3 || cli_status==6)
	{
	    for (i=0;cli_no_conf[i].name;i++)
		printf(" %-*s\t\t%s\r\n", (int)HELPID,cli_no_conf[i].name, cli_no_conf[i].doc);
	}

	if (cli_status==4)
	{
	    for (i=0;cli_no_iface[i].name;i++)
		printf(" %-*s\t\t%s\r\n", (int)HELPID,cli_no_iface[i].name, cli_no_iface[i].doc);
	}
	else if (cli_status==5)
	{
	    for (i=0;cli_no_line[i].name;i++)
		printf(" %-*s\t\t%s\r\n", (int)HELPID,cli_no_line[i].name, cli_no_line[i].doc);
	}
}

void cli_acl_help()
{
    register int i;
	for (i=0;cli_frw[i].name;i++)
	    printf(" %-*s\t\t%s\r\n", (int)HELPID,cli_frw[i].name, cli_frw[i].doc);
}

int cli_tel(name)
char *name;
{
    if (!cli_need_arg ("telnet", name))
	     return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	    cli_telnet(chargv[0]);
	else    
	    printf("%% Invalid input detected\r\n");                         
    }
    return 0;
}

int cli_ping (name)
     char *name;
{
    int pid, status;
    char ip[40];
    if (!cli_need_arg ("ping", name))
	return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1) 
	{
	    strcpy(ip,chargv[0]);
	    ping_cmd[0]=NULL;
	    ping_cmd[1]="-c";
	    ping_cmd[2]="5";
	    ping_cmd[3]=ip;
	    if ((pid = fork()) < 0) 
	    {
		perror("fork");
		return(-1);
	    }
	    if (pid == 0) 
	    {
		ping_main(4,ping_cmd);
		return(0);
	    }
	    while (wait(&status) != pid);
	}                             
	else printf("%% Invalid input detected\r\n");
    }
    return (0);
}

int cli_trace (name)
     char *name;
{
    int pid, status;
    char ip[40];
    if (!cli_need_arg ("trace", name))
    return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1) 
	{
	    strcpy(ip,chargv[0]);
	    trace_cmd[0]=NULL;
	    trace_cmd[1]="-w 2";
	    trace_cmd[2]="-m 15";
	    trace_cmd[3]=ip;
	    if ((pid = fork()) < 0) 
	    {
		perror("fork");
		return(-1);
	    }
	    if (pid == 0) 
	    {
		trace_main(4,trace_cmd);
		return(0);
	    }
	    while (wait(&status) != pid);
	}                             
	else printf("%% Invalid input detected\r\n");
    }
    return (0);
}

int cli_telnet(name)
    char *name;
{
    int pid, status, returncode = 0;
    strcpy(dns,"255.255.255.255");
    resolve();
    printf("Translating ""%s""... domain server (%s)\n",name,dns);
    telnet_cmd[0]=NULL;
    telnet_cmd[1]=name;
    if ((pid = fork()) < 0) 
    {
	perror("fork");
	returncode = -1;
    }
    else if (pid == 0) 
    {
	telnet_main(2,telnet_cmd);
	returncode = 0;
    }
    while (wait(&status) != pid);
    return returncode;
}                             

int cli_hostname(name)
    char *name;
{
    int returncode = 0;
    if (!cli_need_arg ("hostname", name))
       returncode = 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1) 
	{
	    if(strlen(chargv[0]) < 40)
	    { 
		hname[0]=NULL;
		hname[1]=chargv[0];
		hostname_main(2,hname);
	    }
	    else printf ("%% Invalid input detected\r\n");
	}
	else if (numtokens>1)
	printf ("%% Invalid input detected\r\n");
    }       
    return returncode;
}    

    
int cli_enable (name)
char *name;
{
    if (check_secret()==0)
	cli_status=2;
    return(0);
}

int cli_configure (name)
char *name;
{
    int returncode = 0;
    if (!cli_need_arg ("configure", name))
       returncode = 1;
    else if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if (strcmp(chargv[0],"terminal")==0 || strcmp(chargv[0],"term")==0 || strcmp(chargv[0],"t")==0)
		cli_status=3;
	    else printf ("%% Invalid input detected\r\n");
	}
	else if (numtokens>1)
	printf ("%% Invalid input detected\r\n");
    }
    return returncode;
}
int cli_conf_line (name)
char *name;
{
    int returncode = 0;
    if (!cli_need_arg ("line", name))
       returncode = 1;
    else if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if (strcmp(chargv[0],"vty")==0)
		cli_status=5;
	    else printf ("%% Invalid input detected\r\n");
	}
	else if (numtokens>1)
	printf ("%% Invalid input detected\r\n");
    }
    return returncode;
}

void cli_eth_set (name)
char *name;
{
    cli_status=4;
}

int cli_clear_route (name)
char *name;
{
    int returncode = 0;
    if (!cli_need_arg ("clear", name))
	returncode = 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if(strcmp(chargv[0],"ip")==0)
		printf ("%% Incompete command\r\n");
	    else printf ("%% Invalid input detected\r\n");
	}       
	else if (numtokens==2)
	{
	    if(strcmp(chargv[0],"ip")==0)
	    { 
		if(strcmp(chargv[1],"route")==0)
		    printf("%% Incomplete command\r\n");
		else printf("%% Invalid input detected\r\n");
	    }    
	    else printf("%% Invalid input detected\r\n");
	}       
	else if (numtokens==5)
	{  
	    if(strcmp(chargv[0],"ip")==0 && strcmp(chargv[1],"route")==0)
	    {
		if(ipcheck(chargv[2])==0 && ipcheck(chargv[3])==0 && ipcheck(chargv[4])==0)
		{
		    route_cmd[0]=NULL;
		    route_cmd[1]="del";
		    route_cmd[2]="-net";
		    route_cmd[3]=chargv[2];
		    route_cmd[4]="netmask";
		    route_cmd[5]=chargv[3];
		    route_cmd[6]="gw";
		    route_cmd[7]=chargv[4];
		    route_cmd[8]=NULL;
		    route_main(9,route_cmd);
		}
		else printf("%% Invalid input detected\r\n");
	    } 
	    else printf("%% Invalid input detected\r\n");
	}     
	else if (numtokens>5)
	    printf("%% Invalid input detected\r\n");
	else printf("%% Incompete command\r\n");
    }
    return returncode;
}

int cli_ip (name)
char *name;
{
    if (!cli_need_arg ("ip", name))
	return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (strcmp(chargv[0],"name-server")==0)
	    cli_ns(name);
	else if (strcmp(chargv[0],"domain-name")==0)
	    cli_domain(name);
	else if (strcmp(chargv[0],"default-gateway")==0)
	    cli_route_def(name);
	else if (strcmp(chargv[0],"route")==0)
	    cli_route(name);
	else printf ("%% Ambiguous command: %s\r\n", chargv[0]);
    }
    return (0);
}

int cli_domain(name)
char *name;
{
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==2)    
	{
	    if (strlen(chargv[1])<40)
	    {
		strcpy(nsname,chargv[1]);
	    }    
	    else  printf("%% Invalid input detected\r\n");
	}
	else if (numtokens>2)
	    printf("%% Invalid input detected\r\n");
	else printf ("%% Incomplete command\r\n");
    }
    return (0);
}

int cli_ns(name)
char *name;
{
    char nsaddr[15];
    FILE *fres;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==2)    
	{
	    if (ipcheck(chargv[1])==0)
	    {
		if (!ns)
		{
		    strcpy(nsaddr,chargv[1]);
		    fres = fopen ("/etc/resolv.conf", "w"); 
		    if (fres==NULL) {
			 fprintf (stderr,"%% Error opening config!\r\n");
			 return(-1);
		    } else {
			    if (nsname)
				fprintf (fres,"search %s\n",nsname);
			fprintf (fres,"nameserver %s\n",nsaddr);
			ns=1;
			fclose (fres); 
		    }
		}
		else if (ns==1)
		{                                                            
		    strcpy(nsaddr,chargv[1]);
		    fres = fopen ("/etc/resolv.conf", "a+");
		    if (fres==NULL) {
			 fprintf (stderr,"%% Error opening config!\r\n");
			 return(-1);
		    }
		    else {
			fprintf (fres,"nameserver %s\n",nsaddr);
			ns=0;
			fclose (fres); 
		    }
		}
	    }    
	    else  printf("%% Invalid input detected\r\n");
	}
	else if (numtokens>2)
	    printf("%% Invalid input detected\r\n");
	else printf ("%% Incomplete command\r\n");
    }
    return (0);
}

int cli_route(name)
char *name;
{
     if (makeargv(name, BLANK_STRING, &chargv) > 0)
     {
	if (numtokens==2)
	{
	    if (ipcheck(chargv[1])<0)
		    printf ("%% Invalid input detected\r\n");
	    else printf ("%% Incomplete command\r\n");
	}    
	else if (numtokens==3)
	{
	    if(ipcheck(chargv[1])<0 || ipcheck(chargv[2])<0)
		printf ("%% Invalid input detected\r\n");
	    else printf ("%% Incomplete command\r\n");
	}
	else if (numtokens==4)
	{
	    if(ipcheck(chargv[1])==0 && ipcheck(chargv[2])==0 && ipcheck(chargv[3])==0)
	    {
		route_cmd[0]=NULL;
		route_cmd[1]="add";
		route_cmd[2]="-net";
		route_cmd[3]=chargv[1];
		route_cmd[4]="netmask";
		route_cmd[5]=chargv[2];
		route_cmd[6]="gw";
		route_cmd[7]=chargv[3];
		route_cmd[8]=NULL;
		route_main(9,route_cmd);
	    }
	    else printf("%% Invalid input detected\r\n");
	} 
	else if (numtokens>4)
	    printf("%% Invalid input detected\r\n");
	else printf ("%% Incomplete command\r\n");
    }
    return (0);
}

int cli_route_def(name)
	char *name;
{
     if (makeargv(name, BLANK_STRING, &chargv) > 0)
     {
	if (numtokens==2)
	{
	    if(ipcheck(chargv[1])==0)
	    {
		route_cmd[0]=NULL;
		route_cmd[1]="add";
		route_cmd[2]="-net";
		route_cmd[3]="0.0.0.0";
		route_cmd[4]="gw";
		route_cmd[5]=chargv[1];
		route_cmd[6]=NULL;
		route_main(7,&route_cmd[0]);
	    }
	    else printf("%% Invalid input detected\r\n");
	} 
	else if (numtokens>2)
	    printf("%% Invalid input detected\r\n");
	else printf ("%% Incomplete command\r\n");
    }
    return (0);
}

int cli_iface (name)
char *name;
{
   int returncode = 0;
   if (!cli_need_arg ("interface", name))
       returncode = 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if (intverify(chargv[0])==0) 
		{
		    strcpy(intf,chargv[0]);
		    cli_eth_set(intf);
		}    
	    else printf("%% Invalid interface\r\n"); 
	}
	else if (numtokens>1)
	    printf ("%% Invalid input detected\r\n");
    }
    return returncode;
}

int cli_up_iface(name)
char *name;
{
    int returncode = 0;
    if (!cli_need_arg ("ip", name))
	returncode = 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (strcmp(chargv[0],"access-group")==0)
	    cli_access(name);
	else if (strcmp(chargv[0],"address")==0)
	    cli_addr(name);
	else printf ("%% Invalid input detected\r\n");
    }	
    return 0;
}

int cli_addr(name)
char *name;
{
    int returncode = 0;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if (strcmp(chargv[0],"address")==0)
		printf("%% Incomplete command\r\n");
	    else printf ("%% Invalid input detected\r\n");
	}       
	else if (numtokens==2)
	{
	    if (ipcheck(chargv[1])<0)
		printf ("%% Invalid input detected\r\n");
	    else printf("%% Incomplete command\r\n");
	}
	else if (numtokens==3)
	{
	    if (ipcheck(chargv[1])==0 && ipcheck(chargv[2])==0)
	    {
		ipadr_cmd[0]=NULL;
		ipadr_cmd[1]=intf;
		ipadr_cmd[2]=chargv[1];
		ipadr_cmd[3]="netmask";
		ipadr_cmd[4]=chargv[2];     
		ipadr_cmd[5]="up";
		no_shut=1;
	    }
	    else  printf ("%% Invalid input detected\r\n");
	}
	else if (numtokens>3)
	    printf("%% Invalid input detected\r\n");
    
    }
    return returncode;
}       


       


int cli_down_iface(name)
char *name;
{
    int returncode = 0;
    if (!name || !*name)
    {
	ipadr_cmd[0]=NULL;
	ipadr_cmd[1]=intf;
	ipadr_cmd[2]="down";
	ifcon(3,&ipadr_cmd[0]);
	printf ("%%LINEPROTO-5-UPDOWN:Line protocol on Interface %s, changed state to down\n",intf);
	printf ("%%LINK-3-UPDOWN:Interface %s, changed state to down\n",intf);
	
    }
    else printf("%% Invalid input detected\r\n");
    return returncode;
}

int cli_ok_iface(name)
char *name;
{
    int returncode = 0;
    if (!cli_need_arg ("no", name))
	returncode = 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if(strcmp(chargv[0],"shut")==0 || strcmp(chargv[0],"shutdown")==0 )
	    {
		if (no_shut) 
		ifcon(6,&ipadr_cmd[0]);
		printf ("%%LINEPROTO-5-UPDOWN:Line protocol on Interface %s, changed state to up\n",intf);
		printf ("%%LINK-3-UPDOWN:Interface %s, changed state to up\n",intf);
		
	    }
	    else if(strcmp(chargv[0],"ip")==0)
		printf("%% Incomplete command\r\n");
	    else printf("%% Invalid input detected\r\n");
	}
	else if (numtokens==2)
	{
	    if(strcmp(chargv[0],"ip")==0 && strcmp(chargv[1],"access-group")==0)
	    {	noacl();
		unlink("/etc/routercli/aclist");
	    }
	    else
		printf("%% Invalid input detected\r\n");
	}
	else if (numtokens>2)
	     printf("%% Invalid input detected\r\n");
	     
    }
    return returncode;
}    


int cli_show_start(void)
{
    FILE* conf ;
    char c;
    conf = fopen( "/etc/routercli/cli.conf", "r" ); 
    if (conf==NULL) { 
	fprintf (stderr,"%% Non-volatile configuration has been set up\r\n");
	return (-1);
    }
    else 
    {
	c = fgetc(conf);
	while(c!=EOF)
	{
	    putchar(c);
	    c =fgetc(conf);
	}
    }
    fclose(conf);
    return 0;
}

int cli_conf_init(void)
{
    FILE* confin = NULL;
    FILE* fres = NULL;
    char line[MAX_LINE_LEN + 1] ;
    int pid,status;
    int i=0;
    int ns=0;
    char intf_conf[5]=" ";
    confin = fopen( "/etc/routercli/cli.conf", "r" ); 
    if (confin==NULL) { 
	fprintf (stderr,"%% Error opening startup-config\r\n");
	return (-1);
    }
    else 
    {      
	while ( fgets( line, MAX_LINE_LEN, confin) != NULL )
	{
	    if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;

	    if (makeargv(line, BLANK_STRING, &chargv) > 0) 
	    {
		if(chargv[0]!= NULL && strcmp(chargv[0],"!")!=0)
		{ 
		    if (strcmp(chargv[0],"hostname")==0)
		    { 
			hname[0]=NULL;
			hname[1]=chargv[1];
			if (chargv[1])
			    hostname_main(3,&hname[0]);
		    }   
		    if (strcmp(chargv[0],"banner")==0 && strcmp(chargv[1],"login")==0)
		    {       
			    for (i = 2; i < numtokens; i++)
				printf("%s ",chargv[i]);
			    printf ("\n\n");
		    }				 
		    if (strcmp(chargv[0],"enable")==0 && strcmp(chargv[1],"secret")==0)
		    {       
			if (chargv[2])
			    strcpy(enpass,chargv[2]);
		    }        
		    if (strcmp(chargv[0],"ip")==0 && strcmp(chargv[1],"domain-name")==0)
		    {
			if (chargv[2])
			    strcpy(nsname,chargv[2]);
		    }		
		    if (strcmp(chargv[0],"ip")==0 && strcmp(chargv[1],"name-server")==0)
		    {
			if (chargv[2])
			{
			    if (!ns) 
				fres = fopen ("/etc/resolv.conf", "w");
			    else if (ns==1)
				fres = fopen ("/etc/resolv.conf", "a+");
			    if (fres==NULL) { 
				fprintf (stderr,"%% Error getting nameservers\r\n");
			    } else {
				if ((!ns) && (nsname))
				    fprintf (fres,"search %s\n",nsname);
				fprintf (fres,"nameserver %s\n",chargv[2]);
				ns=1;
				fclose (fres);
			    }
			}
		    }
		    if (strcmp(chargv[0],"interface")==0)
			strcpy(intf_conf,chargv[1]);
		    if (strcmp(chargv[0],"ip")==0 && strcmp(chargv[1],"address")==0)
		    {
			 
			 ipadr_cmd[0]=NULL;
			 ipadr_cmd[1]=intf_conf;
			 ipadr_cmd[2]=chargv[2];
			 ipadr_cmd[3]="netmask";
			 ipadr_cmd[4]=chargv[3];
			 ipadr_cmd[5]="up";
			 if (intf_conf && chargv[2] && chargv[3])
			    ifcon(6,&ipadr_cmd[0]);
		    }   
		    if (strcmp(chargv[0],"ip")==0  && strcmp(chargv[1],"route")==0)
		    {
			route_cmd[0]=NULL;
			route_cmd[1]="add";
			route_cmd[2]="-net";
			route_cmd[3]=chargv[2];
			route_cmd[4]="netmask";
			route_cmd[5]=chargv[3];
			route_cmd[6]="gw";
			route_cmd[7]=chargv[4];
			route_cmd[8]=NULL;
			if (chargv[2] && chargv[3] && chargv[4])
			    route_main(9,&route_cmd[0]);
		    }
		    if (strcmp(chargv[0],"ip")==0  && strcmp(chargv[1],"default-gateway")==0)
		    {
			route_cmd[0]=NULL;
			route_cmd[1]="add";
			route_cmd[2]="-net";
			route_cmd[3]="0.0.0.0";
			route_cmd[4]="gw";
			route_cmd[5]=chargv[2];
			route_cmd[6]=NULL;
			if (chargv[2])
			    route_main(7,&route_cmd[0]);	
		    }
		    if(strcmp(chargv[0],"router")==0  && strcmp(chargv[1],"rip")==0)
		    {
		    	if ((pid = fork()) < 0) 
			{
			    perror("fork");
			    return(-1);
			}
			if (pid == 0) 
			{
			    rip_cmd[0]=NULL;
			    rip_cmd[1]="-s";
			    ripd(2,&rip_cmd[0]);
			}
			while (wait(&status) != pid);
		    }
		    // acl
		    if(strcmp(chargv[0],"access-list")==0  && atoi(chargv[1])>0 && atoi(chargv[1])<=99)
		    {
			if (numtokens==6)
			{
		    	    if (strcmp(chargv[3],"any")==0)
		    	    {
				acl_cmd[0]=chargv[2];
				acl_cmd[1]="any";
				acl_cmd[2]="0";
				acl_cmd[3]=chargv[5];
				acl_cmd[4]=chargv[4];
			    	acl_cmd[5]=" ";
				acl_stand(6,&acl_cmd[0]);
			    }
			}	
			else if (numtokens==7)
			{
		    	    if (strcmp(chargv[3],"host")==0 && ipcheck(chargv[4])==0)
		    	    {	
		    		acl_cmd[0]=chargv[2];
				acl_cmd[1]=chargv[4];
				acl_cmd[2]=" ";
				acl_cmd[3]=chargv[6];
				acl_cmd[4]=chargv[5];
			    	acl_cmd[5]=" ";
				acl_stand(6,&acl_cmd[0]);
			    }
			    else if (ipcheck(chargv[3])==0)
			    {
				wild[0]=NULL;
				wild[1]=chargv[4];
				wildcard(2,&wild[0]);
				if (wc)
				{
			    	    acl_cmd[0]=chargv[2];
			    	    acl_cmd[1]=chargv[3];
			    	    acl_cmd[2]=wc;
			    	    acl_cmd[3]=chargv[7];
				    acl_cmd[4]=chargv[5];
				    acl_cmd[5]=" ";
				    acl_stand(6,&acl_cmd[0]);
		    		}	
			    }
			}    
		    }
		    // vty acl
		    if(strcmp(chargv[0],"access-class")==0  && atoi(chargv[1])>0 && atoi(chargv[1])<=99)
		    {
			if (numtokens==5)
			{
		    	    if (strcmp(chargv[3],"any")==0)
		    	    {
				acl_cmd[0]=chargv[2];
				acl_cmd[1]="any";
				acl_cmd[2]="0";
				acl_cmd[3]=intf;
			    	acl_cmd[4]=chargv[4];
			    	acl_cmd[5]="vty";
				acl_stand(6,&acl_cmd[0]);
			    }
			}	
			else if (numtokens==6)
			{
		    	    if (strcmp(chargv[3],"host")==0 && ipcheck(chargv[4])==0)
		    	    {	
		    		acl_cmd[0]=chargv[2];
				acl_cmd[1]=chargv[4];
				acl_cmd[2]=" ";
				acl_cmd[3]=intf;
			    	acl_cmd[4]=chargv[5];
			    	acl_cmd[5]="vty";
				acl_stand(6,&acl_cmd[0]);
			    }
			    else if (ipcheck(chargv[3])==0)
			    {
				wild[0]=NULL;
				wild[1]=chargv[4];
				wildcard(2,&wild[0]);
				if (wc)
				{
			    	    acl_cmd[0]=chargv[2];
			    	    acl_cmd[1]=chargv[3];
			    	    acl_cmd[2]=wc;
			    	    acl_cmd[3]=intf;
				    acl_cmd[4]=chargv[5];
				    acl_cmd[5]="vty";
				    acl_stand(6,&acl_cmd[0]);
		    		}	
			    }
			}    
		    }			
		}
	    }
	}
	
    }
    fclose(confin);
    return (0);
}

int cli_write(name)
char *name;
{

    if (!cli_need_arg ("write", name))
		 return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if (strcmp(chargv[0],"mem")==0 || strcmp(chargv[0],"memory")==0)
	    {
		printf ("Building configuration...\r\n\n");
		if (cli_show_run(1)<0)
		    printf ("%% Error writing memory\r\n");
		else    
		    printf("[OK]\n");
	    }
	    else printf ("%% Invalid input detected\r\n");
	}
	else if (numtokens>1)
	      printf ("%% Invalid input detected\r\n");
    }
    return (0);
}

int cli_password(name)
char *name;
{
    int sec=0;
    if (!cli_need_arg ("enable", name))
	     return 1;

    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if ((strcmp(chargv[0],"secret")==0 || strcmp(chargv[0],"password")==0))
		secret(sec);
	    else printf ("%% Invalid input detected\r\n");
	    
	}
	else if (numtokens>1)
	      printf ("%% Invalid input detected\r\n");
		      
    }
    return (0);
}

int cli_arp_show(void)
{
    int returncode = 0;
    FILE* arp;
    char line[MAX_LINE_LEN + 1] ;
    arp = fopen( "/proc/net/arp", "r" ); 
    if (arp==NULL) { 
	fprintf (stderr,"%% Error opening config\r\n");
	returncode = 1;
    }
    else 
    {      
	printf(" %-*s\t%s\t\t%s\r\n", (int)HELPID,"IP address","MAC-address","Iface");
	while ( fgets( line, MAX_LINE_LEN, arp) != NULL )
	{
	    if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;
	    if (makeargv(line, BLANK_STRING, &chargv) > 0) {
		if(chargv[0]!= NULL && strcmp(chargv[0],"IP")!=0)
		    printf(" %-*s\t%s\t%s\n",(int)HELPID,chargv[0],chargv[3],chargv[5]);                                                                             
	    }
	}
    }
    fclose(arp);
    return returncode;
}

int cli_copy(name)
char *name;
{

    if (!cli_need_arg ("startup-config", name))
                     return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	    printf ("%% Incomplete command\r\n");
	else if (numtokens==2)
	{
	    if ((strcmp(chargv[0],"startup-config")==0 || strcmp(chargv[0],"star")==0) && (strcmp(chargv[1],"running-config")==0 || strcmp(chargv[1],"run")==0))
	    {
		if (cli_conf_init()==0)
		    printf(" %%SYS-5-CONFIG_I:Configured from memory by console\r\n");    
	    }
	    else printf ("%% Invalid input detected\r\n");
	}
	else if (numtokens>2)
	      printf ("%% Invalid input detected\r\n");
    }
    return (0);
}

int cli_acl(name)
char *name;
{
    if (!cli_need_arg ("access-list", name))
	return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (atoi(chargv[0])>0 && atoi(chargv[0])<=99)
	    cli_acl_st(name);
	else if (atoi(chargv[0])>99 && atoi(chargv[0])<=199)
	    cli_acl_ext(name);
	else printf ("Invalid input detected\r\n");
    }
    return 0;
}    

int cli_acl_st(name)
char *name;
{
    FILE* facl = NULL;
    char acl_file[10];
    char dir[50]="/tmp/";
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==3)
	{
	    if (strcmp(chargv[1],"deny")==0 || strcmp(chargv[1],"permit")==0) 
	    {
		if (strcmp(chargv[2],"any")==0)
		{
		    if (aclstring)sprintf(aclstring,"access-list %s %s %s",chargv[0],chargv[1],chargv[2]);
		}	
		else  printf ("%% Invalid input detected\r\n");
	    }
	    else  printf ("%% Invalid input detected\r\n");
	}
	else if (numtokens==4)
	{
	    if (strcmp(chargv[1],"deny")==0 || strcmp(chargv[1],"permit")==0)    
	    {
	    // if host
		if (strcmp(chargv[2],"host")==0)
		{
		    if (ipcheck(chargv[3])==0)
		    {
			if (aclstring)sprintf(aclstring,"access-list %s %s %s %s",chargv[0],chargv[1],chargv[2],chargv[3]);
		    }
		    else printf ("%% Invalid input detected\r\n");
		}	
		// if net
		else if (ipcheck(chargv[2])==0)
		{
		    if (chargv[3])
		    {
			wild[0]=NULL;
			wild[1]=chargv[3];
			if (wildcard(2,&wild[0])==0)
			{
			    if (aclstring)sprintf(aclstring,"access-list %s %s %s %s",chargv[0],chargv[1],chargv[2],chargv[3]);
			}
		    }
		    else printf ("%% Invalid input detected\r\n");
		}
		else printf ("%% Invalid input detected\r\n");
	    }
	    else printf ("%% Invalid input detected\r\n");
	}
	else if (numtokens>4)
	    printf ("%% Invalid input detected\r\n");
	else printf ("%% Incomplete command\r\n");    
    }
    if (acl_file) sprintf(acl_file,"%s.acl.tmp",chargv[0]);
    strcat(dir,acl_file);
    facl = fopen( dir, "a+" );
    if (facl==NULL) {
	 fprintf (stderr,"%% Error saving access-list\r\n");
	 return (-1);
    }				 
    fprintf (facl,"%s\n",aclstring);
    fclose(facl);
    return 0;
}		     

int cli_acl_ext(name)
char *name;
{
printf ("Sorry, extended access-list not supported yet\n");
return 0;
}

int cli_access(name)
char *name;
{
    FILE *facl = NULL;
    FILE *aclist = NULL;
    char line[MAX_LINE_LEN + 1] ;
    int inout = 0;
    int ok = 0;
    int aclstat = 1;
    char aclnum[50];
    char acl_file[50];
    char dir[50]="/tmp/";
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (cli_status==4)
	{
	    if (numtokens==3)
	    {
		if (atoi(chargv[1])>0 && atoi(chargv[1])<=99)
 		{
		    if (strcmp(chargv[2],"in")==0)
		    {
			inout = 1;ok = 1;
			strcpy(aclnum,chargv[1]);
		    }
		    else if (strcmp(chargv[2],"out")==0)
		    {
			inout = 0; ok = 1;
			strcpy(aclnum,chargv[1]);
		    }
		    else printf("%% Invalid input detected\r\n");
		}
		else printf("%% Invalid input detected\r\n");
	    }
	    else if (numtokens<3)
		printf ("%% Incomplete command\r\n");
	    else printf("%% Invalid input detected\r\n");
	}
	else if (cli_status==5)
	{
	    if (numtokens==2)
	    {
		if (atoi(chargv[0])>0 && atoi(chargv[0])<=99)
 		{
		    if (strcmp(chargv[1],"in")==0)
		    {
			inout = 1;ok = 1;
			strcpy(aclnum,chargv[0]);
		    }
		    else if (strcmp(chargv[1],"out")==0)
		    {
			inout = 0; ok = 1;
			strcpy(aclnum,chargv[0]);
		    }
		    else printf("%% Invalid input detected\r\n");
		}
		else printf("%% Invalid input detected\r\n");
	    }
	    else if (numtokens<2)
		printf ("%% Incomplete command\r\n");
	    else printf("%% Invalid input detected\r\n");
	}	
    }	
    if (ok)
    {
	if (acl_file) sprintf(acl_file,"%s.acl.tmp",aclnum);
	strcat(dir,acl_file);
	facl = fopen( dir, "r" );
	if (facl==NULL) 
	{ 
	    fprintf (stderr,"%% Invalid access-list\r\n");
	    return (-1);
	} 
	else 
	{    
	    aclist = fopen("/etc/routercli/aclist", "a+" );
	    if (aclist==NULL) 
	    { 
		fprintf (stderr,"%% Error saving access-list\r\n");
		aclstat=0;
	    }
	    else
		fprintf (aclist,"!\n");  
	    while ( fgets( line, MAX_LINE_LEN, facl) != NULL )
	    {
		if (line[strlen(line) - 1] == '\n')
	    	    line[strlen(line) - 1] = 0;
		if (makeargv(line, BLANK_STRING, &chargv) > 0) 
		{
		    if (cli_status==4 && aclstat)
		    {
			if (chargv[4])
			    fprintf(aclist,"access-list %s %s %s %s %s %s\n!\n",chargv[1],chargv[2],chargv[3],chargv[4],inout ? "in" : "out",intf);
			else
			    fprintf(aclist,"access-list %s %s %s %s %s\n!\n",chargv[1],chargv[2],chargv[3],inout ? "in" : "out",intf);    
		    }
		    else if (cli_status==5 && aclstat)
		    {
			if (chargv[4])
			    fprintf(aclist,"access-class %s %s %s %s %s\n!\n",chargv[1],chargv[2],chargv[3],chargv[4],inout ? "in" : "out");
			else
			    fprintf(aclist,"access-class %s %s %s %s\n!\n",chargv[1],chargv[2],chargv[3],inout ? "in" : "out");   
		    }
		    if(chargv[0]!= NULL && strcmp(chargv[0],"access-list")==0  && atoi(chargv[1])>0 && atoi(chargv[1])<=99)
		    {
			if (strcmp(aclnum,chargv[1])<0)
			{
			    printf ("%% Invalid access-list\r\n");
			    return (-1);
			}    
			else if (numtokens==4)
			{
			    if (strcmp(chargv[3],"any")==0)
			    {
				acl_cmd[0]=chargv[2];
				acl_cmd[1]="any";
				acl_cmd[2]="0";
				acl_cmd[3]=intf;
				if (inout)
			    	    acl_cmd[4]="in";
				else
			    	    acl_cmd[4]="out";
				if (cli_status==4)
				    acl_cmd[5]=" ";
				else if (cli_status==5)
				    acl_cmd[5]="vty";
				acl_stand(6,&acl_cmd[0]);
			    }
			    else printf ("%% Invalid input detected\r\n");
			}	
			else if (numtokens==5)
			{
			    if (strcmp(chargv[3],"host")==0 && ipcheck(chargv[4])==0)
			    {	
				acl_cmd[0]=chargv[2];
				acl_cmd[1]=chargv[4];
				acl_cmd[2]=" ";
				acl_cmd[3]=intf;
				if (inout)
			    	    acl_cmd[4]="in";
				else
			    	    acl_cmd[4]="out";
				if (cli_status==4)
				    acl_cmd[5]=" ";
				else if (cli_status==5)
				    acl_cmd[5]="vty";
				acl_stand(6,&acl_cmd[0]);
			    }
			    else if (ipcheck(chargv[3])==0)
			    {
				wild[0]=NULL;
				wild[1]=chargv[4];
				wildcard(2,&wild[0]);
				if (wc)
				{
				    acl_cmd[0]=chargv[2];
				    acl_cmd[1]=chargv[3];
				    acl_cmd[2]=wc;
				    acl_cmd[3]=intf;
				    if (inout)
					acl_cmd[4]="in";
				    else
					acl_cmd[4]="out";
				    if (cli_status==4)
					acl_cmd[5]=" ";
				    else if (cli_status==5)
					acl_cmd[5]="vty";
				    acl_stand(6,&acl_cmd[0]);
		    		}
			    }
		        }    
		    }
		}   
	    }
	}
    }
    fclose(aclist);
    return 0;
}

int cli_acl_show(name)
char *name;
{
    FILE* aclist = NULL ;
    char c;
    aclist = fopen( "/etc/routercli/aclist", "r" ); 
    if (aclist==NULL) { 
	return (-1);
    }
    else 
    {
	c = fgetc(aclist);
	while(c!=EOF)
	{
	    putchar(c);
	    c =fgetc(aclist);
	}
    }
    fclose(aclist);
    return 0;
}

int cli_conf_no(name)
char *name;
{
    char dir[50]="/tmp/";
    char acl_file[50];
    char line[MAX_LINE_LEN + 1] ;
    FILE* pidfile = NULL;  
    int returncode = 0;
    if (!cli_need_arg ("no", name))
	returncode = 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if (strcmp(chargv[0],"router")==0 || strcmp(chargv[0],"access-list")==0)
	    	printf ("%% Incomplete command\r\n");	
	    else printf("%% Invalid input detected\r\n");
	    
	}
	else if (numtokens==2)
	{
	    if (strcmp(chargv[0],"router")==0 && strcmp(chargv[1],"rip")==0)
	    {
		pidfile= fopen( "/var/run/ripd.pid", "r" );
		if (pidfile==NULL) 
		    return (-1);
		else
		{
		    while ( fgets( line, MAX_LINE_LEN, pidfile) != NULL )
	    	    kill(atoi(line),SIGKILL);	    
		    fclose(pidfile);
		}
	    }
	    else if(strcmp(chargv[0],"access-list")==0 && atoi(chargv[1])>0 && atoi(chargv[1])<=99)
	    {
	    	if (acl_file) sprintf(acl_file,"%s.acl.tmp",chargv[1]);
		        strcat(dir,acl_file);
		if (unlink(dir)<0)
		{
		    printf ("%% Invalid access-list number\r\n");
		    returncode = 1;
		}		
	    }
	    else printf("%% Invalid input detected\r\n");
	}
	else if (numtokens>2)
	    printf("%% Invalid input detected\r\n");
    }
    return returncode;
}    



int cli_no_acl(name)
char *name;
{
    int returncode = 0;
    if (!cli_need_arg ("no", name))
	returncode = 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	printf ("N:%d 0:%s\n",numtokens,chargv[0]);
	if (numtokens==1)
	{
	    if(strcmp(chargv[0],"access-class")==0)
	    {	
		noacl();
		unlink("/etc/routercli/aclist");
	    }
	    else
		printf("%% Invalid input detected\r\n");
	} 
	else if (numtokens>1)
	     printf("%% Invalid input detected\r\n");
	     
    }
    return returncode;
}    

int cli_banner(name)
char *name;
{
    
    int returncode = 0;
    if (!cli_need_arg ("banner", name))
             returncode = 1;
	     
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1)
	{
	    if(strcmp(chargv[0],"login")==0)
	    { 
		printf ("Enter a string [max 100 characters] ending with return:\n");
		fgets (banner,100,stdin);
		
	    }
	    else  printf("%% Invalid input detected\r\n");
	}
	else if (numtokens>1) 
	    printf("%% Invalid input detected\r\n");
    }
    return returncode;
}

int cli_shell(name)
char *name;
{
    if (!name || !*name)
	system("/bin/sh");
    else printf("%% Invalid input detected\r\n");
    return (0);
}

int cli_router (name)
     char *name;
{
    int pid, status;
    if (!cli_need_arg ("router", name))
	return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1) 
	{
	    if (strcmp(chargv[0],"rip")==0)
	    {
	    	cli_status=6;
		if ((pid = fork()) < 0) 
		{
		    perror("fork");
		    return(-1);
		}
		if (pid == 0) 
		{
		    rip_cmd[0]=NULL;
		    rip_cmd[1]="-s";
		    ripd(2,&rip_cmd[0]);
		}
		while (wait(&status) != pid);
	    }
	    else printf("%% Invalid input detected\r\n");
	    
	}                             
	else if (numtokens>1)
	    printf("%% Invalid input detected\r\n");
    }
    return (0);
}

int cli_network(name)
char *name;
{
    FILE* ripconfig = NULL;
    if (!cli_need_arg ("network", name))
	return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1) 
	{
	    if (ipcheck(chargv[0])==0)
		printf ("%% Incomplete command\r\n");
	    else printf("%% Invalid input detected\r\n"); 
	}
	else if (numtokens==2)
	{
	    if (ipcheck(chargv[0])==0 && ipcheck(chargv[1])==0)
	    	printf ("%% Incomplete command\r\n");
	    else printf("%% Invalid input detected\r\n"); 
	}
	else if (numtokens==3)
	{
	    if (ipcheck(chargv[0])==0 && ipcheck(chargv[1])==0 && atoi(chargv[2]))
	    	printf ("%% Incomplete command\r\n");
	    else printf("%% Invalid input detected\r\n"); 
	}    
	else if (numtokens==4)
	{
	    if (ipcheck(chargv[0])==0 && ipcheck(chargv[1])==0 && atoi(chargv[2]) && (strcmp(chargv[3],"active")==0 || strcmp(chargv[3],"passive")==0 || strcmp(chargv[3],"external")==0)) 
	    {	
		ripconfig=fopen( "/etc/routercli/ripd.conf", "a+" );
		if (ripconfig==NULL) 
		{
		    printf("%% Error writing RIP config\r\n");
		    return (-1);
		}
		else
		{
		    fprintf(ripconfig,"net %s gateway %s metric %s %s\n",chargv[0],chargv[1],chargv[2],chargv[3]);
		    fclose(ripconfig);		
		}
	    }
	    else printf("%% Invalid input detected\r\n"); 
	}    
	else if (numtokens>4)
	    printf("%% Invalid input detected\r\n");
	
    }
    return (0);
}

int cli_clock(name)
char *name;
{
    char *time_cmd[2];
    if (!cli_need_arg ("clock", name))
	return 1;
    if (makeargv(name, BLANK_STRING, &chargv) > 0)
    {
	if (numtokens==1) 
	{
	    if (strcmp(chargv[0],"set")==0)
	    	 printf ("%% Incomplete command\r\n");
            else printf("%% Invalid input detected\r\n");
	}
	else if (numtokens==2)
	{
	    if (atoi(chargv[1]) && strlen(chargv[1])==12)
	    { 
		time_cmd[0]=NULL;
		time_cmd[1]=chargv[1];
		setclock(2,&time_cmd[0]);
	    }
	    else printf("%% Invalid input detected\r\n");
	    
	}			     
	else if (numtokens>2)
	    printf("%% Invalid input detected\r\n");
	    
    }
    return (0);
}

int startup(void)
{
    FILE* confin = NULL;
    int i;
    char line[MAX_LINE_LEN + 1] ;
    confin = fopen( "/etc/routercli/cli.conf", "r" ); 
    if (confin==NULL) { 
	return (0);
    }
    else 
    {      
	while ( fgets( line, MAX_LINE_LEN, confin) != NULL )
	{
	    if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;

	    if (makeargv(line, BLANK_STRING, &chargv) > 0) 
	    {
		if(chargv[0]!= NULL && strcmp(chargv[0],"!")!=0)
		{ 
		    if (strcmp(chargv[0],"banner")==0 && strcmp(chargv[1],"login")==0)
		    {       
			    for (i = 2; i < numtokens; i++)
				printf("%s ",chargv[i]);
			    printf ("\n\n");
		    }				 
		    if (strcmp(chargv[0],"enable")==0 && strcmp(chargv[1],"secret")==0)
		    {       
			if (chargv[2])
			    strcpy(enpass,chargv[2]);
		    }        
		}
	    }
	}
	
    }
    fclose(confin);
    return (0);
}
