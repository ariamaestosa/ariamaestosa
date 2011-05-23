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

function writeSectionTop(divid, title, icon)
{
    document.write('<div class="section">');
    document.write('<table>');
    document.write('    <td style="vertical-align: middle;">&nbsp;<img src="' + icon + '"/>&nbsp;&nbsp;');
    document.write('    </td>');
    document.write('    <td style="vertical-align: middle;">');
    document.write('        <a href="javascript:;" onmousedown="toggleDiv(\'' + divid + '\');" border="0" style="color: black; text-decoration: none; font-weight: bold;">');
    document.write('          ' + title);
    document.write('        </a>');
    document.write('    </td>');
    document.write('</tr>');
    document.write('</table>');
    
    document.write('<div class="section_inside ' + divid + '" border="0">');
}

function writeSectionBottom()
{
    document.write('</div>');
    document.write('</div>');
}
