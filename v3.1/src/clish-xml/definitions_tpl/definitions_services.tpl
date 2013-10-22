<!--- start show interface --->
	<COMMAND name="definitions services icmptype"
                help="Display ICMP type/code">
                <ACTION>exec /service/tools/definitions.exc show_icmptype 2>/dev/null |more -dl</ACTION>
        </COMMAND>
<!--- end show interface --->
<!--- start print --->
	<COMMAND name="definitions services print"
                help="Display service definition list">
		<PARAM name="option"
			help="Display option"
			ptype="DS_OPTION"
			default="all"/>
                <ACTION>exec /service/tools/definitions.exc services print ${option} 2>/dev/null |more -dl</ACTION>
        </COMMAND>
<!--- end print --->
<!--- start find --->
	<COMMAND name="definitions services find"
                help="Search service definition">
		<PARAM name="string"
			help="String to find"
			ptype="STRING_ANY" />
		<PARAM name="option"
			help="Display option"
			ptype="DS_OPTION"
			default="all"/>
                <ACTION>exec /service/tools/definitions.exc services find "${string}" ${option} 2>/dev/null |more -dl</ACTION>
        </COMMAND>
<!--- end find --->
<!--- start add --->
	<COMMAND name="definitions services add"
                help="Add new service definition"/>
<!--- start add tcp --->
	<COMMAND name="definitions services add tcp"
                help="Add new TCP definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="dstport"
                        help="Destination port"
                        ptype="PORT_NUM"/>
 		<PARAM name="srcport"
                        help="Source port"
                        ptype="PORT_NUM"
			default="1:65535"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc services add tcp "${name}" ${srcport} ${dstport} "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add tcp --->

<!--- start add udp --->
	<COMMAND name="definitions services add udp"
                help="Add new UDP definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="dstport"
                        help="Destination port"
                        ptype="PORT_NUM"/>
 		<PARAM name="srcport"
                        help="Source port"
                        ptype="PORT_NUM"
			default="1:65535"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc services add udp "${name}" ${srcport} ${dstport} "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add udp --->

<!--- start add tcpudp --->
	<COMMAND name="definitions services add tcpudp"
                help="Add new TCP/UDP definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="dstport"
                        help="Destination port"
                        ptype="PORT_NUM"/>
 		<PARAM name="srcport"
                        help="Source port"
                        ptype="PORT_NUM"
			default="1:65535"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc services add tcpudp "${name}" ${srcport} ${dstport} "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add tcpudp --->

<!--- start add icmp --->
	<COMMAND name="definitions services add icmp"
                help="Add new ICMP definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="type_id"
                        help="Resource Id for ICMP Type/Code"
                        ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc services add icmp "${name}" "${type_id}" "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add icmp --->

<!--- start add ip --->
	<COMMAND name="definitions services add ip"
                help="Add new IP definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="protocol"
                        help="Protocol number"
                        ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc services add ip "${name}" "${protocol}" "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add ip --->

<!--- start add esp --->
	<COMMAND name="definitions services add esp"
                help="Add new ESP definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="spi"
                        help="Security Parameters Index"
                        ptype="PORT_NUM"
			default="256:4294967295"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc services add esp "${name}" "${spi}" "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add esp --->

<!--- start add ah --->
	<COMMAND name="definitions services add ah"
                help="Add new AH definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="spi"
                        help="Security Parameters Index"
                        ptype="PORT_NUM"
			default="256:4294967295"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc services add ah "${name}" "${spi}" "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add ah --->

<!--- start add group --->
	<COMMAND name="definitions services add group"
                help="Add new service group">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="members"
                        help="Definition service"
                        ptype="MULTI_NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc services add group "${name}" "${members}" "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add group --->

<!--- start delete --->
	<COMMAND name="definitions services delete"
                help="Delete service definition">
		<PARAM name="id"
			help="Resource Id '*' for all"
			ptype="MULTI_NUM_ALL" />
                <ACTION>exec /service/tools/definitions.exc services delete "${id}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end delete --->

<!--- start set --->
	<COMMAND name="definitions services set"
                help="Change service definition setting"/>
