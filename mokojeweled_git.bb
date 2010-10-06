DESCRIPTION = "A bejeweled clone in Elementary"
HOMEPAGE = "http://gitorious.org/mokosuite2/mokojeweled"
AUTHOR = "Daniele Ricci"
LICENSE = "GPLv3"
DEPENDS = "elementary"
SECTION = "x11/games"

PV = "0.1+gitr${SRCPV}"
PR = "r1"
#SRCREV = "a84ecfab2f2e214aa04274a6f1a4a77b7b7c6cc3"

SRC_URI = "git://gitorious.org/mokosuite2/mokojeweled.git;protocol=git"
S = "${WORKDIR}/git"

CFLAGS += "-DOPENMOKO"
EXTRA_OECONF = " --with-edje-cc=${STAGING_BINDIR_NATIVE}/edje_cc"
FILES_${PN} += "${datadir}/mokojeweled ${datadir}/applications ${datadir}/pixmaps"

inherit pkgconfig autotools
