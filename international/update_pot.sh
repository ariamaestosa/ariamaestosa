CPP_FILE_LIST=`find . -name '*.cpp' -print`
echo $CPP_FILE_LIST
xgettext -d AriaS -s --keyword=_ --add-comments="I18N:" -p ./international -o aria_maestosa.pot $CPP_FILE_LIST
