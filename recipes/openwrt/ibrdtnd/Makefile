#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ibrdtnd
PKG_VERSION_MAJOR:=0
PKG_VERSION_MINOR:=10
PKG_VERSION_MICRO:=0

PKG_VERSION:=$(PKG_VERSION_MAJOR).$(PKG_VERSION_MINOR).$(PKG_VERSION_MICRO)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=dtndht ibrdtn libopenssl libsqlite3

include $(INCLUDE_DIR)/package.mk

define Package/ibrdtnd
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+dtndht +ibrdtn +libopenssl +libsqlite3
  TITLE:=DTN Deamon
endef

define Package/ibrdtnd/conffiles
/etc/config/ibrdtn
endef

define Package/ibrdtnd/description
 The implementation of the bundle protocol of the IBR (TU Braunschweig).
endef

CONFIGURE_ARGS += \
        --with-tls --with-dtnsec --with-sqlite --with-compression --with-dht

define Package/ibrdtnd/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/dtnd $(1)/usr/sbin/
	$(INSTALL_BIN) files/safety-wrapper.sh $(1)/usr/sbin/dtnd-safety-wrapper.sh
	$(INSTALL_DIR) $(1)/usr/share/ibrdtn/
	$(INSTALL_BIN) files/build-config.sh $(1)/usr/share/ibrdtn/build-config.sh
	$(INSTALL_BIN) files/mkcontainer.sh $(1)/usr/share/ibrdtn/mkcontainer.sh
	$(INSTALL_BIN) files/mountcontainer.sh $(1)/usr/share/ibrdtn/mountcontainer.sh
	$(INSTALL_BIN) files/systemcheck.sh $(1)/usr/share/ibrdtn/systemcheck.sh
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/ibrdtn.init $(1)/etc/init.d/ibrdtn
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/ibrdtn.uci $(1)/etc/config/ibrdtn
endef

$(eval $(call BuildPackage,ibrdtnd))
