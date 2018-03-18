DISTCHECK_CONFIGURE_FLAGS = --with-systemduserunitdir=$$dc_install_base/$(systemduserunitdir)
SUBDIRS = src
if HAVE_SYSTEMD
systemduserunit_DATA = systemd/tasks.socket systemd/tasks.service
dist_systemduserunit_DATA = systemd/tasks.socket systemd/tasks.service
endif

install-local-hook:
	systemctl --user daemon-reload

.SILENT:
