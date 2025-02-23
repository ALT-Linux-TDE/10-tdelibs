#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

// Input devices
#include <linux/input.h>

#define BITS_PER_LONG (sizeof(long) * 8)
#define NUM_BITS(x) ((((x) - 1) / BITS_PER_LONG) + 1)

void reply_Bool(DBusMessage* msg, DBusConnection* conn, int value) {
	DBusMessage* reply;
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_uint32_t serial = 0;

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);

	// add the arguments to the reply
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &value)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_append_basic failed\n", member);
		return;
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

void reply_CanSetGivenPath(DBusMessage* msg, DBusConnection* conn, const char* param) {
	DBusMessage* reply;
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_uint32_t serial = 0;
	int writable = false;

	// check if path is writable
	int rval = access (param, W_OK);
	if (rval == 0) {
		writable = true;
	}

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);

	// add the arguments to the reply
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &writable)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_append_basic failed\n", member);
		return;
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

void reply_SetGivenPath(DBusMessage* msg, DBusConnection* conn, const char* param, const char* contents) {
	DBusMessage* reply;
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_uint32_t serial = 0;
	int writable = false;
	int written = false;

	// check if path is writable
	int rval = access (param, W_OK);
	if (rval == 0) {
		writable = true;
	}

	if (writable) {
		FILE *node = fopen(param, "w");
		if (node != NULL) {
			if (fputs(contents, node) != EOF) {
				written = true;
			}
			if (fclose(node) == EOF) {
				// Error!
			}
		}
	}

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);

	// add the arguments to the reply
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &written)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_append_basic failed\n", member);
		return;
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

void reply_CanSetCPUGovernor(DBusMessage* msg, DBusConnection* conn) {
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_int32_t cpunum;
	char path[256];

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no argument supplied\n", member);
	}
	else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: argument not 32-bit integer\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &cpunum);
	}

	snprintf(path, 256, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpunum);
	reply_CanSetGivenPath(msg, conn, path);
}

void reply_SetCPUGovernor(DBusMessage* msg, DBusConnection* conn) {
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_int32_t cpunum = -1;
	char* governor = NULL;
	char path[256];

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no arguments supplied\n", member);
	}
	else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: first argument not 32-bit integer\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &cpunum);
	}

	if (!dbus_message_iter_next(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: second argument not supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: second argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &governor);
	}

	snprintf(path, 256, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpunum);
	if ((cpunum>-1) && governor) {
		reply_SetGivenPath(msg, conn, path, governor);
	}
	else {
		reply_Bool(msg, conn, false);
	}
}

void reply_CanSetBrightness(DBusMessage* msg, DBusConnection* conn) {
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	char* rawpath;
	char* safepath;

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no argument supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &rawpath);
	}

	safepath = realpath(rawpath, NULL);

	if (safepath &&
		(strstr(safepath, "/sys/devices") == safepath) &&
		(strstr(safepath, "/brightness") == (safepath+strlen(safepath)-strlen("/brightness")))
		) {
			reply_CanSetGivenPath(msg, conn, safepath);
	}
	else {
		reply_Bool(msg, conn, false);
	}

	free(safepath);
}

void reply_SetBrightness(DBusMessage* msg, DBusConnection* conn) {
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	char* rawpath;
	char* safepath;
	char* brightness;

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no arguments supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: first argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &rawpath);
	}

	if (!dbus_message_iter_next(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: second argument not supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: second argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &brightness);
	}

	safepath = realpath(rawpath, NULL);

	if (safepath && brightness &&
		(strstr(safepath, "/sys/devices") == safepath) &&
		(strstr(safepath, "/brightness") == (safepath+strlen(safepath)-strlen("/brightness")))
		) {
			reply_SetGivenPath(msg, conn, safepath, brightness);
	}
	else {
		reply_Bool(msg, conn, false);
	}

	free(safepath);
}

void reply_CanSetPower(DBusMessage* msg, DBusConnection* conn, char* state) {

	// check if path is writable
	int writable = false;
	int rval = access ("/sys/power/state", W_OK);
	if (rval == 0) {
		writable = true;
	}

	// check if method is supported
	int method = false;
	if (writable) {
		FILE *node = fopen("/sys/power/state", "r");
		if (node != NULL) {
			char *line = NULL;
			size_t len = 0;
			ssize_t read = getline(&line, &len, node);
			if (read > 0 && line) {
				method = strstr(line, state) != NULL;
				free(line);
			}
			if (fclose(node) == EOF) {
				// Error!
			}
		}
	}

	// send reply
	reply_Bool(msg, conn, writable && method);
}

void reply_SetPower(DBusMessage* msg, DBusConnection* conn, char* state) {

	// set power state
	reply_SetGivenPath(msg, conn, "/sys/power/state", state);
}

