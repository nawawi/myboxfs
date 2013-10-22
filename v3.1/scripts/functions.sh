_UPX_COMPRESS() {
	local f=$1;
	local lv=$2;
	[ -z "$lv" ] && lv=0;
	local _UPX;
	if [ "x$_FUNC" = "x" ]; then
		_ERROR "${FUNCNAME}: _FUNC not set";
	fi
	if [ ! -f "$f" ]; then
		_ERROR "${FUNCNAME}: $f not found";
	fi

	[ "$lv" = "0" ] && _UPX="$_FUNC/upx-hg/upx-lzma --lzma -q --best";
	[ "$lv" = "1" ] && _UPX="$_FUNC/upx-hg/upx-lzma --lzma -q --best --force-execve";
	echo "[$lv] ${FUNCNAME} $f";
	$_UPX $f >/dev/null
	#echo "$_UPX $f" >> /tmp/upx.debug	
}

_LZMA_COMPRESS() {
	local f=$1;
	local out=$2;
	if [ ! -f "$f" ]; then
		_ERROR "${FUNCNAME}: $f not found";
	fi
	echo "[*] ${FUNCNAME} $f $out";
	lzma -9 -z -c $f > $out;
	rm -f $f;
}

_ERROR() {
	local msg=$@;
	echo "####################################################################";
	echo "ERROR: $msg";
	echo "####################################################################";
	exit 1;
}

_MECHO() {
	local msg=$1;
	echo ">> $msg";
	$msg;	
}

_BUILD_MSG() {
	local msg=$@;
	echo "####################################################################";
	echo ">> BUILD: $msg";	
	echo "####################################################################";
}

_BUILD_CHECK() {
	local msg="$1";
	if [ "x$_NAME" = "x" ]; then
		_ERROR "_NAME not set";
	fi

	if [ "x$_PDIR" = "x" ]; then
		_ERROR "_PDIR not set";
	fi
	if [ "x$_SDIR" = "x" ]; then
		_ERROR "_SDIR not set";
	fi

	_BUILD_MSG "$_NAME $msg -> $_SDIR";

	if ! cd $_PDIR >/dev/null 2>&1; then
        	_ERROR "Failed to change working directory: $_PDIR";
	fi

	if ! cd $_SDIR >/dev/null 2>&1; then
        	_ERROR "Failed to change source directory: $_SDIR";
	fi
}

