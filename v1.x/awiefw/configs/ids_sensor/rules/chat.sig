alert tcp any any -> any any (msg:"0\:1 CHAT - ICQ access"; flow:to_server,established; content: "User-Agent\:ICQ";)
alert tcp any 80 -> any any (msg:"0\:1 CHAT - ICQ forced user addition"; flow:established,to_client; content:"Content-Type\: application/x-icq"; content:"[ICQ User]";)
alert tcp any any <> any 1863 (msg:"0\:1 CHAT - MSN message"; flow:established; content:"MSG "; depth:4; content:"Content-Type\:"; content:"text/plain"; distance:1;)
alert tcp any any <> any 1863 (msg:"0\:1 CHAT - MSN file transfer request"; flow:established; content:"MSG "; depth:4; content:"Content-Type\:"; nocase; distance:0; content:"text/x-msmsgsinvite"; nocase; distance:0; content:"Application-Name\:"; content:"File Transfer"; nocase; distance:0; )
alert tcp any any <> any 1863 (msg:"0\:1 CHAT - MSN file transfer accept"; flow:established; content:"MSG "; depth:4; content:"Content-Type\:"; content:"text/x-msmsgsinvite"; distance:0; content:"Invitation-Command\:"; content:"ACCEPT"; distance:1;)
alert tcp any any <> any 1863 (msg:"0\:1 CHAT - MSN file transfer reject"; flow:established; content:"MSG "; depth:4; content:"Content-Type\:"; content:"text/x-msmsgsinvite"; distance:0; content:"Invitation-Command\:"; content:"CANCEL"; distance:0; content:"Cancel-Code\:"; nocase; content:"REJECT"; nocase; distance:0;)
alert tcp any any -> any 1863 (msg:"0\:1 CHAT - MSN user search"; flow:to_server,established; content:"CAL "; depth:4; nocase;)
alert tcp any any -> any 1863 (msg:"0\:1 CHAT - MSN login attempt"; flow:to_server,established; content:"USR "; depth:4; nocase; content:" TWN "; distance:1; nocase;)
alert tcp any any -> any 6666:7000 (msg:"0\:1 CHAT - IRC nick change"; flow:to_server,established; content: "NICK "; offset:0;)
alert tcp any any -> any 6666:7000 (msg:"0\:1 CHAT - IRC DCC file transfer request"; flow:to_server,established; content:"PRIVMSG "; nocase; offset:0; content:" \:.DCC SEND"; nocase;)
alert tcp any any -> any 6666:7000 (msg:"0\:1 CHAT - IRC DCC chat request"; flow:to_server,established; content:"PRIVMSG "; nocase; offset:0; content:" \:.DCC CHAT chat"; nocase;)
alert tcp any any -> any 6666:7000 (msg:"0\:1 CHAT - IRC channel join"; flow:to_server,established; content:"JOIN \: \#"; nocase; offset:0;)
alert tcp any any <> any 6666:7000 (msg:"0\:1 CHAT - IRC message"; flow:established; content:"PRIVMSG "; nocase;)
alert tcp any any -> any 6666:7000 (msg:"0\:1 CHAT - IRC dns request"; flow:to_server,established; content:"USERHOST "; nocase; offset:0;)
alert tcp any 6666:7000 -> any any (msg:"0\:1 CHAT - IRC dns response"; flow:to_client,established; content:"\:"; offset:0; content:" 302 "; content:"=+";)
alert tcp any any -> $AIM_SERVERS any (msg:"0\:1 CHAT - AIM login"; flow:to_server,established; content:"|2a 01|"; offset:0; depth:2;)
alert tcp any any -> $AIM_SERVERS any (msg:"0\:1 CHAT - AIM send message"; flow:to_server,established; content:"|2a 02|"; offset:0; depth:2; content:"|00 04 00 06|"; offset:6; depth:4;)
alert tcp $AIM_SERVERS any -> any any (msg:"0\:1 CHAT - AIM receive message"; flow:to_client; content:"|2a 02|"; offset:0; depth:2; content:"|00 04 00 07|"; offset:6; depth:4;)