middle_button_scroll_filter.so: middle_button_scroll_filter.cpp
	gcc -shared -o middle_button_scroll_filter.so middle_button_scroll_filter.cpp

install: middle_button_scroll_filter.so
	cp middle_button_scroll_filter.so /boot/home/config/non-packaged/add-ons/input_server/filters
	/system/servers/input_server -q

all: middle_button_scroll_filter.so
