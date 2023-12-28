//Modify this file to change what commands output to your statusbar, and 
//recompile using the make command.
//
static const Block blocks[] = {
	/*Icon*/	/*Command*/		/*Update Interval*/	/*Update Signal*/
    //{"" ,"pomodoro read",   0,                                 8},
    {"   "   ,  "status_battery",  60,                               9},
    {"  🕒 ",  "status_time",     60,                               10},
    {"  🔊 ", "status_volume 3",   0,                                11},
    {"  ", "status_storage",   60,                                12},
    {"  ", "status_ram",   20,                                13},
    {"  🌐 ", "status_ip",   600,                                14}, // Each hour
    {"  ", "notifications get",   60,                                2},

	//{"", "cat ~/.pacupdate | sed /📦0/d",					0,		9},
	
	//{"🧠", "free -h | awk '/^Mem/ { print $3\"/\"$2 }' | sed s/i//g",	30,		0},

	//{"", "~/bin/statusbar/volume",						0,		10},

	//{"☀", "xbacklight | sed 's/\\..*//'",					0,		11},
	
	//{"", "~/bin/statusbar/battery",						5,		0},

	//{"🌡", "sensors | awk '/^temp1:/{print $2}'",				5,		0},

	//{"", "~/bin/statusbar/clock",						5,		0},
};

//sets delimeter between status commands. NULL character ('\0') means no delimeter.
static char delim = ' ';
