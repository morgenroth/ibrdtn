AUTOMAKE_OPTIONS = foreign
SUBDIRS = src doc man
ACLOCAL_AMFLAGS = -I m4

if ENABLE_BASH_COMPLETION
bashcompletiondir = $(BASH_COMPLETION_DIR)
dist_bashcompletion_DATA = scripts/bash_completion/ibrdtn-tools
endif

# debian files for package building
EXTRA_DIST= \
	debian/changelog \
	debian/compat \
	debian/control \
	debian/copyright \
	debian/ibrdtn-tools.docs \
	debian/ibrdtn-tools.install \
	debian/rules \
	debian/source \
	debian/source/format

