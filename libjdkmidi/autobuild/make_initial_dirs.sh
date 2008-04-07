#! /bin/sh

PROJECT="$1"

if [ -z "$PROJECT" ]; then
    PROJECT=example
fi

relative_dir="$(dirname "$0")"
PROJECT_TOP_DIR="$(cd ${relative_dir}/.. && pwd)"

cd "$PROJECT_TOP_DIR"
mkdir -p include src docs/html tests tools gui examples
ln -s autobuild/GNUmakefile
ln -s autobuild/package
ln -s autobuild/configure
ln -s ../autobuild/docs/Doxyfile docs/
touch LICENSE.txt
touch README.txt

(
    echo "PROJECT=$PROJECT"
    echo "PROJECT_NAME=$PROJECT"
    echo "PROJECT_MAINTAINER=project_maintainer"
    echo "PROJECT_COPYRIGHT=project_copyright"
    echo "PROJECT_EMAIL=$USER@$HOSTNAME"
    echo "PROJECT_LICENSE=private"
    echo "PROJECT_LICENSE_FILE=\$(PROJECT_TOP_DIR)/LICENSE.txt"
    echo "PROJECT_README_FILE=\$(PROJECT_TOP_DIR)/README.txt"
    echo "PROJECT_COMPANY=project_company"
    echo "PROJECT_WEBSITE=project_website"
    echo "PROJECT_DESCRIPTION=project_description"
    echo "PROJECT_VERSION=0.0"
    echo "TOP_LIB_DIRS+=."
    echo "SUBLIBS="
    echo "INCLUDES+="
    echo "DEFINES+="
    echo "COMPILE_FLAGS+=-Wall"
    echo "LINK_FLAGS+="
    echo "PKGCONFIG_PACKAGES+="
    echo "CONFIG_TOOLS+="
) >project.mak

(
    echo "PROJECT=\"$PROJECT\""
    echo "PROJECT_NAME=\"$PROJECT\""
    echo "PROJECT_VERSION=0.0"
) >project.sh





