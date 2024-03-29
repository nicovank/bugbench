# Normally you do not need to remake the files here. But if you want
# to you will need the GNU TeX-info utilities. To make a Postscript
# files you also need TeX and dvips. To make the PDF file you will
# need pdftex. We use the teTeX distribution for all of these.

# We avoid default automake rules because of problems with .dvi file
# and BSD makes

# If you know how to fix any of this more elegantly please mail
# docs@mysql.com

TEXI2HTML_FLAGS =	-iso -number
DVIPS =			dvips
MAKEINFO =		@MAKEINFO@
TEXINFO_TEX =		Support/texinfo.tex

noinst_SCRIPTS =	Support/texi2html Support/generate-text-files.pl \
			Support/generate-mirror-listing.pl
info_TEXINFOS =		manual.texi

targets =		manual.txt mysql.info manual.html

BUILT_SOURCES =		$(targets) manual_toc.html include.texi
EXTRA_DIST =		$(noinst_SCRIPTS) $(BUILT_SOURCES) mysqld_error.txt INSTALL-BINARY reservedwords.texi

all:			$(targets) txt_files

txt_files:		../INSTALL-SOURCE ../COPYING ../COPYING.LIB \
			INSTALL-BINARY

CLEAN_FILES:		$(BUILD_SOURCES)
	touch $(BUILD_SOURCES)

# The PostScript and PDF version are so big that they are not included in the
# standard distribution. It is available for download from the home page.  
paper: manual_a4.ps manual_letter.ps  $(PDFMANUAL)

#########################################################################

# The Makefile contains the previous version so we can not use that
include.texi:	../configure.in
	echo "@c This file is autogenerated by the Makefile" > $@
	echo -n "@set mysql_version " >> $@
	grep "AM_INIT_AUTOMAKE(mysql, " ../configure.in | \
	 sed -e 's;AM_INIT_AUTOMAKE(mysql, ;;' -e 's;);;' >> $@
	echo -n "@set default_port " >> $@
	grep "MYSQL_TCP_PORT_DEFAULT=" ../configure.in | \
	 sed -e 's;MYSQL_TCP_PORT_DEFAULT=;;' >> $@

mysql.info:	manual.texi include.texi
	cd $(srcdir) && $(MAKEINFO) --no-split -I $(srcdir) $<

manual.txt:	manual.texi include.texi
	cd $(srcdir) && \
	$(MAKEINFO) -I $(srcdir) --no-headers --no-split --output $@ $<

manual.html:	manual.texi include.texi $(srcdir)/Support/texi2html
	cd $(srcdir) && @PERL@ $(srcdir)/Support/texi2html $(TEXI2HTML_FLAGS) $<

manual_toc.html: manual.html


# Fix: add --output-comment with some interesting info?
# Fix: @image worked with a older version of pdftex. 
# Note: @image will work if we first convert all images to pdf ...
#       is that worth it?
# Comment: We need to run pdftex 2 times to get the cross references right.
manual.pdf: manual.texi
	cat manual.texi | sed -e 's|@image{[^}]*} *||g' > manual-tmp.texi
	pdftex --interaction=nonstopmode manual-tmp.texi
	texindex manual-tmp.??
	pdftex --interaction=nonstopmode manual-tmp.texi
	texindex manual-tmp.??
	pdftex --interaction=nonstopmode manual-tmp.texi
	mv manual-tmp.pdf manual.pdf
	rm -f manual-tmp.*
	touch $@

# Target to produce NuSphere Manual
nusphere.pdf: manual.texi
	cat manual.texi \
	| sed -e 's/@example/@smallexample/g' \
	      -e 's/@end example/@end smallexample/g' \
	      -e 's/@c ifnusphere //g' \
	      -e 's|@image{[^}]*} *||g' \
	> manual-tmp.texi
	pdftex --interaction=nonstopmode manual-tmp.texi
	texindex manual-tmp.??
	pdftex --interaction=nonstopmode manual-tmp.texi
	texindex manual-tmp.??
	pdftex --interaction=nonstopmode manual-tmp.texi
	mv manual-tmp.pdf nusphere.pdf
	rm -f manual-tmp.*
	touch $@

# Target to produce DocBook XML
mysql.xml: manual.texi include.texi
	$(MAKEINFO) --force --no-ifinfo --docbook manual.texi

# The texi2dvi gives a lot of harmless errors. Just ignore them unless
# you want to help with the typesetting part.
# This is the European papersize version
manual_a4.ps:	manual.texi include.texi
	TEXINPUTS=$(srcdir):$$TEXINPUTS \
	  MAKEINFO='$(MAKEINFO) -I $(srcdir)' \
	    $(TEXI2DVI) --batch --texinfo --quiet '@afourpaper' $<
	$(DVIPS) -t a4 manual.dvi -o $@
	touch $@

# This is the American papersize version
manual_letter.ps:	manual.texi include.texi
	TEXINPUTS=$(srcdir):$$TEXINPUTS \
	  MAKEINFO='$(MAKEINFO) -I $(srcdir)' \
	    $(TEXI2DVI) --batch $<
	$(DVIPS) -t letter manual.dvi -o $@
	touch $@

# Include images for the manual in the distribution
dist-hook:
	BD=`cd $(top_srcdir); pwd`; \
	if test ! -d $(distdir)/Flags; then mkdir -p $(distdir)/Flags; fi; \
	$(INSTALL_DATA) $(srcdir)/Flags/*.gif $(srcdir)/Flags/*.eps \
		$(srcdir)/Flags/*.txt $(distdir)/Flags;
	echo "PostScript and PDF versions suitable for printing" \
		> $(distdir)/manual.ps
	echo "are available from http://www.mysql.com/Manual/" \
		>> $(distdir)/manual.ps
	echo "or any mirror site" \
		>> $(distdir)/manual.ps

GT = $(srcdir)/Support/generate-text-files.pl

../INSTALL-SOURCE:	mysql.info $(GT)
	perl -w $(GT) mysql.info "Installing" "Compatibility" > $@

# We put the description for the binary installation here so that
# people who download source wont have to see it. It is moved up to
# the toplevel by the script that makes the binary tar files.
INSTALL-BINARY:	mysql.info $(GT)
	perl -w $(GT) mysql.info "Installing binary" "Installing source" > $@

../COPYING:		mysql.info $(GT)
	perl -w $(GT) mysql.info "GPL license" "LGPL license" > $@

../COPYING.LIB:		mysql.info $(GT)
	perl -w $(GT) mysql.info "LGPL license" "Function Index" > $@

# Don't update the files from bitkeeper
%::SCCS/s.%
