SHELL    = /bin/sh
PYTHON   = $(shell which python3)
BUILDDIR = rst/_build
VENV     = docenv

ifeq ($(shell which python3 >/dev/null 2>&1; echo $$?), 1)
$(error Python3 was not found! Please check README for further instructions)
endif

ifeq ($(shell which virtualenv >/dev/null 2>&1; echo $$?), 1)
$(error virtualenv was not found! Please check README for further instructions)
endif

.PHONY: help clean-all clean guide refman pdf all

help:
	@echo "Please use \`make <target>' where <target> is one of"
	@echo "  guide      to make HTML version of documentation using Sphinx"
	@echo "  refman     to make HTML version of reference manual using Doxygen"
	@echo "  pdf        to make PDF version of the reference manual"
	@echo "  clean      to remove all generated html and latex files"
	@echo "  clean-all  to reset the entire build environment"

clean-all: clean
	-rm -rf $(VENV) 

clean:
	-rm -rf $(BUILDDIR) guide refman latex refman.pdf

refman:
	doxygen

pdf:	refman
	make -C latex pdf
	ln -sf latex/refman.pdf refman.pdf

guide: $(VENV)
	@(\
		. $(VENV)/bin/activate ;\
		${MAKE} -C rst html;\
		deactivate ;\
	)
	ln -s $(BUILDDIR)/html guide
	@echo "Admin guide build finished. The HTML pages are in 'guide'."

$(VENV):
	@( \
		virtualenv -p $(PYTHON) $(VENV); \
		. $(VENV)/bin/activate; \
		pip install Sphinx; \
		pip install sphinxcontrib-images; \
		deactivate;\
	)


