if ( top != self ) top.location.href = unescape(window.location.pathname);
function print_info(ip,url,err,time) {
        var mt=/(\d\d)\/(\S+)\/(\d\d\d\d)\:(\d\d\:\d\d\:\d\d\s+\S+)/;
	var mc=time.match(mt);
	if(mc!=null) {
		time=mc[1]+'-'+mc[2]+'-'+mc[3]+' '+mc[4];
	}
        var txt='';
        txt +='<ul>';
        if(ip!='0.0.0.0') {
                txt +='<LI><B>Your IP address: '+ip+'</B>';
        }
        txt +='<LI><B>Attempted location: <A HREF="'+url+'">'+url+'</A></B>';
        txt +='<LI><B>Error code: '+err+'</B>';
        txt +='<LI><B>Current time: '+time+'</B>';
        txt +='</ul>';
        document.write(txt);
}
