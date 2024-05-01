DOC_DIR := doc/man

ADOC_TO_MANPAGES := asciidoctor -b manpage

.PHONY: manpages
manpages:
	$(ADOC_TO_MANPAGES) $(wildcard $(DOC_DIR)/*.adoc)

clean::
	@$(RM) $(RMFLAGS) $(wildcard $(DOC_DIR)/*.3)
