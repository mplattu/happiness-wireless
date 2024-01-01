build:
	pio run --environment wemosd1

upload:
	pio run --environment wemosd1 -t upload

monitor:
	pio device monitor

clean:
	rm -fR .pio/
