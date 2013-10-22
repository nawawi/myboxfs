<!--- start print --->
	<COMMAND name="definitions timeevents print"
                help="Display Time Event definition list">
		<PARAM name="option"
			help="Display option"
			ptype="DT_OPTION"
			default="all"/>
                <ACTION>exec /service/tools/definitions.exc time_events print ${option} 2>/dev/null |more -dl</ACTION>
        </COMMAND>
<!--- end print --->
<!--- start find --->
	<COMMAND name="definitions timeevents find"
                help="Search time event definition">
		<PARAM name="string"
			help="String to find"
			ptype="STRING_ANY" />
		<PARAM name="option"
			help="Display option"
			ptype="DT_OPTION"
			default="all"/>
                <ACTION>exec /service/tools/definitions.exc time_events find "${string}" ${option} 2>/dev/null |more -dl</ACTION>
        </COMMAND>
<!--- end find --->
<!--- start add --->
	<COMMAND name="definitions timeevents add"
                help="Add new Time Event definition"/>
<!--- start add recurring --->
	<COMMAND name="definitions timeevents add recurring"
                help="Add new Recurring Event definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="starttime"
                        help="Start time"
                        ptype="HOUR_MIN"/>
 		<PARAM name="endtime"
                        help="End time"
                        ptype="HOUR_MIN"/>
 		<PARAM name="days"
                        help="Weekdays"
                        ptype="DAY"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc time_events add recurring "${name}" ${starttime} ${endtime} ${days} "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add recurring --->

<!--- start add single --->
	<COMMAND name="definitions timeevents add single"
                help="Add new Single Event definition">
 		<PARAM name="name"
                        help="Name"
                        ptype="STRING_ANY"/>
 		<PARAM name="startdate"
                        help="Start date"
                        ptype="DATE"/>
 		<PARAM name="enddate"
                        help="End date"
                        ptype="DATE"/>
 		<PARAM name="starttime"
                        help="Start time"
                        ptype="HOUR_MIN"/>
 		<PARAM name="endtime"
                        help="End time"
                        ptype="HOUR_MIN"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
                <ACTION>exec /service/tools/definitions.exc time_events add single "${name}" ${startdate} ${enddate} ${starttime} ${endtime} "${description}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end add single --->

<!--- start delete --->
	<COMMAND name="definitions timeevents delete"
                help="Delete Time Event definition">
		<PARAM name="id"
			help="Resource Id '*' for all"
			ptype="MULTI_NUM_ALL" />
                <ACTION>exec /service/tools/definitions.exc time_events delete "${id}" 2>/dev/null</ACTION>
        </COMMAND>
<!--- end delete --->
<!--- start set --->
	<COMMAND name="definitions timeevents set"
                help="Change Time Event definition setting"/>
<!--- start set recurring --->
	<COMMAND name="definitions timeevents set recurring"
                help="Change Time Event definition recurring"/>
	<COMMAND name="definitions timeevents set recurring name"
	        help="Change Time Event definition recurring name">
		<PARAM name="id"
	                help="Resource Id for Time Event definition recurring"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc time_events set recurring name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions timeevents set recurring start-time"
	        help="Change Time Event definition recurring start-time">
		<PARAM name="id"
	                help="Resource Id for Time Event definition recurring"
	                ptype="NUM"/>
 		<PARAM name="time"
                        help="Start time"
                        ptype="HOUR_MIN"/>
	        <ACTION>exec /service/tools/definitions.exc time_events set recurring stime "${id}" "${time}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions timeevents set recurring end-time"
	        help="Change Time Event definition recurring end-time">
		<PARAM name="id"
	                help="Resource Id for Time Event definition recurring"
	                ptype="NUM"/>
 		<PARAM name="time"
                        help="End time"
                        ptype="HOUR_MIN"/>
	        <ACTION>exec /service/tools/definitions.exc time_events set recurring etime "${id}" "${time}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions timeevents set recurring weekdays"
	        help="Change Time Event definition recurring weekdays">
		<PARAM name="id"
	                help="Resource Id for Time Event definition recurring"
	                ptype="NUM"/>
 		<PARAM name="days"
                        help="Weekdays"
                        ptype="DAY"/>
	        <ACTION>exec /service/tools/definitions.exc time_events set recurring wdays "${id}" "${days}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions timeevents set recurring description"
	        help="Change Time Event definition recurring description">
		<PARAM name="id"
	                help="Resource Id for Time Event definition recurring"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc time_events set recurring note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set recurring --->
<!--- start set single --->
	<COMMAND name="definitions timeevents set single"
                help="Change Time Event definition single"/>
	<COMMAND name="definitions timeevents set single name"
	        help="Change Time Event definition single name">
		<PARAM name="id"
	                help="Resource Id for Time Event definition single"
	                ptype="NUM"/>
		<PARAM name="name"
	                help="Definition name"
	                ptype="STRING_ANY"/>
	        <ACTION>exec /service/tools/definitions.exc time_events set single name "${id}" "${name}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions timeevents set single start-date"
	        help="Change Time Event definition single start-date">
		<PARAM name="id"
	                help="Resource Id for Time Event definition single"
	                ptype="NUM"/>
 		<PARAM name="date"
                        help="Start date"
                        ptype="DATE"/>
	        <ACTION>exec /service/tools/definitions.exc time_events set single sdate "${id}" "${date}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions timeevents set single end-date"
	        help="Change Time Event definition single end-date">
		<PARAM name="id"
	                help="Resource Id for Time Event definition single"
	                ptype="NUM"/>
 		<PARAM name="date"
                        help="End date"
                        ptype="DATE"/>
	        <ACTION>exec /service/tools/definitions.exc time_events set single edate "${id}" "${date}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions timeevents set single start-time"
	        help="Change Time Event definition single start-time">
		<PARAM name="id"
	                help="Resource Id for Time Event definition single"
	                ptype="NUM"/>
 		<PARAM name="time"
                        help="Start time"
                        ptype="HOUR_MIN"/>
	        <ACTION>exec /service/tools/definitions.exc time_events set single stime "${id}" "${time}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions timeevents set single end-time"
	        help="Change Time Event definition single end-time">
		<PARAM name="id"
	                help="Resource Id for Time Event definition single"
	                ptype="NUM"/>
 		<PARAM name="time"
                        help="End time"
                        ptype="HOUR_MIN"/>
	        <ACTION>exec /service/tools/definitions.exc time_events set single etime "${id}" "${time}" 2>/dev/null</ACTION>
	</COMMAND>
	<COMMAND name="definitions timeevents set single description"
	        help="Change Time Event definition single description">
		<PARAM name="id"
	                help="Resource Id for Time Event definition single"
	                ptype="NUM"/>
 		<PARAM name="description"
                        help="Description"
                        ptype="STRING_ANY"
			default=""/>
	        <ACTION>exec /service/tools/definitions.exc time_events set single note "${id}" "${description}" 2>/dev/null</ACTION>
	</COMMAND>
<!--- end set single --->
<!--- end set --->
