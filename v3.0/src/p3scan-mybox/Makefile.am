doc_DOC = AUTHORS ChangeLog CONTRIBUTERS LICENSE NEWS p3scan.sh README README-emergency \
README-ripmime README-rpm spamfaq.html TODO

man8_MANS = man/p3scan.man man/p3scan_readme.man

EXTRA_DIST = CONTRIBUTERS LICENSE p3scan.sh README-emergency README-ripmime README-rpm \
spamfaq.html etc $(man8_MANS)

SUBDIRS = src

sysconf_DATA = etc/p3scan*

DISTCHECK_CONFIGURE_FLAGS = --disable-ripmime --disable-clamav
DIST_CONFIGURE_FLAGS = --disable-ripmime --disable-clamav

P3USER = @P3USER@

install-exec-hook:
	test -z "$(DESTDIR)$(sysconfdir)/p3scan" || \
	$(mkinstalldirs) "$(DESTDIR)$(sysconfdir)/p3scan"
	mv -f $(DESTDIR)$(sysconfdir)/p3scan.conf.sample $(DESTDIR)$(sysconfdir)/p3scan
	if test -f "$(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf"; then \
		mv -f "$(DESTDIR)$(sysconfdir)/p3scan.conf" "$(sysconfdir)/p3scan/p3scan.conf.sample"; \
	else mv -f "$(DESTDIR)$(sysconfdir)/p3scan.conf" "$(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf"; \
	fi
	mv -f $(DESTDIR)$(sysconfdir)/p3scan*.mail $(DESTDIR)$(sysconfdir)/p3scan
	mv -f $(DESTDIR)$(sysconfdir)/p3scan.*list $(DESTDIR)$(sysconfdir)/p3scan
	test -z "$(DESTDIR)$(sysconfdir)/p3scan" || \
	$(mkinstalldirs) "$(DESTDIR)$(sysconfdir)/${P3INIT}"
	cp etc/${P3INIT}/* $(DESTDIR)$(sysconfdir)/${P3INIT}
	test -z "$(DESTDIR)${P3DOCS}/$(PACKAGE)-$(VERSION)" || \
	$(mkinstalldirs) $(DESTDIR)${P3DOCS}/$(PACKAGE)-$(VERSION)
	cp $(doc_DOC) $(DESTDIR)${P3DOCS}/$(PACKAGE)-$(VERSION)

install-data-hook:
	test -z "$(DESTDIR)$(localstatedir)/run/p3scan" || \
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/run/p3scan
	test -z "$(DESTDIR)$(localstatedir)/spool/p3scan" || \
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/spool/p3scan
	test -z "$(DESTDIR)$(localstatedir)/spool/p3scan/notify" || \
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/spool/p3scan/notify
	test -z "$(DESTDIR)$(localstatedir)/spool/p3scan/children" || \
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/spool/p3scan/children
	sed "s|:version|${VERSION}|g; \
	s|:sysconfdir|$(sysconfdir)|g; \
	s|:localstatedir|$(localstatedir)|g; \
	s|:prefix|${prefix}|g; \
	s|:bindir|$(bindir)|g; \
	s|:mandir|${mandir}|g; \
	s|:P3DOCS|${P3DOCS}|g" \
	$(DESTDIR)$(man8dir)/p3scan.8 \
	> $(DESTDIR)$(man8dir)/p3scan.8.new
	mv -f $(DESTDIR)$(man8dir)/p3scan.8.new $(DESTDIR)$(man8dir)/p3scan.8
	sed "s|:version|${VERSION}|g; \
	s|:sysconfdir|$(sysconfdir)|g; \
	s|:localstatedir|$(localstatedir)|g; \
	s|:prefix|${prefix}|g; \
	s|:bindir|$(bindir)|g; \
	s|:mandir|${mandir}|g; \
	s|:P3USER|${P3USER}|g; \
	s|:P3DOCS|${P3DOCS}|g" \
	$(DESTDIR)$(man8dir)/p3scan_readme.8 \
	> $(DESTDIR)$(man8dir)/p3scan_readme.8.new
	mv -f $(DESTDIR)$(man8dir)/p3scan_readme.8.new $(DESTDIR)$(man8dir)/p3scan_readme.8
	gzip -q -f -9 $(DESTDIR)$(man8dir)/p3scan*
	sed "s|:virusdir|$(localstatedir)/spool/p3scan|g; \
	s|:sysconfdir|$(sysconfdir)|g; \
	s|:P3DOCS|${P3DOCS}|g; \
	s|:version|${VERSION}|g; \
	s|:localstatedir|$(localstatedir)|g" \
	$(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf \
	> $(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf.new
	mv -f $(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf.new $(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf
	sed "s|:virusdir|$(localstatedir)/spool/p3scan|g; \
	s|:sysconfdir|$(sysconfdir)|g; \
	s|:P3DOCS|${P3DOCS}|g; \
	s|:version|${VERSION}|g; \
	s|:localstatedir|$(localstatedir)|g" \
	$(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf.sample \
	> $(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf.sample.new
	mv -f $(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf.sample.new $(DESTDIR)$(sysconfdir)/p3scan/p3scan.conf.sample
	@if test `id|sed -e 's/^.[^(]*(\([^)]*\)).*/\1/'` = "root" && test -n "${P3USER}"; then \
		chown -f -R ${P3USER}:${P3USER} $(DESTDIR)$(sysconfdir)/p3scan; \
		chown -f ${P3USER}:${P3USER} $(DESTDIR)$(localstatedir)/run/p3scan; \
		chown -f -R ${P3USER}:${P3USER} $(DESTDIR)$(localstatedir)/spool/p3scan; \
	fi
	chmod 0755 $(DESTDIR)$(sysconfdir)/p3scan
	chmod 0600 $(DESTDIR)$(sysconfdir)/p3scan/*
	chmod 0700 $(DESTDIR)$(localstatedir)/run/p3scan
	chmod -R 0700 $(DESTDIR)$(localstatedir)/spool/p3scan
