<!--- start show interface --->
	<COMMAND name="definitions networks interface"
                help="Display network interfaces">
                <ACTION>exec /service/tools/definitions.exc show_interface 2>/dev/null |more -dl</ACTION>
        </COMMAND>
<!--- end show interface --->
<!--- start print --->
	<COMMAND name="definitions networks print"
                help="Display network definition list">
		<PARAM name="option"
			help="Display option"
			ptype="DN_OPTION"
			default="all"/>
                <ACTION>exec /service/tools/definitions.exc networks print ${option} 2>/dev/null |more -dl</ACTION>
        </COMMAND>
<!--- end print --->
<!--- start find --->
	<COMMAND name="definitions networks find"
                help="Search network definition">
		<PARAM name="string"
			help="String to find"
			ptype="STRING_ANY" />
		<PARAM name="option"
			help="Display option"
			ptype="DN_OPTION"
			default="all"/>
                <ACTION>exec /service/tools/definitions.exc networks find "${string}" ${option} 2>/dev/null |more -dl</ACTION>
        </COMMAND>
<!--- end find --->
<!--- start add --->
	<COMMAND name="definitions networks add"
                help="Add new network definition"/>
<!--- start add host --->
	<COMMAND name="definitions networks add host"
                help="Add new host definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="address"
                        help="Address"
                        ptype="IPV4"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
 		<PARAM name="interface_id"
                        help="Resource Id for Interfaces"
                        ptype="NUM_NULL"
			default="NULL"/>
                <ACTION>exec /service/tools/definitions.exc networks add host "${name}" ${address} "${description}" ${interface_id} 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add host --->
<!--- start adddnshost --->
	<COMMAND name="definitions networks add dnshost"
                help="Add new dnshost definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="hostname"
                        help="Hostname"
                        ptype="HOST"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
 		<PARAM name="interface_id"
                        help="Resource Id for Interfaces"
                        ptype="NUM_NULL"
			default="NULL"/>
                <ACTION>exec /service/tools/definitions.exc networks add dnshost "${name}" ${hostname} "${description}" ${interface_id} 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add dnshost --->
<!--- start add hostrange --->
	<COMMAND name="definitions networks add hostrange"
                help="Add new hostrange definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="address-start"
                        help="Address start"
                        ptype="IPV4"/>
 		<PARAM name="address-end"
                        help="Address end"
                        ptype="IPV4"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
 		<PARAM name="interface_id"
                        help="Resource Id for Interfaces"
                        ptype="NUM_NULL"
			default="NULL"/>
                <ACTION>exec /service/tools/definitions.exc networks add hostrange "${name}" "${address-start}-${address-end}" "${description}" ${interface_id} 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add hostrange --->

<!--- start add hostmac --->
	<COMMAND name="definitions networks add hostmac"
                help="Add new hostmac definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="address"
                        help="MAC Address"
                        ptype="MACADDR"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
 		<PARAM name="interface_id"
                        help="Resource Id for Interfaces"
                        ptype="NUM_NULL"
			default="NULL"/>
                <ACTION>exec /service/tools/definitions.exc networks add hostmac "${name}" ${address} "${description}" ${interface_id} 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add hostmac --->

<!--- start add network --->
	<COMMAND name="definitions networks add network"
                help="Add new hostmac definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="address"
                        help="Address"
                        ptype="IPV4"/>
 		<PARAM name="netmask"
                        help="Netmask or prefix"
                        ptype="NMASK"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
 		<PARAM name="interface_id"
                        help="Resource Id for Interfaces"
                        ptype="NUM_NULL"
			default="NULL"/>
                <ACTION>exec /service/tools/definitions.exc networks add network "${name}" ${address} ${netmask} "${description}" ${interface_id} 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add network --->

<!--- start add group --->
	<COMMAND name="definitions networks add group"
                help="Add new network group">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="members"
                        help="Definition resource Id"
                        ptype="MULTI_NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc networks add group "${name}" "${members}" "${description}" ${int} 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add group --->
