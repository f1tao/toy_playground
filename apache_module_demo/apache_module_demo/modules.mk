mod_apache_module_demo.la: mod_apache_module_demo.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_apache_module_demo.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_apache_module_demo.la
