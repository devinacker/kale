%.ips: %.asm xkas/xkas
	xkas/xkas -ips -o $@ $<

all: maphacks-us0.ips maphacks-us1.ips maphacks-eu.ips maphacks-de.ips maphacks-fc.ips maphacks-jp.ips

xkas/xkas:
	$(MAKE) -C xkas
