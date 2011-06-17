function returnDocument()
{
    var file_name = document.location.href;
    var end = (file_name.indexOf("?") == -1) ? file_name.length : file_name.indexOf("?");
    return file_name.substring(file_name.lastIndexOf("/")+1, end);
}

var selected_tab = returnDocument();

var tabs = new Array();
tabs[0] = ["About", "about.html"];
tabs[1] = ["Downloads", "download.html"];
tabs[2] = ["Manual", "man.html"];
tabs[3] = ["Support", "contact.html"];
tabs[4] = ["Blog", "blog.html"];
tabs[5] = ["Building from source", "building.html"];

document.write('<div style="min-height: 100%;">');
document.write('<table style="margin: 0px; padding: 0px; width:100%; border-spacing: 0pt; height: 100%;">');
document.write('    <tr style="margin: 0px; padding: 0px;">');
document.write('        <td style="width: 150px; background-image:url(\'backgroundleft.png\'); background-repeat:repeat-y; margin: 0px; padding: 0px;">');
document.write('        </td>');
document.write('        <td style="background-color:#628DC2;">');
document.write('            <br/>');
document.write('            <center><img src="aria120.png" style="margin-bottom: 25px; margin-right: 25px;"/><img src="logo.png" style="margin-bottom: 25px;"/></center>');
document.write('        </td>');
document.write('        <td style="width: 150px; background-image:url(\'background.png\'); background-repeat:repeat-y; margin: 0px; padding: 0px;">');
document.write('        </td>');
document.write('    </tr>');

document.write('    <tr style="margin: 0px; padding: 0px;">');
document.write('        <td style="width: 150px; background-image:url(\'backgroundleft.png\'); background-repeat:repeat-y; margin: 0px; padding: 0px;">');
document.write('        </td>');
document.write('        <td style="background-color:#628DC2; padding: 0px; margin: 0px; text-align:center;  vertical-align: text-bottom;">');

//document.write('            <span style="width:100%; text-align:center; align:center; padding: 0px;"><!--');
document.write('            <!--');
document.write('            --><table style="margin: 0px; padding: 0px; border-spacing: 0pt; display: inline-block; vertical-align: text-bottom;">');
document.write('                <tr style="margin: 0px; padding: 0px;">');
document.write('                    <td class="between_tabs">&nbsp;&nbsp;</td>');

                                        for (var i=0; i<tabs.length; i++)
                                        {
                                            if (tabs[i][1] == selected_tab)
                                                document.write( '<td class="selected_tab"><b>' + tabs[i][0] + '</b></td>');
                                            else
                                                document.write( '<td class="tab"><a href="' + tabs[i][1] + '" style="color: black; text-decoration: none;">' + tabs[i][0] + '</a><td>');
                                            document.write( '<td class="between_tabs">&nbsp;&nbsp;</td>');
                                        }

document.write('                </tr>');
//document.write('            </table></span>');
document.write('            </table>');
        
document.write('        </td>');
document.write('        <td style="width: 150px; background-image:url(\'background.png\'); background-repeat:repeat-y; margin: 0px; padding: 0px;">');
document.write('        </td>');
document.write('    </tr>');
    
document.write('    <tr style="margin: 0px; padding: 0px; padding: 0px; margin: 0px;">');
document.write('        <td style="width: 150px; background-image:url(\'backgroundleft.png\'); background-repeat:repeat-y; margin: 0px; padding: 0px;">');
document.write('        </td>');
document.write('        <td class="page" height="100%" style="vertical-align: top;">');

/*

              Some text<br/>

              <script src="included.js" type="text/javascript"> </script> 
              
              Some more text after
        </td>
        <td style="width: 250px; background-image:url('background.png'); background-repeat:repeat-y; margin: 0px; padding: 0px;">
        </td>
    </tr>
    
    <tr style="margin: 0px; padding: 0px;">
        <td style="width: 250px; background-image:url('backgroundleft.png'); background-repeat:repeat-y; margin: 0px; padding: 0px;">
        </td>
        <td style="background-color:#628DC2;">
            <br/>
            <center>
                <span class="bottom">Bottom stuff, etc.</span>
                <br/>
                <br/>
            </center>
        </td>
        <td style="width: 250px; background-image:url('background.png'); background-repeat:repeat-y; margin: 0px; padding: 0px;">
        </td>
    </tr>
</table>
*/
