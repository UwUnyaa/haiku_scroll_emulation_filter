middle_button_scroll_filter.so: middle_button_scroll_filter.cpp
	g++ -shared -o middle_button_scroll_filter.so middle_button_scroll_filter.cpp

caps_lock_control_filter.so: caps_lock_control_filter.cpp
	g++ -shared -o caps_lock_control_filter.so caps_lock_control_filter.cpp


install: middle_button_scroll_filter.so caps_lock_control_filter.so
	cp middle_button_scroll_filter.so /boot/home/config/non-packaged/add-ons/input_server/filters
	cp caps_lock_control_filter.so /boot/home/config/non-packaged/add-ons/input_server/filters
	/system/servers/input_server -q

install-capslock: caps_lock_control_filter.so
	cp caps_lock_control_filter.so /boot/home/config/non-packaged/add-ons/input_server/filters
	/system/servers/input_server -q

all: middle_button_scroll_filter.so caps_lock_control_filter.so
