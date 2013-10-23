alert tcp any any -> any 6000 (msg:"0\:1 X11 - MIT Magic Cookie detected"; flow:established; content: "MIT-MAGIC-COOKIE-1";)
alert tcp any any -> any 6000 (msg:"0\:1 X11 - xopen"; flow:established; content: "|6c00 0b00 0000 0000 0000 0000|";)
