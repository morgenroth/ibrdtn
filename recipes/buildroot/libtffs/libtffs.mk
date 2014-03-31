#############################################################
#
# libtffs (tffs-lib is a Tiny FAT File System library.)
#
#############################################################
LIBTFFS_VERSION = 3
LIBTFFS_SITE = http://tffs-lib.googlecode.com/svn/trunk/
LIBTFFS_SITE_METHOD=svn
LIBTFFS_INSTALL_STAGING = YES
LIBTFFS_INSTALL_TARGET = YES

ifeq ($(BR2_PACKAGE_LIBTFFS_TSH),y)
  MAKE_TARGET = all
  LIBTFFS_DEPENDENCIES += ncurses
else
  MAKE_TARGET = lib
endif

define LIBTFFS_BUILD_CMDS
  $(TARGET_MAKE_ENV) CC="$(TARGET_CC)" LD="$(TARGET_LD)" $(MAKE) -C $(@D) $(MAKE_TARGET)
endef

define LIBTFFS_INSTALL_STAGING_CMDS
  $(INSTALL) -D -m 0644 $(@D)/inc/tffs.h $(STAGING_DIR)/usr/include/tffs.h
  $(INSTALL) -D -m 0644 $(@D)/inc/comdef.h $(STAGING_DIR)/usr/include/comdef.h
  $(INSTALL) -D -m 0755 $(@D)/libtffs.a $(STAGING_DIR)/usr/lib/libtffs.a
endef


ifeq ($(BR2_PACKAGE_LIBTFFS_TSH),y)
  define LIBTFFS_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/libtffs.a $(TARGET_DIR)/usr/lib/libtffs.a
    $(INSTALL) -D -m 0755 $(@D)/tsh $(TARGET_DIR)/usr/bin/tsh
  endef
else
  define LIBTFFS_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/libtffs.a $(TARGET_DIR)/usr/lib/libtffs.a
  endef
endif
$(eval $(generic-package))