<!--- start set tcp --->
	<COMMAND name="definitions services set tcp"
                help="Change service definition tcp"/>
	<COMMAND name="definitions services set tcp name"
	        help="Change service definition tcp name">
		<PARAM name="id"
	                help="Resource Id for service definition tcp"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc services set tcp name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set tcp source-port"
	        help="Change service definition tcp source-port">
		<PARAM name="id"
	                help="Resource Id for service definition tcp"
	                ptype="NUM"/>
 		<PARAM name="srcport"
                        help="Source port"
                        ptype="PORT_NUM"
			default="1:65535"/>
	        <ACTION>exec /service/tools/definitions.exc services set tcp sport "${id}" ${srcport} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set tcp destination-port"
	        help="Change service definition tcp destination-port">
		<PARAM name="id"
	                help="Resource Id for service definition tcp"
	                ptype="NUM"/>
 		<PARAM name="dstport"
                        help="Destination port"
                        ptype="PORT_NUM"/>
	        <ACTION>exec /service/tools/definitions.exc services set tcp dport "${id}" ${dstport} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set tcp description"
	        help="Change service definition tcp description">
		<PARAM name="id"
	                help="Resource Id for service definition tcp"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc services set tcp note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set tcp --->
<!--- start set udp --->
	<COMMAND name="definitions services set udp"
                help="Change service definition udp"/>
	<COMMAND name="definitions services set udp name"
	        help="Change service definition udp name">
		<PARAM name="id"
	                help="Resource Id for service definition udp"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc services set udp name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set udp source-port"
	        help="Change service definition udp source-port">
		<PARAM name="id"
	                help="Resource Id for service definition udp"
	                ptype="NUM"/>
 		<PARAM name="srcport"
                        help="Source port"
                        ptype="PORT_NUM"
			default="1:65535"/>
	        <ACTION>exec /service/tools/definitions.exc services set udp sport "${id}" ${srcport} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set udp destination-port"
	        help="Change service definition udp destination-port">
		<PARAM name="id"
	                help="Resource Id for service definition udp"
	                ptype="NUM"/>
 		<PARAM name="dstport"
                        help="Destination port"
                        ptype="PORT_NUM"/>
	        <ACTION>exec /service/tools/definitions.exc services set udp dport "${id}" ${dstport} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set udp description"
	        help="Change service definition udp description">
		<PARAM name="id"
	                help="Resource Id for service definition udp"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc services set udp note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set udp --->
<!--- start set tcpudp --->
	<COMMAND name="definitions services set tcpudp"
                help="Change service definition tcpudp"/>
	<COMMAND name="definitions services set tcpudp name"
	        help="Change service definition tcpudp name">
		<PARAM name="id"
	                help="Resource Id for service definition tcpudp"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc services set tcpudp name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set tcpudp source-port"
	        help="Change service definition tcpudp source-port">
		<PARAM name="id"
	                help="Resource Id for service definition tcpudp"
	                ptype="NUM"/>
 		<PARAM name="srcport"
                        help="Source port"
                        ptype="PORT_NUM"
			default="1:65535"/>
	        <ACTION>exec /service/tools/definitions.exc services set tcpudp sport "${id}" ${srcport} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set tcpudp destination-port"
	        help="Change service definition tcpudp destination-port">
		<PARAM name="id"
	                help="Resource Id for service definition tcpudp"
	                ptype="NUM"/>
 		<PARAM name="dstport"
                        help="Destination port"
                        ptype="PORT_NUM"/>
	        <ACTION>exec /service/tools/definitions.exc services set tcpudp dport "${id}" ${dstport} 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set tcpudp description"
	        help="Change service definition tcpudp description">
		<PARAM name="id"
	                help="Resource Id for service definition tcpudp"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc services set tcpudp note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set tcpudp --->
