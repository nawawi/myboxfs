use Compress::Zlib;
sub mybox_fopen {
        my $file=shift;
        my %data;
        if($file=~/\.gz$/) {
                my $gz = gzopen($file, "rb")  or die "Cannot open $file: $gzerrno\n" ;
                while ($gz->gzreadline($_) > 0) {
                        $data{$_}=1;
                }
                die "Error reading from $file: $gzerrno\n" if $gzerrno != Z_STREAM_END ;
                $gz->gzclose() ;
        } else {
                open(FF,"< $file") || die "Cannot open $file\n";
                while(<FF>) {
                        $data{$_}=1;
                }
                close(FF);
        }
        return %data;
}

