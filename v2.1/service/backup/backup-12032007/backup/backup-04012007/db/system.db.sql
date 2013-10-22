BEGIN TRANSACTION;
create table alarm (
email varchar not null,
ips int not null,
policy int not null,
system int not null,
note varchar not null
);
create table auth_login (
name varchar not null,
pass varchar not null
);
INSERT INTO auth_login VALUES('console','e2e1362fdaaae831d470e998f33d6f43');
INSERT INTO auth_login VALUES('admin','0192023a7bbd73250516f069df18b500');
INSERT INTO auth_login VALUES('look','163ae89f27d4215b671480e136581274');
create table crontab (
name varchar not null,
val varchar not null
);
INSERT INTO crontab VALUES('chkprog','* * * * * /bin/chkprog');
INSERT INTO crontab VALUES('compresslog','02 1 * * * /service/tools/mfs-query.exc l');
INSERT INTO crontab VALUES('graphs','*/5 * * * * /service/tools/mfs-graph.exc');
INSERT INTO crontab VALUES('flushapp','0 2 */5 * * /service/tools/mfs-query.exc x');
INSERT INTO crontab VALUES('purgelog','02 1 * * * /service/tools/mfs-query.exc p');
create table dns_name (
id integer primary key,
name varchar not null
);
INSERT INTO dns_name VALUES(1,'192.168.0.5');
INSERT INTO dns_name VALUES(2,'202.188.1.5');
INSERT INTO dns_name VALUES(3,'161.142.2.17');
INSERT INTO dns_name VALUES(4,'202.162.24.5');
INSERT INTO dns_name VALUES(5,'203.121.16.85');
create table gateway (
id int not null,
dev varchar not null,
ip varchar not null,
note varchar not null,
stat int not null
);
INSERT INTO gateway VALUES(0,'WAN','192.168.1.1','Primary',1);
INSERT INTO gateway VALUES(1,'WAN2','10.168.0.1','Load balancing',1);
INSERT INTO gateway VALUES(2,'LAN','192.168.3.1','Uplink Failover',0);
create table hosts (
id integer primary key,
ip varchar not null,
host varchar not null
);
create table ipaddr (
id integer primary key,
name varchar not null,
dev varchar not null,
ip varchar not null,
mask varchar not null,
mtu int not null,
type int not null,
onboot int not null,
note varchar not null
);
INSERT INTO ipaddr VALUES(1,'WAN','eth2','192.168.1.2','255.255.255.0',1500,1,1,'Internet (port3)');
INSERT INTO ipaddr VALUES(2,'DMZ','eth3','192.168.3.1','255.255.255.0',1500,2,0,'DMZ (port4)');
INSERT INTO ipaddr VALUES(3,'LAN','eth0','192.168.0.1','255.255.255.0',1500,3,1,'LAN (port1)');
INSERT INTO ipaddr VALUES(4,'WAN2','eth1','10.168.0.2','255.255.255.0',1500,1,1,'2nd Streamyx Connection (port2)');
create table ipalias (
id integer primary key,
pid int not null,
dev varchar not null,
ip varchar not null,
mask varchar not null,
onboot int not null,
note varchar not null
);
create table logger (
id integer primary key,
ip varchar not null,
proto varchar not null,
port int not null
);
INSERT INTO logger VALUES(1,'192.168.3.5','udp',514);
create table misc (
name varchar not null,
val varchar not null
);
INSERT INTO misc VALUES('hostname','fw.tracenetwork.com.my');
INSERT INTO misc VALUES('domainname','tracenetwork.com.my');
INSERT INTO misc VALUES('ntp_server','pool.ntp.org ntp.jaring.my 192.168.0.10');
INSERT INTO misc VALUES('ntp_time',120);
INSERT INTO misc VALUES('time_zone','Asia/Kuala_Lumpur');
INSERT INTO misc VALUES('time_utc',0);
INSERT INTO misc VALUES('kbdmap','us');
INSERT INTO misc VALUES('www_session',60);
INSERT INTO misc VALUES('ids_timeout',60);
INSERT INTO misc VALUES('ids_stat',1);
INSERT INTO misc VALUES('ids_exclude','127.0.0.1 192.168.0.5 202.186.124.206 202.186.124.207 219.94.59.110');
INSERT INTO misc VALUES('ids_mode',3);
INSERT INTO misc VALUES('pscan_stat',1);
INSERT INTO misc VALUES('pscan_exclude','127.0.0.1 192.168.0.5');
INSERT INTO misc VALUES('pscan_mode',3);
INSERT INTO misc VALUES('logger_stat',0);
INSERT INTO misc VALUES('logger_facility',1);
INSERT INTO misc VALUES('logger_priority',6);
INSERT INTO misc VALUES('ftp_server','192.168.0.5');
INSERT INTO misc VALUES('ftp_pass','1iBMIpc1gK0=');
INSERT INTO misc VALUES('ftp_user','root');
INSERT INTO misc VALUES('ftp_path','mybox_backup');
INSERT INTO misc VALUES('ftp_passive','TRUE');
INSERT INTO misc VALUES('fa_timeout',10);
INSERT INTO misc VALUES('fa_idle',20);
INSERT INTO misc VALUES('fa_switch',20);
INSERT INTO misc VALUES('fa_link','66.94.230.34:tcp/80 64.233.187.104:tcp/80');
INSERT INTO misc VALUES('local_logs',0);
INSERT INTO misc VALUES('local_logs_purge',0);
INSERT INTO misc VALUES('local_news',0);
INSERT INTO misc VALUES('blacklist_block',3);
INSERT INTO misc VALUES('blacklist_line',50);
INSERT INTO misc VALUES('update_stat',1);
INSERT INTO misc VALUES('update_date',2);
INSERT INTO misc VALUES('db_version',07122006);
INSERT INTO misc VALUES('blacklist_block_s',1);
INSERT INTO misc VALUES('blacklist_block_c',1);
INSERT INTO misc VALUES('singlelogin',0);
INSERT INTO misc VALUES('ids_acl',0);
INSERT INTO misc VALUES('update_do',1);
INSERT INTO misc VALUES('update_speed',10);
INSERT INTO misc VALUES('lo_link','202.186.124.207 204.152.191.5');
create table service_ssh (
port int not null,
stat int not null,
pass varchar not null
);
INSERT INTO service_ssh VALUES(5052,1,'sss9reRMz8WpQ');
create table vlan (
id integer primary key,
vid int not null,
dev varchar not null,
ip varchar not null,
mask varchar not null,
flag int not null,
mtu int not null,
onboot int not null,
note varchar not null
);
create table alert_email (
id integer primary key,
email varchar not null,
stat int not null,
note varchar not null
);
INSERT INTO alert_email VALUES(1,'dhifire@gmail.com',0,'aa');
INSERT INTO alert_email VALUES(2,'mfsdev@gmail.com',0,'');
create table alert_setting (
stat int not null,
report_min int not null,
report_title varchar not null,
report_logs int not null,
report_strg int not null,
report_mem int not null,
report_sys int not null,
report_names varchar not null,
usesmtp int not null,
smtphost varchar not null,
pophost varchar not null,
user varchar not null,
pass varchar not null,
mailfrom varchar not null
);
INSERT INTO alert_setting VALUES(0,120,'Mybox System Notification',1,1,1,1,'auth,system,firewall,httpsd,sshd,ips,portscan,dhcpd',1,'smtp.tracenetwork.com.my','pop.tracenetwork.com.my','nawawi@tracenetwork.com.my','xg5LGa50juqE8Q==','admin@mybox.net.my');
create table htb_client (
id integer primary key,
cname varchar not null,
bandwidth int not null,
climit int not null,
burst int not null,
priority int not null,
mark int not null,
src varchar not null,
srcp varchar not null,
dst varchar not null,
dstp varchar not null,
note varchar not null,
stat int not null
);
create table htb_class (
id integer primary key,
cname varchar not null,
dev varchar not null, 
bandwidth int not null,
climit int not null,
burst int not null,
priority int not null,
note varchar not null,
stat int not null 
);
create table route (  
id integer primary key,
ip varchar not null,
mask varchar not null,
gateway varchar not null,                                      
note varchar not null,                                                                
stat int not null,
type int not null                                                              
);
create table dhcp_hosts (
id integer primary key,
ip varchar not null,
mac varchar not null,
note varchar not null,
stat int not null
);
INSERT INTO dhcp_hosts VALUES(1,'192.168.0.22','00:13:02:BF:29:D0','awie-wlan',1);
INSERT INTO dhcp_hosts VALUES(2,'192.168.0.23','00:0E:35:F0:A2:A7','nuwa',1);
INSERT INTO dhcp_hosts VALUES(3,'192.168.0.24','00:16:D3:44:1B:62','nizam',1);
INSERT INTO dhcp_hosts VALUES(4,'192.168.0.25','00:16:D3:44:1B:51','awie-lan',1);
INSERT INTO dhcp_hosts VALUES(5,'192.168.0.26','00:16:E6:66:47:A0','arafah',1);
INSERT INTO dhcp_hosts VALUES(6,'192.168.0.27','00:16:E6:6A:66:62','lalila',1);
INSERT INTO dhcp_hosts VALUES(7,'192.168.0.7','00:0C:29:7B:00:E9','awie-vmware',1);
create table service_ftp (
port int not null,
dport int not null,
stat int not null
);
INSERT INTO service_ftp VALUES(5053,5054,1);
create table accesslist (                                                             
id integer primary key,                                                               
ip varchar not null,                                                                  
ssh int not null,                                                                     
https int not null,
ftp int not null,                                                            
stat int not null,                                                                    
note varchar not null                                                                 
);
INSERT INTO accesslist VALUES(1,'192.168.0.22',1,1,1,0,'awie-wlan');
INSERT INTO accesslist VALUES(2,'192.168.0.23',1,1,1,1,'nuwa');
INSERT INTO accesslist VALUES(3,'192.168.0.5',1,1,1,1,'file server');
INSERT INTO accesslist VALUES(4,'192.168.0.24',1,1,1,1,'nizam');
INSERT INTO accesslist VALUES(6,'218.111.146.66',1,1,1,1,'hms');
INSERT INTO accesslist VALUES(7,'202.186.124.206',1,1,1,1,'tracenetwork-skali');
INSERT INTO accesslist VALUES(8,'202.157.186.194',1,1,1,0,'tracenetwork-myloca');
INSERT INTO accesslist VALUES(9,'192.168.0.25',1,1,1,1,'awie-lan');
INSERT INTO accesslist VALUES(11,'192.168.0.7',1,0,1,1,'awie-vmware');
INSERT INTO accesslist VALUES(14,'219.95.132.107',1,1,1,1,'');
INSERT INTO accesslist VALUES(15,'192.168.0.254',1,1,1,0,'meeting room');
INSERT INTO accesslist VALUES(16,'192.168.0.26',1,1,1,1,'arafah');
INSERT INTO accesslist VALUES(17,'192.168.0.27',1,1,1,1,'laila');
INSERT INTO accesslist VALUES(19,'218.111.30.128',1,1,1,1,'');
INSERT INTO accesslist VALUES(20,'219.94.116.38',1,1,1,1,'awie&#039;s home');
create table service_snmp (
port int not null,
stat int not null,
comm varchar not null,
location varchar not null,
contact varchar not null,
astat int not null,
alist varchar not null
);
INSERT INTO service_snmp VALUES(161,0,'mybox','traceoffice','awie@mybox.net.my',1,'');
create table static_arp (
id integer primary key,
ip varchar not null,
mac varchar not null,
note varchar not null,
stat int not null
);
INSERT INTO static_arp VALUES(1,'192.168.0.4','00:11:09:5D:D9:13','web',1);
INSERT INTO static_arp VALUES(3,'192.168.0.5','00:0C:F1:8E:42:C6','samba',1);
INSERT INTO static_arp VALUES(4,'192.168.0.254','00:50:BA:D7:BC:C4','',1);
create table ipscan_hosts (
id integer primary key,
ip varchar not null,
mac varchar not null,
host varchar not null,
note varchar not null,
stat int not null
);
create table auth_ad (
domain varchar not null,
ip varchar not null,
admin varchar not null,
pass varchar not null,
stat int not null
);
INSERT INTO auth_ad VALUES('ad.tracenetwork.com.my','192.168.0.10','administrator','1nwIX/os',0);
create table auth_user (
id integer primary key,
name varchar not null,
passwd varchar not null,
stat int not null
);
INSERT INTO auth_user VALUES(1,'awie','hjlSDv4o3A==',1);
INSERT INTO auth_user VALUES(2,'anuwa','hSpaXvks3w==',1);
create table pptp_vpn (
listen varchar not null,
localip varchar not null,
ipstart varchar not null,
ipend varchar not null,
bcrelay varchar not null,
mswins varchar not null,
dnswin varchar not null,
mtu varchar not null,
grp varchar not null,
usead int not null,
stat int not null
);
INSERT INTO pptp_vpn VALUES('','192.168.0.100','192.168.0.245',254,'eth2','192.168.0.10 192.168.0.5','192.168.0.5',1400,'',0,1);
create table service_dhcp (
stat int not null,
dev varchar not null,
range_start varchar not null,
range_end varchar not null,
default_lease varchar not null,
max_lease varchar not null,
deny int not null
);
INSERT INTO service_dhcp VALUES(1,'LAN','192.168.0.100','192.168.0.254',7200,86400,0);
create table service_https (
port int not null,
stat int not null
);
INSERT INTO service_https VALUES(5051,1);
create table site_certificate (
country varchar not null,
state varchar not null,
city varchar not null,
organization varchar not null,
section varchar not null,
email varchar not null,
hostname varchar not null,
certstat int not null,
cert varchar not null
);
INSERT INTO site_certificate VALUES('my','selangor d.e','p.j','tracenetwork','tracenetwork','admin@tracenetwork.com.my','192.168.0.1','1=1','-----BEGIN RSA PRIVATE KEY-----
MIICXgIBAAKBgQCfN8yFWWuu1EEnTU/TQLV+XDJn3vmieaccfVtj8aDa1eRCs3PK
ysXvuyVZ18lmnvR7J8CxD+YaXrrvjw4tf3VJc4ZT5C1GVNM7cQ/o02P60qmXw3ix
ii9Mzoa19CQz6Gsl0YFYGGVzq7i837Wa4OttxaB6px+gONHHP6KD1NfubQIDAQAB
AoGAU2ZuGle5wU1rh4PG0ZiuuS7gNGqnBsWElLbRXWvfX2H0/D914v8OacH1Eben
xrpyw+QR2m3CkYOXlg6wVYLTUz+y/Orjc+enAo475awOF5LollnBQ9+R7SpjUU8q
WerFBfdGdh/70RoT4A0V6feBedxFYmbRKB+apkCU7Ww5qaECQQDQhNozrZSyWMxJ
uvfAkv3Xbad4svBNjo3F3Yq5BrV52JIRFkFxVjQpL7ml7xBbHEHPVIP/8zazaHfO
TP7M/VXlAkEAw3kOJFJzyJAl7dcpMnd5YDRBeBukjM1lD0tOg9krf7mFWXXWv4E/
F9Rd/uLJ7bcejkvLXqALdvBD6X3NvsWt6QJBAMzPIM+PmDUQ+R8WUNDK0B9wKgVo
pJVZBrGpZ99S0p8qdFXqKT9cL4GgydVNGiCTgaRWXAzjZAAWBU8R21eXIW0CQQCG
XuH8z/0towGfxKdaPMzf+CwFH6N28v6Ei/T2J8SCMYrcW2f7DyLLA751p3q+AYJs
uOedLZGT+kgE6ukTOhp5AkEAutBYVxESAFtRx2h4gYwdK16XnkomLQ2OY0ZnQZNN
8a3nuW+LcAauOHA2h59mV+nzq1+5hpR2nso3DjC2THUmjA==
-----END RSA PRIVATE KEY-----
-----BEGIN CERTIFICATE-----
MIIC1TCCAj6gAwIBAgIJAPpXaAcbI2tfMA0GCSqGSIb3DQEBBQUAMIGgMQswCQYD
VQQGEwJteTEVMBMGA1UECBMMc2VsYW5nb3IgZC5lMQwwCgYDVQQHEwNwLmoxFTAT
BgNVBAoTDHRyYWNlbmV0d29yazEVMBMGA1UECxMMdHJhY2VuZXR3b3JrMRQwEgYD
VQQDEwsxOTIuMTY4LjAuMTEoMCYGCSqGSIb3DQEJARYZYWRtaW5AdHJhY2VuZXR3
b3JrLmNvbS5teTAeFw0wNjEyMTMxMDA4MjJaFw0wNzEyMTMxMDA4MjJaMIGgMQsw
CQYDVQQGEwJteTEVMBMGA1UECBMMc2VsYW5nb3IgZC5lMQwwCgYDVQQHEwNwLmox
FTATBgNVBAoTDHRyYWNlbmV0d29yazEVMBMGA1UECxMMdHJhY2VuZXR3b3JrMRQw
EgYDVQQDEwsxOTIuMTY4LjAuMTEoMCYGCSqGSIb3DQEJARYZYWRtaW5AdHJhY2Vu
ZXR3b3JrLmNvbS5teTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAnzfMhVlr
rtRBJ01P00C1flwyZ975onmnHH1bY/Gg2tXkQrNzysrF77slWdfJZp70eyfAsQ/m
Gl66748OLX91SXOGU+QtRlTTO3EP6NNj+tKpl8N4sYovTM6GtfQkM+hrJdGBWBhl
c6u4vN+1muDrbcWgeqcfoDjRxz+ig9TX7m0CAwEAAaMVMBMwEQYJYIZIAYb4QgEB
BAQDAgZAMA0GCSqGSIb3DQEBBQUAA4GBAFs7kwve2G504+xej+E6Nbf++NObdg3i
d8sSwKBZrOYiOUjIvXfpJApzVnNTAKmIJFr/OhQGhlAxAUSXC1pOLBmOHXMV4pI8
+CQK1m1D/bv+6dVF0c6LCzeeECgY61XRBA9h23V9swP8KlZgzrduD8cSA0p1so8f
3R89ARVjMGka
-----END CERTIFICATE-----');
create table def_services (
id integer primary key,
name varchar not null,
val varchar not null,
note varchar not null,
stat int not null
);
INSERT INTO def_services VALUES(0,'any','any','matches any port',1);
INSERT INTO def_services VALUES(1,'HTTP',80,'HTTP port',1);
INSERT INTO def_services VALUES(2,'HTTPS',443,'HTTPS port',1);
INSERT INTO def_services VALUES(3,'FTP',21,'FTP port',1);
INSERT INTO def_services VALUES(4,'FTP-DATA',20,'FTP-DATA port',1);
INSERT INTO def_services VALUES(5,'SSH',22,'SSH port',1);
INSERT INTO def_services VALUES(6,'TELNET',23,'TELNET port',1);
INSERT INTO def_services VALUES(7,'SMTP',25,'SMTP port',1);
INSERT INTO def_services VALUES(8,'POP3',110,'POP3 port',1);
INSERT INTO def_services VALUES(9,'IMAP',143,'IMAP port',1);
INSERT INTO def_services VALUES(10,'SNMP',161,'SNMP port',1);
INSERT INTO def_services VALUES(11,'PPTP',1723,'PPTP port',1);
INSERT INTO def_services VALUES(12,'NETBIOS','137:139','NETBIOS port',1);
INSERT INTO def_services VALUES(13,'AD-LDAP',389,'AD-LDAP port',1);
INSERT INTO def_services VALUES(14,'AD-AUTHENTICATION',10260,'AD-Authentication port',1);
INSERT INTO def_services VALUES(15,'DNS',53,'DNS port',1);
INSERT INTO def_services VALUES(16,'NTP',123,'NTP port',1);
INSERT INTO def_services VALUES(17,'KERBEROS',88,'KERBEROS port',1);
INSERT INTO def_services VALUES(18,'DCOM','5000:5500','DCOM port',1);
INSERT INTO def_services VALUES(19,'MICROSOFT DS',445,'MICROSOFT DS port',1);
INSERT INTO def_services VALUES(20,'KPASSWORD',464,'KPassword port',1);
INSERT INTO def_services VALUES(21,'PRINTER',515,'Printer port',1);
INSERT INTO def_services VALUES(22,'LDAP-SSL',636,'LDAP-SSL port',1);
INSERT INTO def_services VALUES(23,'LOTUS-NOTES',1352,'Lotus-Notes port',1);
INSERT INTO def_services VALUES(24,'MICROSOFT-SQL',1433,'Microsoft-SQL port',1);
INSERT INTO def_services VALUES(25,'MICROSOFT-SQL-MONITOR',1434,'Microsoft-SQL-Monitor port',1);
INSERT INTO def_services VALUES(26,'PRINTING-PORT',9100,'Printing-Port port',1);
create table def_networks (
id integer primary key,
name varchar not null,
val varchar not null,
note varchar not null,
stat int not null
);
INSERT INTO def_networks VALUES(0,'any','any','matches any ip/network',1);
INSERT INTO def_networks VALUES(1,'ADMIN','192.168.0.22 192.168.0.23 192.168.0.24 192.168.0.25','core tracenetwork',1);
create table def_times (
id integer primary key,
name varchar not null,
val varchar not null,
val2 varchar not null,
val3 varchar not null,
note varchar not null,
stat int not null
);
INSERT INTO def_times VALUES(0,'any','any','','','matches any times/dates/days',1);
INSERT INTO def_times VALUES(1,'WORK DAYS','','09:00','18:34','work days',1);
create table policy_inbound (
id int not null,
proto varchar not null,
srcn int not null,
src varchar not null,
srcp varchar not null,
dstn int not null,
dst varchar not null,
dstp varchar not null,
action varchar not null,
usetime int not null,
timet varchar not null,
timeday varchar not null,
timestart varchar not null,
timeend varchar not null,
log int not null,
stat int not null,
note varchar not null
);
INSERT INTO policy_inbound VALUES(3,'any',0,'any','any',0,'any','any','PASS',0,'','','','',0,1,'');
INSERT INTO policy_inbound VALUES(2,'IPP2P',0,'any','any',0,'any','any','DROP',0,'','','','',0,1,'');
INSERT INTO policy_inbound VALUES(1,'TCP',0,'admin','any',0,'any','any','PASS',0,'','','','',0,1,'');
create table policy_outbound (
id int not null,
proto varchar not null,
srcn int not null,
src varchar not null,
srcp varchar not null,
dstn int not null,
dst varchar not null,
dstp varchar not null,
action varchar not null,
usetime int not null,
timet varchar not null,
timeday varchar not null,
timestart varchar not null,
timeend varchar not null,
log int not null,
stat int not null,
note varchar not null
);
INSERT INTO policy_outbound VALUES(3,'any',0,'any','any',0,'any','any','PASS',0,'','','','',0,1,'');
INSERT INTO policy_outbound VALUES(2,'IPP2P',0,'any','any',0,'any','any','DROP',0,'','','','',0,1,'');
INSERT INTO policy_outbound VALUES(1,'any',0,'192.168.0.22 192.168.0.23 192.168.0.24 192.168.0.25 192.168.0.27','any',0,'any','any','PASS',0,'','','','',0,1,'');
create table policy_dnat (
id int not null,
proto varchar not null,
src varchar not null,
dst varchar not null,
dstp varchar not null,
fwd varchar not null,
fwdp varchar not null,
usetime int not null,
timet varchar not null,
timeday varchar not null,
timestart varchar not null,
timeend varchar not null,
log int not null,
stat int not null,
note varchar not null
);
INSERT INTO policy_dnat VALUES(1,'TCP','any','WAN',222,'192.168.0.5','SSH',0,'','','','',0,1,'');
INSERT INTO policy_dnat VALUES(2,'TCP','any','WAN','HTTP','192.168.0.4','HTTP',0,'','','','',0,1,'');
INSERT INTO policy_dnat VALUES(3,'TCP','any','WAN','SSH','192.168.0.4','SSH',0,'','','','',0,1,'');
INSERT INTO policy_dnat VALUES(4,'TCP','any','WAN','FTP','192.168.0.5','FTP',0,'','','','',0,0,'');
INSERT INTO policy_dnat VALUES(5,'TCP/UDP','any','WAN','DNS','192.168.0.4','DNS',0,'','','','',0,0,'');
INSERT INTO policy_dnat VALUES(6,'TCP','any','WAN',10000,'192.168.0.4',10000,0,'','','','',0,0,'test dsdf');
create table policy_snat (
id int not null,
proto varchar not null,
src varchar not null,
dst varchar not null,
dstp varchar not null,
fwd varchar not null,
fwdp varchar not null,
usetime int not null,
timet varchar not null,
timeday varchar not null,
timestart varchar not null,
timeend varchar not null,
log int not null,
stat int not null,
note varchar not null
);
create table policy_netmap (
id int not null,
type int not null,
src varchar not null,
dst varchar not null,
fwd varchar not null,
usetime int not null,
timet varchar not null,
timeday varchar not null,
timestart varchar not null,
timeend varchar not null,
stat int not null,
note varchar not null
);
COMMIT;
