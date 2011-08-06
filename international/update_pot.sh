CPP_FILE_LIST=`find . -name '*.cpp' -print`
echo $CPP_FILE_LIST

H_FILE_LIST=`find . -name '*.h' -print`
echo $H_FILE_LIST

xgettext -d AriaS --keyword=_ --add-comments="I18N:" -p ./international -o aria_maestosa.pot $CPP_FILE_LIST $H_FILE_LIST
