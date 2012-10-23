#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dtndht
PKG_VERSION_MAJOR:=0
PKG_VERSION_MINOR:=2
PKG_VERSION_MICRO:=3

PKG_VERSION:=$(PKG_VERSION_MAJOR).$(PKG_VERSION_MINOR).$(PKG_VERSION_MICRO)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://jenkins.ibr.cs.tu-bs.de/download

PKG_INSTALL:=1
PKG_FIXUP:=libtool
PKG_BUILD_DEPENDS:=libopenssl

include $(INCLUDE_DIR)/package.mk

define Package/dtndht
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libopenssl
  TITLE:=Library for DHT lookups for DTNs
endef

define Package/dtndht/description
 A library with DHT functions for DTN name lookups.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/dtndht/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib$(PKG_NAME){.so,-$(PKG_VERSION_MAJOR).$(PKG_VERSION_MINOR).so.*} $(1)/usr/lib/
endef

$(eval $(call BuildPackage,dtndht))
