alert tcp any 119 -> $HOME_NET any (msg:"1\:1 NNTP return code buffer overflow attempt"; flow:to_server,established,no_stream; content:"200 "; offset:0; depth:4; content:!"|0a|"; within:64;)
alert tcp any any -> $HOME_NET 119 (msg:"1\:1 NNTP AUTHINFO USER overflow attempt"; flow:to_server,established; content:"AUTHINFO USER "; nocase; depth:14; content:!"|0a|"; within:500;)
