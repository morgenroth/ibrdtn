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

define LIBTFFS_BUILD_CMDS
  $(TARGET_MAKE_ENV) CC="$(TARGET_CC)" LD="$(TARGET_LD)" $(MAKE) -C $(@D)
endef

define LIBTFFS_INSTALL_STAGING_CMDS
  $(INSTALL) -D -m 0644 $(@D)/inc/tffs.h $(STAGING_DIR)/usr/include/tffs.h
  $(INSTALL) -D -m 0644 $(@D)/inc/comdef.h $(STAGING_DIR)/usr/include/comdef.h
  $(INSTALL) -D -m 0755 $(@D)/libtffs.a $(STAGING_DIR)/usr/lib/libtffs.a
endef

define LIBTFFS_INSTALL_TARGET_CMDS
  $(INSTALL) -D -m 0755 $(@D)/libtffs.a $(TARGET_DIR)/usr/lib/libtffs.a
endef

$(eval $(generic-package))