<!--- end add --->
<!--- start delete --->
	<COMMAND name="definitions networks delete"
                help="Delete network definition">
		<PARAM name="id"
			help="Resource Id '*' for all"
			ptype="MULTI_NUM_ALL" />
                <ACTION>exec /service/tools/definitions.exc networks delete "${id}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end delete --->

<!--- start set --->
	<COMMAND name="definitions networks set"
                help="Change network definition setting"/>
<!--- start set host --->
	<COMMAND name="definitions networks set host"
                help="Change network definition host"/>
	<COMMAND name="definitions networks set host name"
	        help="Change network definition host name">
		<PARAM name="id"
	                help="Resource Id for network definition host"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc networks set host name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set host address"
	        help="Change network definition host address">
		<PARAM name="id"
	                help="Resource Id for network definition host"
	                ptype="NUM"/>
		<PARAM name="address"
	                help="Address"
	                ptype="IPV4"/>
	        <ACTION>exec /service/tools/definitions.exc networks set host addr "${id}" ${address} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set host description"
	        help="Change network definition host description">
		<PARAM name="id"
	                help="Resource Id for network definition host"
	                ptype="NUM"/>
		<PARAM name="description"
	                help="Description"
	                ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc networks set host note "${id}" ${description} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set host interface"
	        help="Change network definition host interface">
		<PARAM name="id"
	                help="Resource Id for network definition host"
	                ptype="NUM"/>
		<PARAM name="interface_id"
	                help="Resource Id for Interfaces"
	                ptype="NUM_NULL"
			default="NULL"/>
	        <ACTION>exec /service/tools/definitions.exc networks set host dname "${id}" ${interface_id} 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set host --->
<!--- start set dnshost --->
	<COMMAND name="definitions networks set dnshost"
                help="Change network definition dnshost"/>
	<COMMAND name="definitions networks set dnshost name"
	        help="Change network definition dnshost name">
		<PARAM name="id"
	                help="Resource Id for network definition dnshost"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc networks set dnshost name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set dnshost hostname"
	        help="Change network definition dnshost hostname">
		<PARAM name="id"
	                help="Resource Id for network definition dnshost"
	                ptype="NUM"/>
 		<PARAM name="hostname"
                        help="Hostname"
                        ptype="HOST"/>
	        <ACTION>exec /service/tools/definitions.exc networks set dnshost hostname "${id}" ${hostname} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set dnshost description"
	        help="Change network definition dnshost description">
		<PARAM name="id"
	                help="Resource Id for network definition dnshost"
	                ptype="NUM"/>
		<PARAM name="description"
	                help="Description"
	                ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc networks set dnshost note "${id}" ${description} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set dnshost interface"
	        help="Change network definition dnshost interface">
		<PARAM name="id"
	                help="Resource Id for network definition dnshost"
	                ptype="NUM"/>
		<PARAM name="interface_id"
	                help="Resource Id for Interfaces"
	                ptype="NUM_NULL"
			default="NULL"/>
	        <ACTION>exec /service/tools/definitions.exc networks set dnshost dname "${id}" ${interface_id} 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set dnshost --->
<!--- start set hostrange --->
	<COMMAND name="definitions networks set hostrange"
                help="Change network definition hostrange"/>
	<COMMAND name="definitions networks set hostrange name"
	        help="Change network definition hostrange name">
		<PARAM name="id"
	                help="Resource Id for network definition hostrange"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc networks set hostrange name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set hostrange address"
	        help="Change network definition hostrange address">
		<PARAM name="id"
	                help="Resource Id for network definition hostrange"
	                ptype="NUM"/>
 		<PARAM name="address-start"
                        help="Address start"
                        ptype="IPV4"/>
 		<PARAM name="address-end"
                        help="Address end"
                        ptype="IPV4"/>
	        <ACTION>exec /service/tools/definitions.exc networks set hostrange addr "${id}" "${address-start}-${address-end}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set hostrange description"
	        help="Change network definition hostrange description">
		<PARAM name="id"
	                help="Resource Id for network definition hostrange"
	                ptype="NUM"/>
		<PARAM name="description"
	                help="Description"
	                ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc networks set hostrange note "${id}" ${description} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set hostrange interface"
	        help="Change network definition hostrange interface">
		<PARAM name="id"
	                help="Resource Id for network definition hostrange"
	                ptype="NUM"/>
		<PARAM name="interface_id"
	                help="Resource Id for Interfaces"
	                ptype="NUM_NULL"
			default="NULL"/>
	        <ACTION>exec /service/tools/definitions.exc networks set hostrange dname "${id}" ${interface_id} 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set hostrange --->