<!--- start set icmp --->
	<COMMAND name="definitions services set icmp"
                help="Change service definition icmp"/>
	<COMMAND name="definitions services set icmp name"
	        help="Change service definition icmp name">
		<PARAM name="id"
	                help="Resource Id for service definition icmp"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc services set icmp name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set icmp type"
	        help="Change service definition icmp type">
		<PARAM name="id"
	                help="Resource Id for service definition icmp"
	                ptype="NUM"/>
 		<PARAM name="type_id"
                        help="Resource Id for ICMP Type/Code"
                        ptype="NUM"/>
	        <ACTION>exec /service/tools/definitions.exc services set icmp code "${id}" ${type_id} 2>/dev/null</ACTION>
	</COMMAND>

	<COMMAND name="definitions services set icmp description"
	        help="Change service definition icmp description">
		<PARAM name="id"
	                help="Resource Id for service definition icmp"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc services set icmp note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set icmp --->
<!--- start set ip --->
	<COMMAND name="definitions services set ip"
                help="Change service definition ip"/>
	<COMMAND name="definitions services set ip name"
	        help="Change service definition ip name">
		<PARAM name="id"
	                help="Resource Id for service definition ip"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc services set ip name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set ip protocol"
	        help="Change service definition ip protocol">
		<PARAM name="id"
	                help="Resource Id for service definition ip"
	                ptype="NUM"/>
 		<PARAM name="protocol"
                        help="Protocol number"
                        ptype="NUM"/>
	        <ACTION>exec /service/tools/definitions.exc services set ip num "${id}" ${protocol} 2>/dev/null</ACTION>
	</COMMAND>

	<COMMAND name="definitions services set ip description"
	        help="Change service definition ip description">
		<PARAM name="id"
	                help="Resource Id for service definition ip"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc services set ip note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set ip --->
<!--- start set ah --->
	<COMMAND name="definitions services set ah"
                help="Change service definition ah"/>
	<COMMAND name="definitions services set ah name"
	        help="Change service definition ah name">
		<PARAM name="id"
	                help="Resource Id for service definition ah"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc services set ah name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set ah spi"
	        help="Change service definition ah spi">
		<PARAM name="id"
	                help="Resource Id for service definition ah"
	                ptype="NUM"/>
 		<PARAM name="spi"
                        help="Security Parameters Index"
                        ptype="PORT_NUM"
			default="256:4294967295"/>
	        <ACTION>exec /service/tools/definitions.exc services set ah spi "${id}" ${spi} 2>/dev/null</ACTION>
	</COMMAND>

	<COMMAND name="definitions services set ah description"
	        help="Change service definition ah description">
		<PARAM name="id"
	                help="Resource Id for service definition ah"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc services set ah note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set ah --->
<!--- start set esp --->
	<COMMAND name="definitions services set esp"
                help="Change service definition esp"/>
	<COMMAND name="definitions services set esp name"
	        help="Change service definition esp name">
		<PARAM name="id"
	                help="Resource Id for service definition esp"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc services set esp name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set esp spi"
	        help="Change service definition esp spi">
		<PARAM name="id"
	                help="Resource Id for service definition esp"
	                ptype="NUM"/>
 		<PARAM name="spi"
                        help="Security Parameters Index"
                        ptype="PORT_NUM"
			default="256:4294967295"/>
	        <ACTION>exec /service/tools/definitions.exc services set esp spi "${id}" ${spi} 2>/dev/null</ACTION>
	</COMMAND>

	<COMMAND name="definitions services set esp description"
	        help="Change service definition esp description">
		<PARAM name="id"
	                help="Resource Id for service definition esp"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc services set esp note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set esp --->
<!--- start set group --->
	<COMMAND name="definitions services set group"
                help="Change service definition group"/>
	<COMMAND name="definitions services set group name"
	        help="Change service definition group name">
		<PARAM name="id"
	                help="Resource Id for service definition group"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc services set group name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions services set group members"
	        help="Change service definition group members">
		<PARAM name="id"
	                help="Resource Id for service definition group"
	                ptype="NUM"/>
 		<PARAM name="members"
                        help="Definition resource Id"
                        ptype="MULTI_NUM"/>
	        <ACTION>exec /service/tools/definitions.exc services set group service "${id}" ${members} 2>/dev/null</ACTION>
	</COMMAND>

	<COMMAND name="definitions services set group description"
	        help="Change service definition group description">
		<PARAM name="id"
	                help="Resource Id for service definition group"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc services set group note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set group --->
<!--- end set --->
