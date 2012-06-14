function returnDocument()
{
    var file_name = document.location.href;
    var end = (file_name.indexOf("?") == -1) ? file_name.length : file_name.indexOf("?");
    return file_name.substring(file_name.lastIndexOf("/")+1, end);
}

document.write('<a href="http://sourceforge.net/projects/ariamaestosa" style="position: absolute; left: 5px; top: 5px;">')
document.write('<img src="http://sflogo.sourceforge.net/sflogo.php?group_id=186987&type=12" width="120" height="30" border="0" alt="Get Aria Maestosa at SourceForge.net. Fast, secure and Free Open Source software downloads" /></a>');
document.write('<br/><center><img src="aria120.png" style="margin-bottom: 25px; margin-right: 25px;"/><img src="logo.png" style="margin-bottom: 25px;"/></center>');

var selected_tab = returnDocument();
if (selected_tab.indexOf("#") != -1)
{
    selected_tab = selected_tab.substring(0, selected_tab.indexOf("#"));
}

var tabs = new Array();
tabs[0] = ["About", "index.html"];
tabs[1] = ["Downloads", "download.html"];
tabs[2] = ["Manual", "man.html"];
tabs[3] = ["Support", "contact.html"];
tabs[4] = ["Blog", "http://ariamaestosa.blogspot.com/"];
tabs[5] = ["Building from source", "building.html"];

document.write('<div style="text-align: center; min-height: 100%"><div style="text-align: center;">');
for (var i=0; i<tabs.length; i++)
{
    document.write( '&nbsp;' );
    if (tabs[i][1] == selected_tab || (selected_tab == "" && i == 0))
        document.write( '<div class="selected_tab"><b>' + tabs[i][0] + '</b></div>');
    else
        document.write( '<div class="tab"><a href="' + tabs[i][1] + '" style="color: black; text-decoration: none;">' + tabs[i][0] + '</a></div>');
    document.write( '&nbsp;' );
}
document.write('</div>');
    

document.write('<div class="page" style="min-height: 500px;">');
