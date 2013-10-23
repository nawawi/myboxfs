alert tcp any any -> $HOME_NET 109 (msg:"1\:1 POP2 FOLD overflow attempt"; flow:to_server,established; content:"FOLD "; content:!"|0A|"; within:256;)
alert tcp any any -> $HOME_NET 109 (msg:"1\:1 POP2 FOLD arbitrary file attempt"; flow:to_server,established; content:"FOLD /";)
alert tcp any any -> $HOME_NET 109 (msg:"1\:1 POP2 x86 Linux overflow"; flow:to_server,established; content:"|eb2c 5b89 d980 c106 39d9 7c07 8001|";)
alert tcp any any -> $HOME_NET 109 (msg:"1\:1 POP2 x86 Linux overflow"; flow:to_server,established; content:"|ffff ff2f 4249 4e2f 5348 00|";)
