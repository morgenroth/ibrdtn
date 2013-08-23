docdir = $(datadir)/doc/@PACKAGE@

docfiles = api.tex BPSEC.txt README.txt

if HAVE_PDFLATEX
doc_DATA = api.pdf
MANNAME = api
MANTEXSRC = $(MANNAME).tex
MANAUX = $(MANNAME).aux
MANPDF = $(MANNAME).pdf

CLEANFILES = $(MANPDF) $(MANNAME).log $(MANNAME).idx $(MANNAME).out \
 $(MANNAME).toc $(MANAUX)

$(MANPDF): $(srcdir)/$(MANTEXSRC)
	$(PDFLATEX) $<
endif

dist_doc_DATA = $(docfiles)
EXTRA_DIST = win32
