alert udp $WORLD 53 -> $HOME any (msg:"DNS-SIG SPOOF query response PTR with TTL of 1 min. and no authority"; content:"|85 80 00 01 00 01 00 00 00 00|"; content:"|C0 0C 00 0C 00 01 00 00 00|<|00 0F|"; sid:253; )
alert udp $WORLD 53 -> $HOME any (msg:"DNS-SIG SPOOF query response with TTL of 1 min. and no authority"; content:"|81 80 00 01 00 01 00 00 00 00|"; content:"|C0 0C 00 01 00 01 00 00 00|<|00 04|"; sid:254; )
alert tcp $WORLD any -> $HOME 53 (msg:"DNS-SIG EXPLOIT named 8.2->8.2.1"; flow:to_server,established; content:"../../../"; sid:258; )
alert tcp $WORLD any -> $HOME 53 (msg:"DNS-SIG EXPLOIT named tsig overflow attempt (1)"; flow:to_server,established; content:"|AB CD 09 80 00 00 00 01 00 00 00 00 00 00 01 00 01|    |02|a"; sid:303; )
alert udp $WORLD any -> $HOME 53 (msg:"DNS-SIG EXPLOIT named tsig overflow attempt (2)"; content:"|80 00 07 00 00 00 00 00 01|?|00 01 02|"; sid:314; )
alert tcp $WORLD any -> $HOME 53 (msg:"DNS-SIG EXPLOIT named overflow ADM"; flow:to_server,established; content:"thisissometempspaceforthesockinaddrinyeahyeahiknowthisislamebutanywaywhocareshorizongotitworkingsoalliscool"; sid:259; )
alert tcp $WORLD any -> $HOME 53 (msg:"DNS-SIG EXPLOIT named overflow ADMROCKS"; flow:to_server,established; content:"ADMROCKS"; sid:260; )
alert tcp $WORLD any -> $HOME 53 (msg:"DNS-SIG EXPLOIT named overflow attempt"; flow:to_server,established; content:"|CD 80 E8 D7 FF FF FF|/bin/sh"; sid:261; )
alert tcp $WORLD any -> $HOME 53 (msg:"DNS-SIG EXPLOIT sparc overflow attempt"; flow:to_server,established; content:"|90 1A C0 0F 90 02| |08 92 02| |0F D0 23 BF F8|"; sid:267; )
