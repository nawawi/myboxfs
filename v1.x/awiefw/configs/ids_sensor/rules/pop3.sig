alert tcp any any -> $HOME_NET 110 (msg:"1\:1 POP3 EXPLOIT x86 BSD overflow"; flow:to_server,established; content:"|5e0 e31c 0b03 b8d7 e0e8 9fa 89f9|";)
alert tcp any any -> $HOME_NET 110 (msg:"1\:1 POP3 EXPLOIT x86 BSD overflow"; flow:to_server,established; content:"|685d 5eff d5ff d4ff f58b f590 6631|";)
alert tcp any any -> $HOME_NET 110 (msg:"1\:1 POP3 EXPLOIT x86 Linux overflow"; flow:to_server,established; content:"|d840 cd80 e8d9 ffff ff|/bin/sh";)
alert tcp any any -> $HOME_NET 110 (msg:"1\:1 POP3 EXPLOIT x86 SCO overflow"; flow:to_server,established; content:"|560e 31c0 b03b 8d7e 1289 f989 f9|";)
alert tcp any any -> $HOME_NET 110 (msg:"1\:1 POP3 EXPLOIT qpopper overflow"; flow:to_server,established; content:"|E8 D9FF FFFF|/bin/sh";)
