#
# Regular cron jobs for the forest package
#
0 4	* * *	root	[ -x /usr/bin/forest_maintenance ] && /usr/bin/forest_maintenance
