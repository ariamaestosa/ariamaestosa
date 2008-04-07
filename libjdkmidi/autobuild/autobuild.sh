#! /bin/bash

declare -a configure_flags

relative_dir="$(dirname "$0")"

project_top_dir=$(cd "${relative_dir}/.." && pwd)

MAKE="${MAKE:-make}"

# load project specific bash definitions

. "${project_top_dir}/project.sh"

add_configure_flags() {
    for i in "$@"; do
        configure_flags[${#configure_flags[@]}]=$i
    done
}

# handle help

show_targets() {
    for i in $(find "${relative_dir}/targets/" -maxdepth 1 -type f -print); do
        echo "    " $(basename "$i")
    done
}

show_platforms() {
    for i in $(find "${relative_dir}/platforms/" -maxdepth 1 -type f -print); do
        echo "    " $(basename "$i")
    done
}

show_options() {
    for i in $(find "${relative_dir}/options/" -maxdepth 1 -type f -print); do
        echo "    " $(basename "$i")
    done
}

show_packages() {
    for i in $(find "${relative_dir}/packages/" -maxdepth 1 -type f -print); do
        echo "    " $(basename "$i")
    done
}

usage() {
    echo "Autobuild script for the J.D. Koftinoff Software Ltd.'s MagicMake system."
    echo "See http://opensource.jdkoftinoff.com/jdks/trac/jdks/wiki/MagicMakefileV5 for more information"
    echo ""
    echo "Usage:"
    echo ""
    echo "  mkdir work_dir"
    echo "  cd work_dir"
    echo "  project_dir/autobuild/autobuild.sh [target] [platform] [option] [package] [configure flags...]"
    echo ""
    echo "Where:"
    echo ""
    echo " [target] is one of: "
    show_targets
    echo ""
    echo " [platform] is one of: "
    show_platforms
    echo ""
    echo " [option] is one of: "
    show_options
    echo ""
    echo " [package] is one of: "
    show_packages
    echo ""
    echo " useful configure flags: "
    echo "    --target-install-dir=/opt  [defaults to /opt/local/$PROJECT-$PROJECT_VERSION]"
    echo "    --target-bin-dir=bin"
    echo "    --target-etc-dir=etc"
    echo "    --target-share-dir=share/$PROJECT-$PROJECT_VERSION"
    echo "    --target-docs-dir=share/doc/$PROJECT-$PROJECT_VERSION"
    echo "    --target-lib-dir=lib"
    echo "    --package-suffix=-abcd"
    echo "    --ship-to=USERNAME@SERVER:PATH"
    echo ""
}

error() {
    echo "$@"
    echo ""
    echo "try:"
    echo "   \"$0\" --help"
    echo ""
    echo "for information"
    exit 1
}

if [ x"$1" = x"--help" -o x"$1" = x"" ]; then
    usage
    exit 1
fi

target="$1"
shift
platform="$1"
shift
option="$1"
shift
package="$1"
shift

add_configure_flags "$@"

target_path="${relative_dir}/targets/${target}"
platform_path="${relative_dir}/platforms/${platform}"
option_path="${relative_dir}/options/${option}"
package_path="${relative_dir}/packages/${package}"

echo \"${relative_dir}/autobuild.sh\" \"$target\" \"$platform\" \"$option\" \"$package\" "\\" >./.autobuild.sh
for i in "$@"; do
    echo  " \"$i\" \\" >> ./.autobuild.sh
done
echo " \"\$@\" " >> ./.autobuild.sh
chmod +x ./.autobuild.sh
rm -f ./autobuild.sh
mv ./.autobuild.sh ./autobuild.sh

if [ -f "${platform_path}" ];
then
    . "${platform_path}"
else
    echo "unknown platform: \"${platform}\""
    echo
    echo "available platforms are:"
    show_platforms
    error
fi

if [ -f "${option_path}" ];
then
    . "${option_path}"
else
    echo "unknown option: \"${option}\""
    echo
    echo "available options are:"
    show_options
    error
fi

if [ -f "${package_path}" ];
then
    . "${package_path}"
else
    echo "unknown package: \"${package}\""
    echo
    echo "available packages are:"
    show_packages
    error
fi

. "${relative_dir}/targets/configure"    

if [ "${target}" != "configure" ]; 
then
    if "${MAKE}"; then
        if [ -f "${target_path}" ];
            then
            . "${target_path}"
        else
            echo "unknown target: \"${target}\""
            echo
            echo "available targets are:"
            show_targets
            error
        fi        
    fi
fi