void reply_CanSetHibernation(DBusMessage* msg, DBusConnection* conn, char* state, char* disk) {

	// check if path is writable
	bool state_writable = false;
	int rval = access ("/sys/power/state", W_OK);
	if (rval == 0) {
		state_writable = true;
	}
	bool disk_writable = false;
	rval = access ("/sys/power/disk", W_OK);
	if (rval == 0) {
		disk_writable = true;
	}

	// check if method is supported
	bool method1 = false, method2 = false;
	if (state_writable && disk_writable) {
		FILE *statenode = fopen("/sys/power/state", "r");
		if (statenode != NULL) {
			char *line = NULL;
			size_t len = 0;
			ssize_t read = getline(&line, &len, statenode);
			if (read > 0 && line) {
				method1 = (strstr(line, state) != NULL);
				free(line);
			}
			if (fclose(statenode) == EOF) {
				// Error!
			}
		}
		FILE *disknode = fopen("/sys/power/disk", "r");
		if (disknode != NULL) {
			char *line = NULL;
			size_t len = 0;
			ssize_t read = getline(&line, &len, disknode);
			if (read > 0 && line) {
				method2 = (strstr(line, disk) != NULL);
				free(line);
			}
			if (fclose(disknode) == EOF) {
				// Error!
			}
		}
	}

	// send reply
	reply_Bool(msg, conn, state_writable && disk_writable && method1 && method2);
}

void reply_SetHibernation(DBusMessage* msg, DBusConnection* conn, char* state, char* disk) {
	// set hibernation state
	DBusMessage* reply;
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_uint32_t serial = 0;
	bool written1 = false, written2 = false;

	// check if path is writable
	int state_writable = false;
	int rval = access ("/sys/power/state", W_OK);
	if (rval == 0) {
		state_writable = true;
	}
	int disk_writable = false;
	rval = access ("/sys/power/disk", W_OK);
	if (rval == 0) {
		disk_writable = true;
	}
	
	if (state_writable && disk_writable) {
		FILE *disknode = fopen("/sys/power/disk", "w");
		if (disknode != NULL) {
			if (fputs(disk, disknode) != EOF) {
				written1 = true;
			}
			if (fclose(disknode) == EOF) {
				// Error!
			}
		}
		if (written1)
		{
			FILE *statenode = fopen("/sys/power/state", "w");
			if (statenode != NULL) {
				if (fputs(state, statenode) != EOF) {
					written2 = true;
				}
				if (fclose(statenode) == EOF) {
					// Error!
				}
			}
		}
	}

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);

	// add the arguments to the reply
	bool written = written1 && written2;
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &written)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_append_basic failed\n", member);
		return;
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

void reply_CanSetHibernationMethod(DBusMessage* msg, DBusConnection* conn) {

	// check if path is writable
	reply_CanSetGivenPath(msg, conn, "/sys/power/disk");
}

void reply_SetHibernationMethod(DBusMessage* msg, DBusConnection* conn) {
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	char* method = NULL;

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no arguments supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &method);
	}

	// set hibernation method
	if (method) {
		reply_SetGivenPath(msg, conn, "/sys/power/disk", method);
	}
	else {
		reply_Bool(msg, conn, false);
	}
}

void reply_InputEventsGetSwitches(DBusMessage* msg, DBusConnection* conn, bool active) {
	DBusMessage* reply;
	DBusMessageIter args, arrayIter;
	const char* member = dbus_message_get_member(msg);
	dbus_uint32_t serial = 0;
	char* rawpath;
	char* safepath;
	int fd, r;
	unsigned long switches[NUM_BITS(EV_CNT)];

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no argument supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &rawpath);
	}

	safepath = realpath(rawpath, NULL);

	if (safepath &&
		(strstr(safepath, "/dev/input/event") == safepath)
		) {

		fd = open(safepath, O_RDONLY);
		if( active ) {
			r = ioctl(fd, EVIOCGSW(sizeof(switches)), switches);
		}
		else {
			r = ioctl(fd, EVIOCGBIT(EV_SW, EV_CNT), switches);
		}
		if( r > 0 ) {
			dbus_uint32_t dSwitches[NUM_BITS(EV_CNT)];
			dbus_uint32_t *dSwitchesP = dSwitches;
			int i;

			// create a reply from the message
			reply = dbus_message_new_method_return(msg);

			// add the arguments to the reply
			for( i = 0; i < sizeof(switches)/sizeof(switches[0]); i++ ) {
				dSwitches[i] = switches[i];
			}
			dbus_message_iter_init_append(reply, &args);
			if (!dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "u", &arrayIter)) {
				fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_open_container failed\n", member);
				return;
			}
			if( !dbus_message_iter_append_fixed_array(&arrayIter, DBUS_TYPE_UINT32,
					&dSwitchesP, sizeof(switches)/sizeof(switches[0])) ) {
				fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_append_fixed_array failed\n", member);
				return;
			}
			if (!dbus_message_iter_close_container(&args, &arrayIter)) {
				fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_close_container failed\n", member);
				return;
			}
		}
		else {
			// create a reply from the message
			reply = dbus_message_new_error_printf(msg,
				"org.freedesktop.DBus.Error.NotSupported",
				"Event device \"%s\" not support EV_SW ioctl",
				safepath);
		}
		close(fd);
	}
	else {
		// create a reply from the message
		reply = dbus_message_new_error_printf(msg,
			"org.freedesktop.DBus.Error.InvalidArgs",
			"Event device \"%s\" is invalid",
			rawpath);
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);

	// free safepath
	free(safepath);
}

