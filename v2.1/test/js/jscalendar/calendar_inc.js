<script type='text/javascript'>
calendar=null;
function showCalendar(id) {
	var el = document.getElementById(id);
	if (calendar != null) {
		calendar.hide();
	} else {
		var cal = new Calendar(true, null, selected, closeHandler);
		cal.weekNumbers = false;
		cal.showsTime = true;
		cal.time24 = true;
		cal.showsOtherMonths = false;
		calendar = cal;
		cal.setRange(1900, 2070);
		cal.create();
	}

	calendar.setDateFormat('%Y-%m-%d %H:%M');
	calendar.parseDate(el.value);
	calendar.sel = el;
	calendar.showAtElement(el, "Br");
	return false;
}

function selected(cal, date) {
	cal.sel.value = date;
}
function closeHandler(cal) {
	cal.hide();
	calendar = null;
}
</script>