alert udp any any -> $HOME_NET 161:162 (msg:"1\:1 SNMP community string buffer overflow attempt"; content:"|02 01 00 04 82 01 00|"; offset:4;)
alert udp any any -> $HOME_NET 161:162 (msg:"1\:1 SNMP community string buffer overflow attempt (with evasion)"; content:" | 04 82 01 00 |"; offset: 7; depth: 5;)
