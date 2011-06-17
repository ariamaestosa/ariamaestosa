function toggleDiv(classid)
{
    var elements = document.getElementsByClassName(classid)
    for (var i=0;i<elements.length;i++)
    {
        if (elements[i].style.display == 'none')
        {
            elements[i].style.display = 'block';
        }
        else
        {
            elements[i].style.display = 'none';
        }
    }
}

function writeSectionTop(divid, title, icons)
{
    var OSName="Unknown OS";
    if (navigator.appVersion.indexOf("Win")!=-1) OSName="windows";
    if (navigator.appVersion.indexOf("Mac")!=-1) OSName="osx";
    if (navigator.appVersion.indexOf("X11")!=-1) OSName="linux";
    if (navigator.appVersion.indexOf("Linux")!=-1) OSName="linux";

    var display = "block";
    
    if (divid.search(OSName) == -1) display = "none";

    document.write('<div class="section">');
    document.write('<table style="margin: 0px; padding: 0px;">');
    for (var i=0; i<icons.length; i++)
    {
    document.write('    <td style="vertical-align: middle;">&nbsp;<img src="' + icons[i] + '"/>&nbsp;&nbsp;');
    }
    document.write('    </td>');
    document.write('    <td style="vertical-align: middle;">');
    document.write('        <a href="javascript:;" onmousedown="toggleDiv(\'' + divid + '\');" style="border: 0px; color: black; text-decoration: none; font-weight: bold;">');
    document.write('          ' + title);
    document.write('        </a>');
    document.write('    </td>');
    document.write('</tr>');
    document.write('</table>');
    
    document.write('<div class="section_inside ' + divid + '" style="border: 0px; display: ' + display + '">');
}

function writeSectionBottom()
{
    document.write('</div>');
    document.write('</div>');
}
