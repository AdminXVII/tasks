DISTCHECK_CONFIGURE_FLAGS = --with-systemduserunitdir=$$dc_install_base/$(systemduserunitdir)
SUBDIRS = src
sysconf_DATA = cert.pem
if HAVE_SYSTEMD
systemduserunit_DATA = systemd/tasks.socket systemd/tasks.service
dist_systemduserunit_DATA = systemd/tasks.socket systemd/tasks.service
endif

cert.pem:
	printf "Creating default certificate\n"
	openssl req -newkey rsa:4096 -nodes -keyout cert.key -x509 -days 365 -out cert.pem
	cat cert.key >> cert.pem
	rm -f cert.key

clean-local:
	-rm -f cert.pem

install-local-hook:
	systemctl --user daemon-reload

.SILENT: