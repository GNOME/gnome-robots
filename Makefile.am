SUBDIRS = data src

if BUILD_HELP
SUBDIRS += help
endif

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-for cfn in $(config_DATA); do \
	    cfn2=`echo $$cfn | sed -e 's/\.cfg//'`; \
            fname1=$(DESTDIR)$(scoredir)/gnobots2.$$cfn2.scores; \
            fname2=$(DESTDIR)$(scoredir)/gnobots2.$$cfn2-safe.scores; \
            fname3=$(DESTDIR)$(scoredir)/gnobots2.$$cfn2-super-safe.scores; \
	    touch $$fname1; chmod 664 $$fname1; chown $(scores_user):$(scores_group) $$fname1; \
	    touch $$fname2; chmod 664 $$fname2; chown $(scores_user):$(scores_group) $$fname2; \
	    touch $$fname3; chmod 664 $$fname3; chown $(scores_user):$(scores_group) $$fname3; \
	    done

-include $(top_srcdir)/git.mk