<!--- start set hostmac --->
	<COMMAND name="definitions networks set hostmac"
                help="Change network definition hostmac"/>
	<COMMAND name="definitions networks set hostmac name"
	        help="Change network definition hostmac name">
		<PARAM name="id"
	                help="Resource Id for network definition hostmac"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc networks set hostmac name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set hostmac address"
	        help="Change network definition hostmac address">
		<PARAM name="id"
	                help="Resource Id for network definition hostmac"
	                ptype="NUM"/>
 		<PARAM name="address"
                        help="MAC Address"
                        ptype="MACADDR"/>
	        <ACTION>exec /service/tools/definitions.exc networks set hostmac addr "${id}" ${address} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set hostmac description"
	        help="Change network definition hostmac description">
		<PARAM name="id"
	                help="Resource Id for network definition hostmac"
	                ptype="NUM"/>
		<PARAM name="description"
	                help="Description"
	                ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc networks set hostmac note "${id}" ${description} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set hostmac interface"
	        help="Change network definition hostmac interface">
		<PARAM name="id"
	                help="Resource Id for network definition hostmac"
	                ptype="NUM"/>
		<PARAM name="interface_id"
	                help="Resource Id for Interfaces"
	                ptype="NUM_NULL"
			default="NULL"/>
	        <ACTION>exec /service/tools/definitions.exc networks set hostmac dname "${id}" ${interface_id} 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set hostmac --->
<!--- start set network --->
	<COMMAND name="definitions networks set network"
                help="Change network definition network"/>
	<COMMAND name="definitions networks set network name"
	        help="Change network definition network name">
		<PARAM name="id"
	                help="Resource Id for network definition network"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc networks set network name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set network address"
	        help="Change network definition network address">
		<PARAM name="id"
	                help="Resource Id for network definition network"
	                ptype="NUM"/>
		<PARAM name="address"
	                help="Address"
	                ptype="IPV4"/>
 		<PARAM name="netmask"
                        help="Netmask or prefix"
                        ptype="NMASK"/>
	        <ACTION>exec /service/tools/definitions.exc networks set network addr "${id}" ${address} ${netmask} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set network description"
	        help="Change network definition network description">
		<PARAM name="id"
	                help="Resource Id for network definition network"
	                ptype="NUM"/>
		<PARAM name="description"
	                help="Description"
	                ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc networks set network note "${id}" ${description} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set network interface"
	        help="Change network definition network interface">
		<PARAM name="id"
	                help="Resource Id for network definition network"
	                ptype="NUM"/>
		<PARAM name="interface_id"
	                help="Resource Id for Interfaces"
	                ptype="NUM_NULL"
			default="NULL"/>
	        <ACTION>exec /service/tools/definitions.exc networks set network dname "${id}" ${interface_id} 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set network --->
<!--- start set group --->
	<COMMAND name="definitions networks set group"
                help="Change network definition group"/>
	<COMMAND name="definitions networks set group name"
	        help="Change network definition group name">
		<PARAM name="id"
	                help="Resource Id for network definition group"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc networks set group name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set group members"
	        help="Change network definition group members">
		<PARAM name="id"
	                help="Resource Id for network definition group"
	                ptype="NUM"/>
 		<PARAM name="members"
                        help="Definition resource Id"
                        ptype="MULTI_NUM"/>
	        <ACTION>exec /service/tools/definitions.exc networks set group addr "${id}" ${members} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions networks set group description"
	        help="Change network definition group description">
		<PARAM name="id"
	                help="Resource Id for network definition group"
	                ptype="NUM"/>
		<PARAM name="description"
	                help="Description"
	                ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc networks set group note "${id}" ${description} 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set group --->
<!--- end set --->