_COPY_FILE() {
        if [ $# != 2 ]; then
                _ERROR "${FUNCNAME}: invalid options";
        fi
	local _SRC="$1" _DST="$2" _DSRC _BSRC _LSRC="";
	
	if [ ! -e "$_SRC" ]; then
		echo "[!] ${FUNCNAME}: file '$_SRC' not found";
		return 1;
	fi
	_BSRC="$(basename $_SRC)";
	[ -e "$_DST/$_BSRC" ] && return 1;
	
	echo "[*] ${FUNCNAME} $_SRC $_DST";
	if [ -L "$_SRC" ]; then
		_MECHO "cp -fa $_SRC $_DST";
		_LSRC="$_SRC";
		_DSRC="$(dirname $_SRC)";
		while [ -L "$_LSRC" ]; do
			_LSRC="$(readlink $_LSRC)";
			# check if not in fullpatch
			if [ ! -e "$_LSRC" ] && [ -e "${_DSRC}/${_LSRC}" ]; then
				_LSRC="${_DSRC}/${_LSRC}";
			fi
			_MECHO "cp -fa $_LSRC $_DST";
			 # strip if not symlink
			_BSRC="$(basename $_LSRC)";
			if [ ! -L "$_DST/$_BSRC" ]; then
				_MECHO "strip --strip-debug --strip-unneeded $_DST/$_BSRC";
			fi
			unset _BSRC;
		done
	else
		_MECHO "cp -fa $_SRC $_DST";
		_MECHO "strip --strip-debug --strip-unneeded $_DST/$_BSRC";		
	fi
}

_COPY_LIB() {
        if [ $# != 2 ]; then
                _ERROR "${FUNCNAME}: invalid options";
        fi

        local _SRC="$1" _DST="$2" _BSRC _DSRC _LSRC="";
	local _LO _P;
        echo "[*] ${FUNCNAME} $_SRC $_DST";
	if [ ! -e "$_SRC" ]; then
		echo "[!] ${FUNCNAME}: file '$_SRC' not found";
		return 1;
	fi
	
	# check in destination
	_BSRC="$(basename $_SRC)";
	[ -e "$_DST/$_BSRC" ] && return 1;
	[ ! -d "$_DST" ] && _MECHO "mkdir -p $_DST";
	# copy first
	_COPY_FILE "$_SRC" "$_DST";
	local _LO="$(_P=$(ldd $_SRC);echo "$_P" |tr -d '^\t' |sed -e 's/=>//g' |cut -d ' ' -f 3; echo "$_P" |tr -d '^\t' |grep -v "=>" |cut -d ' ' -f 1)";
	if [ -n "${_LO:-}" ]; then
		unset _P;
		for _P in ${_LO}; do
			_COPY_FILE "$_P" "$_DST";
		done
	fi
}

_COPY_BIN_LIB() {
	if [ "x$1" = "x" -a "x$2" = "x" ]; then
                _ERROR "_COPY_BIN_LIB(): invalid options";
        fi

        local __FILE=$1;
	local __ILIB=$2;

    	echo "[*] _COPY_BIN_LIB $__FILE $__ILIB";
	if [ ! -e "$__FILE" ]; then
		echo "[!] _COPY_BIN_LIB file '$__FILE' not found";
		return 1;
	fi
	local __LO=$(P=$(ldd $__FILE);echo "$P" |tr -d '^\t' |sed -e 's/=>//g' |cut -d ' ' -f 3; echo "$P" |tr -d '^\t' |grep -v "=>" |cut -d ' ' -f 1);
        if [ "x$__LO" != "x" ]; then
		local _LS= _FD= _FB=;
                for _FF in $__LO; do
                        _LS="";
                        if [ "x$_FF" != "x" ]; then
                                _FD="" _FB=""
				[ ! -e "$_FF" ] && continue;
                                if [ -L "$_FF" ]; then
					_FB=$(basename $_FF);
					if [ -n "$_FB" ]; then
						[ ! -e "$__ILIB/$_FB" ] && _MECHO "cp -fa $_FF $__ILIB";
					fi
                                        _LS=$(readlink -f $_FF);
					_FD=$(basename $_LS);
					if [ -n "$_FD" ]; then
						[ ! -e "$__ILIB/$_FD" ] && [ -e "$_LS" ] && _MECHO "cp -fa $_LS $__ILIB";
						[ -e "$__ILIB/$_FD" ] && _MECHO "strip --strip-debug --strip-unneeded $__ILIB/$_FD";
					fi
				else
					_FD=$(basename $_FF);
					if [ -n "$_FD" ]; then
						[ ! -e "$__ILIB/$_FD" ] && _MECHO "cp -f $_FF $__ILIB";
						[ -e "$__ILIB/$_FD" ] && _MECHO "strip --strip-debug --strip-unneeded $__ILIB/$_FD";
					fi
                                fi
                        fi
                done
        fi
}

_COPY_BIN() {
	if [ "x$1" = "x" -a "x$2" = "x" ]; then
                _ERROR "_COPY_BIN(): invalid options";
        fi

        local __FILE=$1;
	local __ROOT_DIR=$2;
	local __UPX_OPT=$3;
	[ "x$__UPX_OPT" = "x" ] && __UPX_OPT=0;
 
        if [ ! -x "$__FILE" ]; then
		_ERROR "_COPY_BIN(): $__FILE not found";
	fi

    	echo "[*] _COPY_BIN $__FILE $__ROOT_DIR $__UPX_OPT";

	local __DNAME=$(dirname $__FILE);
        local __BNAME=$(basename $__FILE);
        local __IBIN="$__ROOT_DIR/bin";
        local __ILIB="$__ROOT_DIR/lib";
	
	[ ! -d "$__IBIN" ] && _MECHO "mkdir -p $__IBIN";
	[ ! -d "$__ILIB" ] && _MECHO "mkdir -p $__ILIB";
	_MECHO "cp -f $__FILE $__IBIN";
	if [ -f "$__IBIN/$__BNAME" ]; then
        	_MECHO "chmod a-srwx $__IBIN/$__BNAME";
        	_MECHO "chmod u+rwx $__IBIN/$__BNAME";
        	_MECHO "strip --strip-all $__IBIN/$__BNAME";
		_UPX_COMPRESS $__IBIN/$__BNAME $__UPX_OPT;
		#_COPY_BIN_LIB "$__FILE" "$__ILIB";
		_COPY_LIB "$__FILE" "$__ILIB";
	else
		_ERROR "_COPY_BIN(): $__FILE copy failed";
	fi
}

_COPY_MOD() {
	if [ "x$1" = "x" -a "x$2" = "x" -a  "x$3" = "x"  "x$24" = "x" ]; then
                _ERROR "_COPY_MOD(): invalid options";
        fi
	local _SRC=$1;
	local _DST=$2;
	local _MOD=$3;
	local _OPT=$4;
	[ ! -e "$_SRC" ] && _ERROR "_COPY_MOD(): $_SRC not exist";
	[ ! -e "$_DSC" ] && _ERROR "_COPY_MOD(): $_DST not exist";
	_MECHO "cp $_OPT $_SRC $_DST";
	_MECHO "chmod $_MOD $_DST";
}

_RM_DUP_LIB() {

	if [ "x$1" = "x" ]; then
                _ERROR "_RM_DUP_LIB(): invalid options";
        fi

	local _F=$1;
	local _START="$_TOP";
	local _MSRC="$_START/src";

	if [ "x$_START" = "x" ]; then
		_ERROR "_RM_DUP_LIB(): _START not set";
	fi
	if [ "x$_NAME" = "x" ]; then
		_ERROR "_RM_DUP_LIB(): _NAME not set";
	fi
	if [ "x$_MSRC" = "x" ]; then
		_ERROR "_RM_DUP_LIB(): _MSRC not set";
	fi
	if [ "x$_MMDIR" = "x" ]; then
		_ERROR "_RM_DUP_LIB(): _MMDIR not set";
	fi
	if [ ! -d "$_START/rootfs/lib" ]; then
		_ERROR "_RM_DUP_LIB(): _START/rootfs not set";
	fi
	local _RLIB="$_START/rootfs/lib";

	if [ ! -d "$_F/lib" ]; then
		_ERROR "_RM_DUP_LIB(): $_F/lib not found";
	fi

	local _LX;
	for _LX in $_F/lib/*;do
        	local _PP=$(basename $_LX);
       		if [ -e "$_RLIB/$_PP" ]; then
               		echo "@@@ $_PP found in $_RLIB/$_PP";
               		rm -f $_LX;
		fi
	done
	_LX="";
	for _LX in $_MSRC/*/pkg/*/lib/*;do
		[[ $_LX = $_MSRC/$_NAME/pkg/* ]] && continue;
        	local _PP=$(basename $_LX);
		local _DP="${_PP//\.*}";
       		if [ -e "$_MMDIR/lib/$_PP" ]; then
               		echo "@@@ $_PP found in $_LX";
               		rm -f $_MMDIR/lib/$_PP;
			local _DPX=$(basename $(dirname $(dirname $_LX)));
			[ ! -d "$_MMDIR/tmp" ] && mkdir -p $_MMDIR/tmp/
			if [ ! -e "$_MMDIR/tmp/$_NAME-DEPEND" ]; then
				echo "$_DP : $_DPX" > $_MMDIR/tmp/$_NAME-DEPEND
			else
				if ! grep -q $_DP $_MMDIR/tmp/$_NAME-DEPEND; then
					echo "$_DP : $_DPX" >> $_MMDIR/tmp/$_NAME-DEPEND
				fi
			fi
		fi
	done
}

_PERL_STRIP() {
	local f=$1;
	if [ "x$_FUNC" = "x" ]; then
		_ERROR "_PERL_STRIP(): _FUNC not set";
	fi
	if [ ! -f "$f" ]; then
		_ERROR "_PERL_STRIP(): $f not found";
	fi
	if [ ! -x "$_FUNC/perl_strip" ]; then
		_ERROR "_PERL_STRIP(): $_FUNC/perl_strip not found";
	fi
	echo "[*] _PERL_STRIP $f";
	$_FUNC/perl_strip $f
	
}

_SH_COMPRESS() {
	local file="$1";
	local p;
	[ -f "$file.tmp1" ] && rm -f $file.tmp1 &>/dev/null;
	[ -f "$file.tmp2" ] && rm -f $file.tmp2 &>/dev/null;
	[ -f "$file.tmp3" ] && rm -f $file.tmp3 &>/dev/null;
	if [ -f "$file" ]; then
		echo "[*] _SHELL_COMPRESS $file";
		read p < $file
		if [ ! -f "$file.tmp3" ] && [[ $p = \#!* ]]; then
			echo "$p" > $file.tmp3
		fi
                ( cat $file \
			|perl -pi -e 's/^#.*\n$//g' \
			| perl -pi -e 's/^\s+#.*\n$//g' \
			| perl -pi -e 's/fi\n$/fi;/g' \
			| perl -pi -e 's/done\n$/done;/g' \
			| perl -pi -e 's/esac\n$/esac;/g' \
			| perl -pi -e 's/}\n$/};/g' \
			| perl -pi -e 's/\s+fi\s+/fi;/g' \
			| perl -pi -e 's/\s+done\s+/done;/g' \
			| perl -pi -e 's/\s+esac\s+/esac;/g' \
			| perl -pi -e 's/\s+}\s+/};/g' \
			| perl -pi -e 's/^\s+//g' \
			| perl -pi -e 's/"\n$/";/g' \
			| perl -pi -e 's/\\\n$/\n/g' \
			| perl -pi -e 's/\\\s+\n$/\n/g' \
			| perl -pi -e 's/\n$/ /g' \
			| perl -pi -e 's/^\s+$//g' \
			| perl -pi -e 's/;\s+/;/g' \
			| perl -pi -e 's/\s+;/;/g' \
			| perl -pi -e 's/done;</done </g' \
                        >> $file.tmp1
                );
        fi
	if [ -f "$file.tmp1" ]; then
		local _year=$(date +"%Y");
		[ -f "$file.tmp3" ] && cat $file.tmp3 > $file.tmp2
		echo "# Copyright (c) 2002-${_year} Tracenetwork Corporation Sdn. Bhd." >> $file.tmp2
		echo "# Authors: nawawi, Mohd nawawi Mohamad Jamili <nawawi@tracenetworkcorporation.com" >> $file.tmp2
		echo "# Compressed: $(date)" >> $file.tmp2
		cat $file.tmp1 >> $file.tmp2
	fi
	if [ -f "$file.tmp2" ]; then
		cat $file.tmp2 > $file
	fi
	rm -f $file.tmp1 $file.tmp2 $file.tmp3 &>/dev/null;
}

_JS_COMPRESS() {
	local f=$1;
	if [ ! -f "$f" ]; then
		_ERROR "_JS_COMPRESS(): $f not found";
	fi
	rm -f $f.pack
	echo "[*] _JS_COMPRESS (yuicompressor) $f";
	java -jar $_FUNC/jspack/yuicompressor -v --type js $f -o $f.pack
        [ -f "$f.pack" ] && mv $f.pack $f
	echo "[*] _JS_COMPRESS (pack) $f";
	java -jar $_FUNC/jspack/js.jar $_FUNC/jspack/build/pack.js $f $f.pack
	[ -f "$f.pack" ] && mv $f.pack $f
	rm -f $f.pack
}

_CSS_COMPRESS() {
	local f=$1;
	if [ ! -f "$f" ]; then
		_ERROR "_CSS_COMPRESS(): $f not found";
	fi
	echo "[*] _CSS_COMPRESS (yuicompressor) $f";
	rm -f $f.pack
	java -jar $_FUNC/jspack/yuicompressor -v --type css $f -o $f.pack
	[ -f "$f.pack" ] && mv $f.pack $f
	rm -f $f.pack
}

_PNG_COMPRESS() {
	[ ! -x "$_FUNC/advpng" ] && return 1;
	local f=$1;
	if [ ! -f "$f" ]; then
		_ERROR "_PNG_COMPRESS(): $f not found";
	fi
	echo "[*] _PNG_COMPRESS $f";
	$_FUNC/advpng -z4 $f;
	return $?;
}
