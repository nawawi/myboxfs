#!/bin/php -q
<?

$IP=`config -r NETWORK INSIDE_IP /hd/configs/ominix.cnf`;
if($IP==NULL) {
	$IP='Ominix';
}
$IP=trim($IP);
$rsakey=<<<_END_
-----BEGIN RSA PRIVATE KEY-----
MIICWwIBAAKBgQDCtfpbXP296CnWQgJCgvfKc6ZReoAl7nlEyM2t9bgIMdHed/l6
m47au8jW2nMtwmKO7KrRDDZBqMAYRtoznvKCvqiiijM6uKZp+0Na0LvqaEEjtgsv
qmQZZVzluCdQWPlMwJpnuRAlNAGKQkny4VdtgCTVNVWM5+MHbioqBMt4LwIDAQAB
AoGAMYnfMpOMM174Ff0hWg68QJQAAsbqB7yOugNk4RedROh3/uQDY0Bkrh4M0S0v
FLtaHAb2xs0WmMn4ZbTRG0UbadoX7a8azrbqgr1cO9Jcx7fmXdQhGnv1FwsiiKvo
ULzmYbu79CFllShhLQfG0SwlrcWBQLou3nh9Mt3di6Se1KECQQD4qUsd7SiFnOdT
9zZqkCaVSerZXYNGXKWIFKNTZQC1qbMcWlAVgVsL9Pz84IuYfssfcwwe9KLpTOwb
9NM1bpMnAkEAyHUS1ujpDpHaOvsPFGPtBZGWen7WKn3fRbDZP298W88dTyrgUNbB
1edvzxoaz4MGncEDKXv01/5NwWvCMCR3uQJAKGK2hIHEFw9IsuZvvBb1bUG4Ft2N
OJxLdjKD5EN/PgH1ySER6Kw9sU90101Q3s+ZBd6VlWF+JA4OOuwkcYUn+QJADMOP
oImSFXBqEelci3LVa85aY5Bh+LCtH9Xj72sc4wesGRdk0yDplMI411oVLPNt32uj
1OdkrsAtmUxQQBqCWQJAdHOKxoITIZRGE/EjODOaRBe3C2oODNrcQz/CGsyTeokX
7FIRZY0KvUCWNsEeGUodjdGbRQBVfpkrqALLAti9uA==
-----END RSA PRIVATE KEY-----
_END_;

$dn = array(
   "countryName" => "MY",
   "stateOrProvinceName" => "Selangor Darul Ehsan",
   "localityName" => "Petaling Jaya",
   "organizationName" => "$IP",
   "organizationalUnitName" => "$IP",
   "commonName" => "$IP",
   "emailAddress" => "admin@ominix.net"
);
$privkey = openssl_pkey_new();
$csr = openssl_csr_new($dn, $privkey);
#$sscert = openssl_csr_sign($csr, null, $privkey, 365);
openssl_csr_export($csr, $csrout);

$handle=fopen('/tmp/key.pem','w');
fputs($handle,"$rsakey");
fclose($handle);

$handle=fopen('/tmp/cert.csr','w');
fputs($handle,"$csrout");
fclose($handle);

shell_exec('/usr/miniserv/openssl/bin/openssl x509 -in /tmp/cert.csr -out /tmp/cert.pem -req -signkey /tmp/key.pem -days 365');
copy('/tmp/cert.pem','/usr/miniserv/cert.pem');
copy('/tmp/key.pem','/usr/miniserv/key.pem');
shell_exec('rm -f /tmp/*');
?>
