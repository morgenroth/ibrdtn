#############################################################
#
#	tiny shell tool for tffs library                                                                     
#	(see libtffs package)
#
#############################################################
LIBTFFS_TSHELL_VERSION = 0.1
#LIBTFFS_TSHELL_SITE = http://tffs-lib.googlecode.com/svn/trunk/
LIBTFFS_TSHELL_SOURCE = libtffs-$(LIBTFFS_VERSION).tar
LIBTFFS_TSHELL_INSTALL_STAGING = NO
LIBTFFS_TSHELL_INSTALL_TARGET = YES
LIBTFFS_TSHELL_DEPENDENCIES = ncurses

define LIBTFFS_TSHELL_BUILD_CMDS
  $(TARGET_MAKE_ENV) CC="$(TARGET_CC)" LD="$(TARGET_LD)" $(MAKE) -C $(@D)
  $(TARGET_MAKE_ENV) CC="$(TARGET_CC)" LD="$(TARGET_LD)" $(MAKE) -C $(@D) tsh
endef

define LIBTFFS_TSHELL_INSTALL_STAGING_CMDS
  $(INSTALL) -D -m 0644 $(@D)/inc/tffs.h $(STAGING_DIR)/usr/include/tffs.h
  $(INSTALL) -D -m 0644 $(@D)/inc/comdef.h $(STAGING_DIR)/usr/include/comdef.h
endef

define LIBTFFS_TSHELL_INSTALL_TARGET_CMDS
  $(INSTALL) -D -m 0755 $(@D)/tsh $(TARGET_DIR)/usr/bin/tsh
endef

$(eval $(call GENTARGETS))

