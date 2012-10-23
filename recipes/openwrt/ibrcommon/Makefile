#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ibrcommon
PKG_VERSION_MAJOR:=0
PKG_VERSION_MINOR:=8
PKG_VERSION_MICRO:=0

PKG_VERSION:=$(PKG_VERSION_MAJOR).$(PKG_VERSION_MINOR).$(PKG_VERSION_MICRO)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases

PKG_INSTALL:=1
PKG_FIXUP:=libtool
PKG_BUILD_DEPENDS:=libnl libopenssl

include $(INCLUDE_DIR)/package.mk

define Package/ibrcommon
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp +libpthread +librt +libnl +libopenssl
  TITLE:=IBR Common C++ Library
endef

CONFIGURE_ARGS += \
	--with-openssl

define Package/ibrcommon/description
 A library with common functions for C++.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/ibrcommon/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib$(PKG_NAME){.so,-$(PKG_VERSION_MAJOR).$(PKG_VERSION_MINOR).so.*} $(1)/usr/lib/
endef

$(eval $(call BuildPackage,ibrcommon))