void signal_NameAcquired(DBusMessage* msg) {
	DBusMessageIter args;
	char *name = NULL;
	if(dbus_message_iter_init(msg, &args)) {
		if(DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args)) {
			dbus_message_iter_get_basic(&args, &name);
		}
	}
	fprintf(stderr, "[tde_dbus_hardwarecontrol] Name acquired: %s\n", name);
}

void reply_Introspect(DBusMessage* msg, DBusConnection* conn) {
	DBusMessage* reply;
	DBusMessageIter args;
	dbus_uint32_t serial = 0;
	size_t size = 4096;
	const char* member = dbus_message_get_member(msg);
	const char *path = dbus_message_get_path(msg);
	char *data = malloc(size);

	// compose reply
	strncpy(data,
		"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
		" \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n",
		size);
	strncat(data, "<node>\n", size-strlen(data));
	if(strcmp("/", path) == 0) {
		strncat(data, "  <node name=\"org\" />\n", size-strlen(data));
	}
	else if(strcmp("/org", path) == 0) {
		strncat(data, "  <node name=\"trinitydesktop\" />\n", size-strlen(data));
	}
	else if(strcmp("/org/trinitydesktop", path) == 0) {
		strncat(data, "  <node name=\"hardwarecontrol\" />\n", size-strlen(data));
	}
	else if(strcmp("/org/trinitydesktop/hardwarecontrol", path) == 0) {
		strncat(data,
			"  <interface name=\"org.trinitydesktop.hardwarecontrol.Brightness\">\n"
			"    <method name=\"CanSetBrightness\">\n"
			"      <arg name=\"device\" direction=\"in\" type=\"s\" />\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"SetBrightness\">\n"
			"      <arg name=\"device\" direction=\"in\" type=\"s\" />\n"
			"      <arg name=\"brightness\" direction=\"in\" type=\"s\" />\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"  </interface>\n",
			size-strlen(data));
		strncat(data,
			"  <interface name=\"org.trinitydesktop.hardwarecontrol.CPUGovernor\">\n"
			"    <method name=\"CanSetCPUGovernor\">\n"
			"      <arg name=\"cpu\" direction=\"in\" type=\"i\" />\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"SetCPUGovernor\">\n"
			"      <arg name=\"cpu\" direction=\"in\" type=\"i\" />\n"
			"      <arg name=\"governor\" direction=\"in\" type=\"s\" />\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"  </interface>\n",
			size-strlen(data));
		strncat(data,
			"  <interface name=\"org.trinitydesktop.hardwarecontrol.InputEvents\">\n"
			"    <method name=\"GetProvidedSwitches\">\n"
			"      <arg name=\"device\" direction=\"in\" type=\"s\" />\n"
			"      <arg name=\"value\" direction=\"out\" type=\"au\" />\n"
			"    </method>\n"
			"    <method name=\"GetActiveSwitches\">\n"
			"      <arg name=\"device\" direction=\"in\" type=\"s\" />\n"
			"      <arg name=\"value\" direction=\"out\" type=\"au\" />\n"
			"    </method>\n"
			"  </interface>\n",
			size-strlen(data));
		strncat(data,
			"  <interface name=\"org.trinitydesktop.hardwarecontrol.Power\">\n"
			"    <method name=\"CanStandby\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"Standby\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"CanFreeze\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"Freeze\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"CanSuspend\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"Suspend\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"CanHibernate\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"Hibernate\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"CanHybridSuspend\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"HybridSuspend\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"CanSetHibernationMethod\">\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"    <method name=\"SetHibernationMethod\">\n"
			"      <arg name=\"method\" direction=\"in\" type=\"s\" />\n"
			"      <arg name=\"value\" direction=\"out\" type=\"b\" />\n"
			"    </method>\n"
			"  </interface>\n",
			size-strlen(data));
	}
	strncat(data, "</node>\n", size-strlen(data));

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);

	// add the arguments to the reply
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &data)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_append_basic failed\n", member);
		return;
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
	free((void*)data);
}

