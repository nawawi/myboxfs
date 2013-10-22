alert tcp $WORLD 10101 -> $HOME any (msg:"SCAN-SIG myscan"; flow:stateless; ack:0; flags:S; ttl:>220; sid:613; )
alert tcp $WORLD any -> $HOME 113 (msg:"SCAN-SIG ident version request"; flow:to_server,established; content:"VERSION|0A|"; depth:16; sid:616; )
alert tcp $WORLD any -> $HOME 80 (msg:"SCAN-SIG cybercop os probe"; flow:stateless; dsize:0; flags:SF12; sid:619; )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG FIN"; flow:stateless; flags:F,12; sid:621; )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG ipEye SYN scan"; flow:stateless; flags:S; seq:1958810375; sid:622; )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NULL"; flow:stateless; ack:0; flags:0; seq:0; sid:623; )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG SYN FIN"; flow:stateless; flags:SF,12; sid:624; )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG XMAS"; flow:stateless; flags:SRAFPU,12; sid:625; )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG nmap XMAS"; flow:stateless; flags:FPU,12; sid:1228; )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG synscan portscan"; flow:stateless; flags:SF; id:39426; sid:630; )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG cybercop os PA12 attempt"; flow:stateless; flags:PA12; content:"AAAAAAAAAAAAAAAA"; depth:16; sid:626; )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG cybercop os SFU12 probe"; flow:stateless; ack:0; flags:SFU12; content:"AAAAAAAAAAAAAAAA"; depth:16; sid:627; )
alert udp $WORLD any -> $HOME 10080:10081 (msg:"SCAN-SIG Amanda client version request"; content:"Amanda"; nocase; sid:634; )
alert udp $WORLD any -> $HOME 49 (msg:"SCAN-SIG XTACACS logout"; content:"|80 07 00 00 07 00 00 04 00 00 00 00 00|"; sid:635; )
alert udp $WORLD any -> $HOME 7 (msg:"SCAN-SIG cybercop udp bomb"; content:"cybercop"; sid:636; )
alert udp $WORLD any -> $HOME any (msg:"SCAN-SIG Webtrends Scanner UDP Probe"; content:"|0A|help|0A|quite|0A|"; sid:637; )
alert tcp $WORLD any -> $HOME 22 (msg:"SCAN-SIG SSH Version map attempt"; flow:to_server,established; content:"Version_Mapper"; nocase; sid:1638; )
alert udp $WORLD any -> $HOME 1900 (msg:"SCAN-SIG UPnP service discover attempt"; content:"M-SEARCH "; depth:9; content:"ssdp|3A|discover"; sid:1917; )
alert icmp $WORLD any -> $HOME any (msg:"SCAN-SIG SolarWinds IP scan attempt"; icode:0; itype:8; content:"SolarWinds.Net"; sid:1918; )
alert tcp $WORLD any -> $HOME $HTTP_PORTS (msg:"SCAN-SIG cybercop os probe"; flow:stateless; ack:0; flags:SFP; content:"AAAAAAAAAAAAAAAA"; depth:16; sid:1133; )
#alert tcp $WORLD any -> $HOME 53 (msg:"SCAN-SIG F5 BIG-IP 3DNS TCP Probe 1"; id: 1; dsize: 24; flags: S,12; content:"|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|"; window: 2048; sid:2001609;  )
#alert tcp $WORLD any -> $HOME 53 (msg:"SCAN-SIG F5 BIG-IP 3DNS TCP Probe 2"; id: 2; dsize: 24; flags: S,12; content:"|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|"; window: 2048; sid:2001610;  )
#alert tcp $WORLD any -> $HOME 53 (msg:"SCAN-SIG F5 BIG-IP 3DNS TCP Probe 3"; id: 3; dsize: 24; flags: S,12; content:"|00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00|"; window: 2048; sid:2001611;  )
#alert icmp $WORLD any -> $HOME any (msg:"SCAN-SIG ICMP PING IPTools"; itype: 8; icode: 0; content:"|A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7|"; depth: 64; sid:9400; )
#alert tcp any any -> $HOME 3306 (msg:"SCAN-SIG MYSQL 4.0 brute force root login attempt"; flow: to_server,established; content:"|01|"; within: 1; distance: 3; content:"root|00|"; nocase; within: 5; distance: 5; threshold: type both, track by_src, count 5, seconds 60; sid:2001906;  )
#alert tcp $HOME any -> $WORLD 445 (msg:"SCAN-SIG Behavioral Unusual Port 445 traffic, Potential Scan or Infection"; flags: S,12; threshold: type both, track by_src, count 200 , seconds 60; sid:2001569;  )
#alert tcp $HOME any -> $WORLD 139 (msg:"SCAN-SIG Behavioral Unusual Port 139 traffic, Potential Scan or Infection"; flags: S,12; threshold: type both, track by_src, count 200 , seconds 60; sid:2001579;  )
#alert tcp $HOME any -> $WORLD 137 (msg:"SCAN-SIG Behavioral Unusual Port 137 traffic, Potential Scan or Infection"; flags: S,12; threshold: type both, track by_src, count 200 , seconds 60; sid:2001580;  )
#alert tcp $HOME any -> $WORLD 135 (msg:"SCAN-SIG Behavioral Unusual Port 135 traffic, Potential Scan or Infection"; flags: S,12; threshold: type both, track by_src, count 200 , seconds 60; sid:2001581;  )
#alert tcp $HOME any -> $WORLD 1434 (msg:"SCAN-SIG Behavioral Unusual Port 1434 traffic, Potential Scan or Infection"; flags: S,12; threshold: type both, track by_src, count 200 , seconds 60; sid:2001582;  )
#alert tcp $HOME any -> $WORLD 1433 (msg:"SCAN-SIG Behavioral Unusual Port 1433 traffic, Potential Scan or Infection"; flags: S,12; threshold: type both, track by_src, count 200 , seconds 60; sid:2001583;  )
#alert ip $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -sO"; dsize: 0; ip_proto: 21; sid:9402; )
#alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -sS"; fragbits: !D; dsize: 0; flags: S,12; ack: 0; window: 2048; sid:9405; )
#alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -sA"; fragbits: !D; dsize: 0; flags: A,12; window: 1024; sid:9406; )
#alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -sA"; fragbits: !D; dsize: 0; flags: A,12; window: 3072; sid:9407; )
#alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -f -sF"; fragbits: !M; dsize: 0; flags: F,12; ack: 0; window: 2048; sid:9408; )
#alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -f -sN"; fragbits: !M; dsize: 0; flags: 0,12; ack: 0; window: 2048; sid:9409;  )
#alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -f -sS"; fragbits: !M; dsize: 0; flags: S,12; ack: 0; window: 2048; sid:9500;  )
#alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -f -sX"; fragbits: !M; dsize: 0; flags: FPU,12; ack: 0; window: 2048; sid:9502; )
#alert tcp $WORLD any -> $HOME 22 (msg:"SCAN-SIG Potential SSH Scan"; flags: S; flowbits: set,ssh.brute.attempt; threshold: type threshold, track by_src, count 5, seconds 120; sid:2001219;  )
#alert tcp $WORLD any -> $HOME 443 (msg:"SCAN-SIG Scan Possible SSL Brute Force attack or Site Crawl"; flow: established; flags: S; threshold: type threshold, track by_src, count 100, seconds 60; sid:2001553;  )
#alert tcp any any -> any 23 (msg:"SCAN-SIG Behavioral Unusually fast Telnet Connections, Potential Scan or Brute Force"; flags: S,12; threshold: type both, track by_src, count 30, seconds 60; sid:2001904;  )
#alert tcp $HOME any -> $WORLD 3389 (msg:"SCAN-SIG Behavioral Unusually fast Terminal Server Traffic, Potential Scan or Infection"; flags: S,12; threshold: type both, track by_src, count 30 , seconds 60; sid:2001972;  )
alert icmp $WORLD any -> $HOME any (msg:"SCAN-SIG ICMP PING IPTools"; itype: 8; icode: 0; content:"|A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7 A7|"; depth: 64;  )
alert ip $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -sO"; dsize: 0; ip_proto: 21;  )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -sS"; fragbits: !D; dsize: 0; flags: S,12; ack: 0; window: 2048;  )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -sA (1)"; fragbits: !D; dsize: 0; flags: A,12; window: 1024;  )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -sA (2)"; fragbits: !D; dsize: 0; flags: A,12; window: 3072;  )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -f -sF"; fragbits: !M; dsize: 0; flags: F,12; ack: 0; window: 2048;  )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -f -sN"; fragbits: !M; dsize: 0; flags: 0,12; ack: 0; window: 2048;  )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -f -sS"; fragbits: !M; dsize: 0; flags: S,12; ack: 0; window: 2048;  )
alert tcp $WORLD any -> $HOME any (msg:"SCAN-SIG NMAP -f -sX"; fragbits: !M; dsize: 0; flags: FPU,12; ack: 0; window: 2048;  )