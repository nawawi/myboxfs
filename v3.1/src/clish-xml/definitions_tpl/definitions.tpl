<COMMAND name="definitions"
	help="Definitions configuration"/>

<COMMAND name="definitions networks"
	help="Network definition">
	<ACTION>exec /service/tools/definitions.exc networks print all 2>/dev/null |more -dl</ACTION>
</COMMAND>
<COMMAND name="definitions services"
	help="Service definition">
	<ACTION>exec /service/tools/definitions.exc services print all 2>/dev/null |more -dl</ACTION>
</COMMAND>
<COMMAND name="definitions timeevents"
	help="Time Events definition">
	<ACTION>exec /service/tools/definitions.exc time_events print all 2>/dev/null |more -dl</ACTION>
</COMMAND>