void reply_PropertiesGetAll(DBusMessage* msg, DBusConnection* conn) {
	DBusMessage* reply;
	DBusMessageIter args, arrayIter;
	const char* member = dbus_message_get_member(msg);
	dbus_uint32_t serial = 0;

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);

	// add the arguments to the reply
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "sv", &arrayIter)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_open_container failed\n", member);
		return;
	}
	if (!dbus_message_iter_close_container(&args, &arrayIter)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_close_container failed\n", member);
		return;
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

void error_UnknownMessage(DBusMessage* msg, DBusConnection* conn) {
	DBusMessage* reply;
	dbus_uint32_t serial = 0;
	const char* member = dbus_message_get_member(msg);
	const char* interface = dbus_message_get_interface(msg);

	// print message
	fprintf(stderr, "[tde_dbus_hardwarecontrol] Unknown method '%s' called on interface '%s', ignoring\n", member, interface);
	if (DBUS_MESSAGE_TYPE_METHOD_CALL != dbus_message_get_type(msg)) {
		return;
	}

	// create a reply from the message
	reply = dbus_message_new_error_printf(msg,
		"org.freedesktop.DBus.Error.UnknownMethod",
		"Method \"%s\" on interface \"%s\" doesn't exist",
		member, interface);

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

void listen() {
	DBusMessage* msg;
	DBusConnection* conn;
	DBusError err;
	int ret;

	fprintf(stderr, "[tde_dbus_hardwarecontrol] Listening...\n");

	// initialise the error structure
	dbus_error_init(&err);

	// connect to the bus and check for errors
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] Connection failed with error '%s'\n", err.message);
		dbus_error_free(&err);
	}
	if (NULL == conn) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] No connection, exiting!\n");
		exit(1);
	}

	// request our name on the bus and check for errors
	ret = dbus_bus_request_name(conn, "org.trinitydesktop.hardwarecontrol", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] Name request failed with error '%s'\n", err.message);
		dbus_error_free(&err);
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] Not primary owner (%d), exiting!\n", ret);
		exit(1);
	}

	// loop, testing for new messages
	while (true) {
		// non blocking read of the next available message
		dbus_connection_read_write(conn, 1000); // block for up to 1 second
		msg = dbus_connection_pop_message(conn);

		// loop again if we haven't got a message
		if (NULL == msg) {
			continue;
		}

		// check this is a method call for the right interface & method
		if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.CPUGovernor", "CanSetCPUGovernor")) {
			reply_CanSetCPUGovernor(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.CPUGovernor", "SetCPUGovernor")) {
			reply_SetCPUGovernor(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Brightness", "CanSetBrightness")) {
			reply_CanSetBrightness(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Brightness", "SetBrightness")) {
			reply_SetBrightness(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "CanStandby")) {
			reply_CanSetPower(msg, conn, "standby");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "Standby")) {
			reply_SetPower(msg, conn, "standby");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "CanFreeze")) {
			reply_CanSetPower(msg, conn, "freeze");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "Freeze")) {
			reply_SetPower(msg, conn, "freeze");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "CanSuspend")) {
			reply_CanSetPower(msg, conn, "mem");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "Suspend")) {
			reply_SetPower(msg, conn, "mem");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "CanHibernate")) {
			reply_CanSetHibernation(msg, conn, "disk", "platform");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "Hibernate")) {
			reply_SetHibernation(msg, conn, "disk", "platform");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "CanHybridSuspend")) {
			reply_CanSetHibernation(msg, conn, "disk", "suspend");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "HybridSuspend")) {
			reply_SetHibernation(msg, conn, "disk", "suspend");
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "CanSetHibernationMethod")) {
			reply_CanSetHibernationMethod(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Power", "SetHibernationMethod")) {
			reply_SetHibernationMethod(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.InputEvents", "GetProvidedSwitches")) {
			reply_InputEventsGetSwitches(msg, conn, false);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.InputEvents", "GetActiveSwitches")) {
			reply_InputEventsGetSwitches(msg, conn, true);
		}
		else if (dbus_message_is_signal(msg, "org.freedesktop.DBus", "NameAcquired")) {
			signal_NameAcquired(msg);
		}
		else if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Introspectable", "Introspect")) {
			reply_Introspect(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.freedesktop.DBus.Properties", "GetAll")) {
			reply_PropertiesGetAll(msg, conn);
		}
		else {
			error_UnknownMessage(msg, conn);
		}

		// free the message
		dbus_message_unref(msg);
	}

	// close the connection
	dbus_connection_close(conn);
}

int main(int argc, char** argv) {
	listen();
	return 0;
}
