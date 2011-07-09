/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "Printing/RenderRoutines.h"

#include "IO/IOUtils.h"
#include "Range.h"
#include <wx/dc.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/graphics.h>

using namespace AriaMaestosa;
using namespace AriaMaestosa::RenderRoutines;

#if wxCHECK_VERSION(2,9,1) && wxUSE_GRAPHICS_CONTEXT
#include <wx/graphics.h>

/**
 * Based on TuxGuitar render routines, released under GNU GPL
 * (c) Julian Gabriel Casadesus and others
 */
void AriaMaestosa::RenderRoutines::paintTreble(wxGraphicsContext& painter, int x, int B_y, int E_y)
{
    wxGraphicsPath path = painter.CreatePath();
    
    // B : (x + (1.2321913f * scale)),(y + (2.1369102f * scale))
    // E : (x + (1.578771f * scale)),(y + (3.8828878f * scale))
    
    float scale = (E_y - B_y)/1.95; // 1.95 = approximate distance from B to E
    float y = B_y - (2.14f * scale); // 2.14 = approximate coordinate of B
    
    path.MoveToPoint((x + (0.9706216f * scale)),(y + (-0.9855771f * scale)));
    path.AddCurveToPoint((x + (0.99023926f * scale)),(y + (-0.99538594f * scale)),
                         (x + (0.99350905f * scale)),(y + (-0.99538594f * scale)),
                         (x + (1.0131269f * scale)),(y + (-0.99538594f * scale)));
    path.AddCurveToPoint((x + (1.0392835f * scale)),(y + (-0.99211615f * scale)),
                         (x + (1.055632f * scale)),(y + (-0.9823073f * scale)),
                         (x + (1.0915977f * scale)),(y + (-0.9430719f * scale)));
    path.AddCurveToPoint((x + (1.3270102f * scale)),(y + (-0.7011198f * scale)),
                         (x + (1.5231876f * scale)),(y + (-0.26953024f * scale)),
                         (x + (1.572232f * scale)),(y + (0.09666765f * scale)));
    path.AddCurveToPoint((x + (1.578771f * scale)),(y + (0.1555208f * scale)),
                         (x + (1.578771f * scale)),(y + (0.29284477f * scale)),
                         (x + (1.572232f * scale)),(y + (0.35496712f * scale)));
    path.AddCurveToPoint((x + (1.5362663f * scale)),(y + (0.6917379f * scale)),
                         (x + (1.3956721f * scale)),(y + (0.9990827f * scale)),
                         (x + (1.0719799f * scale)),(y + (1.4404812f * scale)));
    path.AddLineToPoint((x + (1.0262054f * scale)),(y + (1.502604f * scale)));
    path.AddLineToPoint((x + (1.0523622f * scale)),(y + (1.577805f * scale)));
    path.AddCurveToPoint((x + (1.1144851f * scale)),(y + (1.7576342f * scale)),
                         (x + (1.1864164f * scale)),(y + (1.9766989f * scale)),
                         (x + (1.2321913f * scale)),(y + (2.1369102f * scale)));
    
    //painter.SetBrush( *wxYELLOW_BRUSH );
    //painter.DrawRectangle((x + (1.2321913f * scale)),(y + (2.1369102f * scale)), 3, 3);
    
    path.AddCurveToPoint((x + (1.24527f * scale)),(y + (2.1761456f * scale)),
                         (x + (1.2550789f * scale)),(y + (2.208842f * scale)),
                         (x + (1.2550789f * scale)),(y + (2.212112f * scale)));
    path.AddCurveToPoint((x + (1.2550789f * scale)),(y + (2.212112f * scale)),
                         (x + (1.2779659f * scale)),(y + (2.212112f * scale)),
                         (x + (1.3008534f * scale)),(y + (2.2153816f * scale)));
    path.AddCurveToPoint((x + (1.4152898f * scale)),(y + (2.22519f * scale)),
                         (x + (1.5133789f * scale)),(y + (2.2513473f * scale)),
                         (x + (1.6147372f * scale)),(y + (2.3003914f * scale)));
    path.AddCurveToPoint((x + (1.6964773f * scale)),(y + (2.3428962f * scale)),
                         (x + (1.7684091f * scale)),(y + (2.3919404f * scale)),
                         (x + (1.8370711f * scale)),(y + (2.457333f * scale)));
    path.AddCurveToPoint((x + (1.9122725f * scale)),(y + (2.5325344f * scale)),
                         (x + (1.9613168f * scale)),(y + (2.601196f * scale)),
                         (x + (2.0070913f * scale)),(y + (2.6960156f * scale)));
    path.AddCurveToPoint((x + (2.0757532f * scale)),(y + (2.8333395f * scale)),
                         (x + (2.10518f * scale)),(y + (2.9772024f * scale)),
                         (x + (2.10191f * scale)),(y + (3.121066f * scale)));
    path.AddCurveToPoint((x + (2.0986407f * scale)),(y + (3.2126155f * scale)),
                         (x + (2.085562f * scale)),(y + (3.2812777f * scale)),
                         (x + (2.0561357f * scale)),(y + (3.3662882f * scale)));
    path.AddCurveToPoint((x + (2.0169f * scale)),(y + (3.4905329f * scale)),
                         (x + (1.9449685f * scale)),(y + (3.60497f * scale)),
                         (x + (1.8468798f * scale)),(y + (3.7030587f * scale)));    
    path.AddCurveToPoint((x + (1.7618695f * scale)),(y + (3.7880688f * scale)),
                         (x + (1.6833986f * scale)),(y + (3.8403826f * scale)),
                         (x + (1.578771f * scale)),(y + (3.8828878f * scale)));
    
    //painter.SetBrush( *wxBLUE_BRUSH );
    //painter.DrawRectangle((x + (1.578771f * scale)),(y + (3.8828878f * scale)), 10, 10);
    
    path.AddLineToPoint((x + (1.5395356f * scale)),(y + (3.8992357f * scale)));
    path.AddLineToPoint((x + (1.5395356f * scale)),(y + (4.016942f * scale)));
    path.AddCurveToPoint((x + (1.5395356f * scale)),(y + (4.183693f * scale)),
                         (x + (1.5297267f * scale)),(y + (4.37987f * scale)),
                         (x + (1.516648f * scale)),(y + (4.497576f * scale)));
    path.AddCurveToPoint((x + (1.50357f * scale)),(y + (4.6185517f * scale)),
                         (x + (1.4708736f * scale)),(y + (4.732989f * scale)),
                         (x + (1.4185596f * scale)),(y + (4.837617f * scale)));
    path.AddCurveToPoint((x + (1.2812357f * scale)),(y + (5.1155343f * scale)),
                         (x + (1.0392835f * scale)),(y + (5.262667f * scale)),
                         (x + (0.7679054f * scale)),(y + (5.2365108f * scale)));
    path.AddCurveToPoint((x + (0.46056065f * scale)),(y + (5.2070837f * scale)),
                         (x + (0.21207006f * scale)),(y + (4.997828f * scale)),
                         (x + (0.14994715f * scale)),(y + (4.716641f * scale)));
    path.AddCurveToPoint((x + (0.12052006f * scale)),(y + (4.589125f * scale)),
                         (x + (0.13359922f * scale)),(y + (4.491037f * scale)),
                         (x + (0.18591277f * scale)),(y + (4.409296f * scale)));
    path.AddCurveToPoint((x + (0.25457475f * scale)),(y + (4.3013988f * scale)),
                         (x + (0.38535964f * scale)),(y + (4.2392764f * scale)),
                         (x + (0.5128747f * scale)),(y + (4.2425456f * scale)));
    path.AddCurveToPoint((x + (0.6632773f * scale)),(y + (4.2490854f * scale)),
                         (x + (0.78425336f * scale)),(y + (4.3667912f * scale)),
                         (x + (0.80714035f * scale)),(y + (4.5302725f * scale)));
    path.AddCurveToPoint((x + (0.8234888f * scale)),(y + (4.661057f * scale)),
                         (x + (0.7679054f * scale)),(y + (4.782033f * scale)),
                         (x + (0.6632773f * scale)),(y + (4.8474255f * scale)));
    path.AddCurveToPoint((x + (0.6044242f * scale)),(y + (4.8833914f * scale)),
                         (x + (0.52268356f * scale)),(y + (4.8997393f * scale)),
                         (x + (0.46056065f * scale)),(y + (4.8899307f * scale)));
    path.AddCurveToPoint((x + (0.45075235f * scale)),(y + (4.8899307f * scale)),
                         (x + (0.44094297f * scale)),(y + (4.8866606f * scale)),
                         (x + (0.44094297f * scale)),(y + (4.8899307f * scale)));
    path.AddLineToPoint((x + (0.48017892f * scale)),(y + (4.929166f * scale)));
    path.AddCurveToPoint((x + (0.55211014f * scale)),(y + (5.0010977f * scale)),
                         (x + (0.63385075f * scale)),(y + (5.0468726f * scale)),
                         (x + (0.7384789f * scale)),(y + (5.06976f * scale)));
    path.AddCurveToPoint((x + (0.76136583f * scale)),(y + (5.0730295f * scale)),
                         (x + (0.78098357f * scale)),(y + (5.0730295f * scale)),
                         (x + (0.8332976f * scale)),(y + (5.0730295f * scale)));
    path.AddCurveToPoint((x + (0.89215076f * scale)),(y + (5.0730295f * scale)),
                         (x + (0.8986898f * scale)),(y + (5.0730295f * scale)),
                         (x + (0.9313861f * scale)),(y + (5.0664897f * scale)));
    path.AddCurveToPoint((x + (0.97716117f * scale)),(y + (5.0534115f * scale)),
                         (x + (1.0163965f * scale)),(y + (5.0370636f * scale)),
                         (x + (1.0523622f * scale)),(y + (5.0174456f * scale)));
    path.AddCurveToPoint((x + (1.2158434f * scale)),(y + (4.919357f * scale)),
                         (x + (1.3270102f * scale)),(y + (4.716641f * scale)),
                         (x + (1.3564366f * scale)),(y + (4.47142f * scale)));
    path.AddCurveToPoint((x + (1.3662455f * scale)),(y + (4.37987f * scale)),
                         (x + (1.3760543f * scale)),(y + (4.183693f * scale)),
                         (x + (1.3760543f * scale)),(y + (4.0398297f * scale)));
    path.AddCurveToPoint((x + (1.3760543f * scale)),(y + (3.9450107f * scale)),
                         (x + (1.3760543f * scale)),(y + (3.9384713f * scale)),
                         (x + (1.3695153f * scale)),(y + (3.9384713f * scale)));
    path.AddCurveToPoint((x + (1.3564366f * scale)),(y + (3.9450107f * scale)),
                         (x + (1.2877747f * scale)),(y + (3.95155f * scale)),
                         (x + (1.2387304f * scale)),(y + (3.9548192f * scale)));
    path.AddCurveToPoint((x + (1.1766075f * scale)),(y + (3.9580889f * scale)),
                         (x + (1.0719799f * scale)),(y + (3.9580889f * scale)),
                         (x + (1.0229356f * scale)),(y + (3.95155f * scale)));
    path.AddCurveToPoint((x + (0.8300278f * scale)),(y + (3.9286623f * scale)),
                         (x + (0.65346843f * scale)),(y + (3.8632698f * scale)),
                         (x + (0.4997966f * scale)),(y + (3.755372f * scale)));
    path.AddCurveToPoint((x + (0.2709232f * scale)),(y + (3.595161f * scale)),
                         (x + (0.10744194f * scale)),(y + (3.3564782f * scale)),
                         (x + (0.038779963f * scale)),(y + (3.0818305f * scale)));
    path.AddCurveToPoint((x + (-0.04949972f * scale)),(y + (2.7385209f * scale)),
                         (x + (0.012623194f * scale)),(y + (2.3723233f * scale)),
                         (x + (0.22841798f * scale)),(y + (1.9865077f * scale)));
    path.AddCurveToPoint((x + (0.31996745f * scale)),(y + (1.8262968f * scale)),
                         (x + (0.40824714f * scale)),(y + (1.6955118f * scale)),
                         (x + (0.6273117f * scale)),(y + (1.4045155f * scale)));
    path.AddLineToPoint((x + (0.751557f * scale)),(y + (1.2377651f * scale)));
    path.AddLineToPoint((x + (0.72866946f * scale)),(y + (1.1756423f * scale)));
    path.AddCurveToPoint((x + (0.5717278f * scale)),(y + (0.7604004f * scale)),
                         (x + (0.51941377f * scale)),(y + (0.52171814f * scale)),
                         (x + (0.5030659f * scale)),(y + (0.17513847f * scale)));
    path.AddCurveToPoint((x + (0.49652684f * scale)),(y + (0.024735928f * scale)),
                         (x + (0.5030659f * scale)),(y + (-0.102779746f * scale)),
                         (x + (0.52268356f * scale)),(y + (-0.2074073f * scale)));
    path.AddCurveToPoint((x + (0.5717278f * scale)),(y + (-0.48205525f * scale)),
                         (x + (0.69924295f * scale)),(y + (-0.72727656f * scale)),
                         (x + (0.8986898f * scale)),(y + (-0.92345417f * scale)));
    path.AddCurveToPoint((x + (0.9379252f * scale)),(y + (-0.9626896f * scale)),
                         (x + (0.9542737f * scale)),(y + (-0.9757682f * scale)),
                         (x + (0.9706216f * scale)),(y + (-0.9855771f * scale)));
    
    path.MoveToPoint((x + (1.2289215f * scale)),(y + (-0.4264719f * scale)));
    path.AddCurveToPoint((x + (1.1962258f * scale)),(y + (-0.48205525f * scale)),
                         (x + (1.2027647f * scale)),(y + (-0.478786f * scale)),
                         (x + (1.1733383f * scale)),(y + (-0.45916772f * scale)));
    path.AddCurveToPoint((x + (1.0817888f * scale)),(y + (-0.40358436f * scale)),
                         (x + (0.98697f * scale)),(y + (-0.3185745f * scale)),
                         (x + (0.9183075f * scale)),(y + (-0.23356462f * scale)));
    path.AddCurveToPoint((x + (0.77771425f * scale)),(y + (-0.05373496f * scale)),
                         (x + (0.69924295f * scale)),(y + (0.1555208f * scale)),
                         (x + (0.68943465f * scale)),(y + (0.38112438f * scale)));
    path.AddCurveToPoint((x + (0.6861648f * scale)),(y + (0.4726739f * scale)),
                         (x + (0.69597363f * scale)),(y + (0.5315269f * scale)),
                         (x + (0.7384789f * scale)),(y + (0.6721202f * scale)));
    path.AddCurveToPoint((x + (0.77117467f * scale)),(y + (0.7865572f * scale)),
                         (x + (0.8627241f * scale)),(y + (1.0644748f * scale)),
                         (x + (0.86926377f * scale)),(y + (1.0710139f * scale)));
    path.AddCurveToPoint((x + (0.86926377f * scale)),(y + (1.0742836f * scale)),
                         (x + (0.88888097f * scale)),(y + (1.0513968f * scale)),
                         (x + (0.90849864f * scale)),(y + (1.0219696f * scale)));
    path.AddCurveToPoint((x + (1.1341028f * scale)),(y + (0.6982775f * scale)),
                         (x + (1.2550789f * scale)),(y + (0.43343806f * scale)),
                         (x + (1.3008534f * scale)),(y + (0.16859889f * scale)));
    path.AddCurveToPoint((x + (1.3106622f * scale)),(y + (0.09993696f * scale)),
                         (x + (1.3172013f * scale)),(y + (0.044353604f * scale)),
                         (x + (1.320471f * scale)),(y + (-0.03411722f * scale)));
    path.AddCurveToPoint((x + (1.3237408f * scale)),(y + (-0.2139464f * scale)),
                         (x + (1.3041232f * scale)),(y + (-0.30222607f * scale)),
                         (x + (1.2289215f * scale)),(y + (-0.4264719f * scale)));
    
    path.MoveToPoint((x + (0.9477346f * scale)),(y + (1.7739828f * scale)));
    path.AddCurveToPoint((x + (0.9281169f * scale)),(y + (1.71186f * scale)),
                         (x + (0.90849864f * scale)),(y + (1.6628156f * scale)),
                         (x + (0.90849864f * scale)),(y + (1.6628156f * scale)));
    path.AddCurveToPoint((x + (0.9052294f * scale)),(y + (1.6628156f * scale)),
                         (x + (0.7679054f * scale)),(y + (1.8459139f * scale)),
                         (x + (0.7090518f * scale)),(y + (1.9276547f * scale)));
    path.AddCurveToPoint((x + (0.5161445f * scale)),(y + (2.1990333f * scale)),
                         (x + (0.39843827f * scale)),(y + (2.4180977f * scale)),
                         (x + (0.32977578f * scale)),(y + (2.6338923f * scale)));
    path.AddCurveToPoint((x + (0.2840013f * scale)),(y + (2.7679467f * scale)),
                         (x + (0.26438358f * scale)),(y + (2.8921926f * scale)),
                         (x + (0.25784454f * scale)),(y + (3.0295167f * scale)));
    path.AddCurveToPoint((x + (0.25784454f * scale)),(y + (3.1047182f * scale)),
                         (x + (0.26111433f * scale)),(y + (3.1504927f * scale)),
                         (x + (0.27419245f * scale)),(y + (3.2060761f * scale)));
    path.AddCurveToPoint((x + (0.34939402f * scale)),(y + (3.4970722f * scale)),
                         (x + (0.6600081f * scale)),(y + (3.7357545f * scale)),
                         (x + (1.0262054f * scale)),(y + (3.7880688f * scale)));
    path.AddCurveToPoint((x + (1.0817888f * scale)),(y + (3.7946076f * scale)),
                         (x + (1.2387304f * scale)),(y + (3.7946076f * scale)),
                         (x + (1.3073924f * scale)),(y + (3.7880688f * scale)));
    path.AddCurveToPoint((x + (1.3760543f * scale)),(y + (3.7782598f * scale)),
                         (x + (1.3695153f * scale)),(y + (3.7815294f * scale)),
                         (x + (1.3662455f * scale)),(y + (3.7521029f * scale)));
    path.AddCurveToPoint((x + (1.3466283f * scale)),(y + (3.4414887f * scale)),
                         (x + (1.320471f * scale)),(y + (3.2518506f * scale)),
                         (x + (1.2681575f * scale)),(y + (2.9706633f * scale)));
    path.AddCurveToPoint((x + (1.2485392f * scale)),(y + (2.8529572f * scale)),
                         (x + (1.192956f * scale)),(y + (2.604466f * scale)),
                         (x + (1.1896861f * scale)),(y + (2.601196f * scale)));
    path.AddCurveToPoint((x + (1.1864164f * scale)),(y + (2.5979269f * scale)),
                         (x + (1.1242939f * scale)),(y + (2.604466f * scale)),
                         (x + (1.0883284f * scale)),(y + (2.614275f * scale)));
    path.AddCurveToPoint((x + (1.0425533f * scale)),(y + (2.6240838f * scale)),
                         (x + (1.0131269f * scale)),(y + (2.6338923f * scale)),
                         (x + (0.9738914f * scale)),(y + (2.6535103f * scale)));
    path.AddCurveToPoint((x + (0.76136583f * scale)),(y + (2.7614079f * scale)),
                         (x + (0.69924295f * scale)),(y + (3.0066295f * scale)),
                         (x + (0.8332976f * scale)),(y + (3.2060761f * scale)));
    path.AddCurveToPoint((x + (0.85618514f * scale)),(y + (3.2387724f * scale)),
                         (x + (0.9150382f * scale)),(y + (3.2976255f * scale)),
                         (x + (0.9510039f * scale)),(y + (3.3205128f * scale)));
    path.AddCurveToPoint((x + (0.96408254f * scale)),(y + (3.3303218f * scale)),
                         (x + (0.99350905f * scale)),(y + (3.3499393f * scale)),
                         (x + (1.0131269f * scale)),(y + (3.3597484f * scale)));
    path.AddCurveToPoint((x + (1.0425533f * scale)),(y + (3.3760962f * scale)),
                         (x + (1.0490924f * scale)),(y + (3.3826356f * scale)),
                         (x + (1.055632f * scale)),(y + (3.3957143f * scale)));
    path.AddCurveToPoint((x + (1.0817888f * scale)),(y + (3.4382195f * scale)),
                         (x + (1.0654408f * scale)),(y + (3.4905329f * scale)),
                         (x + (1.0163965f * scale)),(y + (3.5134206f * scale)));
    path.AddCurveToPoint((x + (0.99023926f * scale)),(y + (3.5264988f * scale)),
                         (x + (0.9706216f * scale)),(y + (3.5232296f * scale)),
                         (x + (0.9183075f * scale)),(y + (3.4970722f * scale)));
    path.AddCurveToPoint((x + (0.79079294f * scale)),(y + (3.4349499f * scale)),
                         (x + (0.69597363f * scale)),(y + (3.3499393f * scale)),
                         (x + (0.6273117f * scale)),(y + (3.2453117f * scale)));
    path.AddCurveToPoint((x + (0.4997966f * scale)),(y + (3.0491343f * scale)),
                         (x + (0.48344818f * scale)),(y + (2.7973735f * scale)),
                         (x + (0.5880763f * scale)),(y + (2.5848482f * scale)));
    path.AddCurveToPoint((x + (0.67962575f * scale)),(y + (2.4017498f * scale)),
                         (x + (0.84310645f * scale)),(y + (2.2775042f * scale)),
                         (x + (1.0523622f * scale)),(y + (2.2317295f * scale)));
    path.AddCurveToPoint((x + (1.0719799f * scale)),(y + (2.22519f * scale)),
                         (x + (1.0883284f * scale)),(y + (2.2219207f * scale)),
                         (x + (1.0883284f * scale)),(y + (2.2219207f * scale)));
    path.AddCurveToPoint((x + (1.0915977f * scale)),(y + (2.2186508f * scale)),
                         (x + (1.0000482f * scale)),(y + (1.9211154f * scale)),
                         (x + (0.9477346f * scale)),(y + (1.7739828f * scale)));
    
    path.MoveToPoint((x + (1.3924028f * scale)),(y + (2.617545f * scale)));
    path.AddCurveToPoint((x + (1.382594f * scale)),(y + (2.617545f * scale)),
                         (x + (1.3727851f * scale)),(y + (2.614275f * scale)),
                         (x + (1.3695153f * scale)),(y + (2.614275f * scale)));
    path.AddLineToPoint((x + (1.3597065f * scale)),(y + (2.611005f * scale)));
    path.AddLineToPoint((x + (1.3760543f * scale)),(y + (2.6796675f * scale)));
    path.AddCurveToPoint((x + (1.4512559f * scale)),(y + (3.0098987f * scale)),
                         (x + (1.5068393f * scale)),(y + (3.369557f * scale)),
                         (x + (1.526457f * scale)),(y + (3.6736321f * scale)));
    path.AddCurveToPoint((x + (1.5297267f * scale)),(y + (3.7063284f * scale)),
                         (x + (1.5297267f * scale)),(y + (3.7324853f * scale)),
                         (x + (1.5297267f * scale)),(y + (3.7324853f * scale)));
    path.AddCurveToPoint((x + (1.5329965f * scale)),(y + (3.7324853f * scale)),
                         (x + (1.5755012f * scale)),(y + (3.7128677f * scale)),
                         (x + (1.5983888f * scale)),(y + (3.6965194f * scale)));
    path.AddCurveToPoint((x + (1.6539721f * scale)),(y + (3.6638227f * scale)),
                         (x + (1.7062862f * scale)),(y + (3.6147785f * scale)),
                         (x + (1.7487912f * scale)),(y + (3.559195f * scale)));
    path.AddCurveToPoint((x + (1.8501498f * scale)),(y + (3.4251404f * scale)),
                         (x + (1.8861152f * scale)),(y + (3.25839f * scale)),
                         (x + (1.8468798f * scale)),(y + (3.0883694f * scale)));
    path.AddCurveToPoint((x + (1.8174533f * scale)),(y + (2.9510455f * scale)),
                         (x + (1.7389826f * scale)),(y + (2.8235304f * scale)),
                         (x + (1.6278152f * scale)),(y + (2.735251f * scale)));
    path.AddCurveToPoint((x + (1.5591533f * scale)),(y + (2.6829367f * scale)),
                         (x + (1.4676039f * scale)),(y + (2.6371622f * scale)),
                         (x + (1.3924028f * scale)),(y + (2.617545f * scale)));
    
    painter.SetPen( *wxBLACK_PEN );
    painter.SetBrush( *wxBLACK_BRUSH );
    painter.FillPath(path);
}

/**
 * Based on TuxGuitar render routines, released under GNU GPL
 * (c) Julian Gabriel Casadesus and others
 */
void AriaMaestosa::RenderRoutines::paintBass(wxGraphicsContext& painter, int x, int score_top, int E_y)
{
    //float scale = 75.0f;   

    float scale = (E_y - score_top)/1.25; // 1.25 = approximate distance from top to E
    float y = score_top - 0.17f*scale; // topmost y at 0.17

    wxGraphicsPath path = painter.CreatePath();
    
    path.MoveToPoint((x + (0.71937084f * scale)),(y + (0.16147426f * scale)));
    path.AddCurveToPoint((x + (0.75454587f * scale)),(y + (0.15827677f * scale)),
                         (x + (0.8920496f * scale)),(y + (0.16147426f * scale)),
                         (x + (0.94321334f * scale)),(y + (0.16467176f * scale)));
    path.AddCurveToPoint((x + (1.3429334f * scale)),(y + (0.20944051f * scale)),
                         (x + (1.6147422f * scale)),(y + (0.4077018f * scale)),
                         (x + (1.7042797f * scale)),(y + (0.72108173f * scale)));
    path.AddCurveToPoint((x + (1.7266634f * scale)),(y + (0.8074205f * scale)),
                         (x + (1.7362571f * scale)),(y + (0.87137556f * scale)),
                         (x + (1.7330583f * scale)),(y + (0.9800993f * scale)));
    path.AddCurveToPoint((x + (1.7330583f * scale)),(y + (1.1176031f * scale)),
                         (x + (1.7170696f * scale)),(y + (1.2327217f * scale)),
                         (x + (1.6723021f * scale)),(y + (1.3542367f * scale)));
    path.AddCurveToPoint((x + (1.5092158f * scale)),(y + (1.8370967f * scale)),
                         (x + (1.0327508f * scale)),(y + (2.236817f * scale)),
                         (x + (0.16296211f * scale)),(y + (2.6205468f * scale)));
    path.AddCurveToPoint((x + (0.12778586f * scale)),(y + (2.6365356f * scale)),
                         (x + (0.095808364f * scale)),(y + (2.6525242f * scale)),
                         (x + (0.092610866f * scale)),(y + (2.655722f * scale)));
    path.AddCurveToPoint((x + (0.073424615f * scale)),(y + (2.668513f * scale)),
                         (x + (0.054238364f * scale)),(y + (2.6717105f * scale)),
                         (x + (0.035052113f * scale)),(y + (2.6621182f * scale)));
    path.AddCurveToPoint((x + (0.019063365f * scale)),(y + (2.655722f * scale)),
                         (x + (0.012667115f * scale)),(y + (2.6493268f * scale)),
                         (x + (0.006272115f * scale)),(y + (2.633338f * scale)));
    path.AddCurveToPoint((x + (-0.0033216353f * scale)),(y + (2.6173494f * scale)),
                         (x + (-1.2413526E-4f * scale)),(y + (2.6013606f * scale)),
                         (x + (0.006272115f * scale)),(y + (2.5885694f * scale)));
    path.AddCurveToPoint((x + (0.012667115f * scale)),(y + (2.575778f * scale)),
                         (x + (0.025458366f * scale)),(y + (2.5661855f * scale)),
                         (x + (0.095808364f * scale)),(y + (2.5246143f * scale)));
    path.AddCurveToPoint((x + (0.3804096f * scale)),(y + (2.355133f * scale)),
                         (x + (0.58826333f * scale)),(y + (2.2048392f * scale)),
                         (x + (0.7641396f * scale)),(y + (2.0513468f * scale)));
    path.AddCurveToPoint((x + (0.8185009f * scale)),(y + (2.000183f * scale)),
                         (x + (0.9208296f * scale)),(y + (1.8978542f * scale)),
                         (x + (0.96559834f * scale)),(y + (1.8466904f * scale)));
    path.AddCurveToPoint((x + (1.1606609f * scale)),(y + (1.6196505f * scale)),
                         (x + (1.2629896f * scale)),(y + (1.3990055f * scale)),
                         (x + (1.2981646f * scale)),(y + (1.1431843f * scale)));
    path.AddCurveToPoint((x + (1.3045596f * scale)),(y + (1.0824268f * scale)),
                         (x + (1.3045596f * scale)),(y + (0.93852806f * scale)),
                         (x + (1.2981646f * scale)),(y + (0.87777054f * scale)));
    path.AddCurveToPoint((x + (1.2853733f * scale)),(y + (0.7850368f * scale)),
                         (x + (1.2629896f * scale)),(y + (0.69869673f * scale)),
                         (x + (1.2342097f * scale)),(y + (0.6219505f * scale)));
    path.AddCurveToPoint((x + (1.2118247f * scale)),(y + (0.5707868f * scale)),
                         (x + (1.2054296f * scale)),(y + (0.554798f * scale)),
                         (x + (1.1798471f * scale)),(y + (0.51322675f * scale)));
    path.AddCurveToPoint((x + (1.0871121f * scale)),(y + (0.35653678f * scale)),
                         (x + (0.94641083f * scale)),(y + (0.27019802f * scale)),
                         (x + (0.7897221f * scale)),(y + (0.27019802f * scale)));
    path.AddCurveToPoint((x + (0.63622963f * scale)),(y + (0.27019802f * scale)),
                         (x + (0.5083196f * scale)),(y + (0.35653678f * scale)),
                         (x + (0.42517713f * scale)),(y + (0.51962304f * scale)));
    path.AddCurveToPoint((x + (0.4123871f * scale)),(y + (0.5452043f * scale)),
                         (x + (0.3804096f * scale)),(y + (0.6283468f * scale)),
                         (x + (0.3804096f * scale)),(y + (0.63154423f * scale)));
    path.AddCurveToPoint((x + (0.3804096f * scale)),(y + (0.6347418f * scale)),
                         (x + (0.39319962f * scale)),(y + (0.63154423f * scale)),
                         (x + (0.40599087f * scale)),(y + (0.63154423f * scale)));
    path.AddCurveToPoint((x + (0.4795396f * scale)),(y + (0.6283468f * scale)),
                         (x + (0.55948335f * scale)),(y + (0.65712553f * scale)),
                         (x + (0.62024087f * scale)),(y + (0.705093f * scale)));
    path.AddCurveToPoint((x + (0.7353596f * scale)),(y + (0.80422306f * scale)),
                         (x + (0.7673371f * scale)),(y + (0.9641105f * scale)),
                         (x + (0.6969871f * scale)),(y + (1.1080092f * scale)));
    path.AddCurveToPoint((x + (0.64902085f * scale)),(y + (1.2135355f * scale)),
                         (x + (0.54669213f * scale)),(y + (1.280688f * scale)),
                         (x + (0.43477085f * scale)),(y + (1.2902818f * scale)));
    path.AddCurveToPoint((x + (0.27808085f * scale)),(y + (1.2998742f * scale)),
                         (x + (0.12458836f * scale)),(y + (1.1975467f * scale)),
                         (x + (0.073424615f * scale)),(y + (1.0504493f * scale)));
    path.AddCurveToPoint((x + (0.057435866f * scale)),(y + (1.002483f * scale)),
                         (x + (0.054238364f * scale)),(y + (0.95451677f * scale)),
                         (x + (0.060633365f * scale)),(y + (0.87777054f * scale)));
    path.AddCurveToPoint((x + (0.08941337f * scale)),(y + (0.55799556f * scale)),
                         (x + (0.29406962f * scale)),(y + (0.28618675f * scale)),
                         (x + (0.5754721f * scale)),(y + (0.19345176f * scale)));
    path.AddCurveToPoint((x + (0.62343836f * scale)),(y + (0.17746301f * scale)),
                         (x + (0.6714046f * scale)),(y + (0.16786925f * scale)),
                         (x + (0.71937084f * scale)),(y + (0.16147426f * scale)));
    path.MoveToPoint((x + (1.9632971f * scale)),(y + (0.462063f * scale)));
    path.AddCurveToPoint((x + (2.0144608f * scale)),(y + (0.44607428f * scale)),
                         (x + (2.0752184f * scale)),(y + (0.462063f * scale)),
                         (x + (2.1167896f * scale)),(y + (0.5004368f * scale)));
    path.AddCurveToPoint((x + (2.1455696f * scale)),(y + (0.53241426f * scale)),
                         (x + (2.1615584f * scale)),(y + (0.57398427f * scale)),
                         (x + (2.1615584f * scale)),(y + (0.6155555f * scale)));
    path.AddCurveToPoint((x + (2.1615584f * scale)),(y + (0.7018943f * scale)),
                         (x + (2.091207f * scale)),(y + (0.7722455f * scale)),
                         (x + (2.0048683f * scale)),(y + (0.7722455f * scale)));
    path.AddCurveToPoint((x + (1.9153309f * scale)),(y + (0.7722455f * scale)),
                         (x + (1.8449808f * scale)),(y + (0.7018943f * scale)),
                         (x + (1.8449808f * scale)),(y + (0.6155555f * scale)));
    path.AddCurveToPoint((x + (1.8449808f * scale)),(y + (0.54200673f * scale)),
                         (x + (1.8929471f * scale)),(y + (0.48124927f * scale)),
                         (x + (1.9632971f * scale)),(y + (0.462063f * scale)));
    path.MoveToPoint((x + (1.9632971f * scale)),(y + (1.2583042f * scale)));
    path.AddCurveToPoint((x + (2.0144608f * scale)),(y + (1.2423155f * scale)),
                         (x + (2.0752184f * scale)),(y + (1.2583042f * scale)),
                         (x + (2.1167896f * scale)),(y + (1.2966768f * scale)));
    path.AddCurveToPoint((x + (2.155162f * scale)),(y + (1.338248f * scale)),
                         (x + (2.171151f * scale)),(y + (1.3958068f * scale)),
                         (x + (2.155162f * scale)),(y + (1.4469718f * scale)));
    path.AddCurveToPoint((x + (2.1391733f * scale)),(y + (1.5205193f * scale)),
                         (x + (2.0784159f * scale)),(y + (1.5684855f * scale)),
                         (x + (2.0048683f * scale)),(y + (1.5684855f * scale)));
    path.AddCurveToPoint((x + (1.9153309f * scale)),(y + (1.5684855f * scale)),
                         (x + (1.8449808f * scale)),(y + (1.4981354f * scale)),
                         (x + (1.8449808f * scale)),(y + (1.4117955f * scale)));
    path.AddCurveToPoint((x + (1.8449808f * scale)),(y + (1.338248f * scale)),
                         (x + (1.8929471f * scale)),(y + (1.2774905f * scale)),
                         (x + (1.9632971f * scale)),(y + (1.2583042f * scale)));
    
    //painter.SetBrush( *wxRED_BRUSH );
    //painter.DrawRectangle((x + (2.1167896f * scale)),(y + (1.2966768f * scale)), 5, 5);
    
    painter.SetPen( *wxBLACK_PEN );
    painter.SetBrush( *wxBLACK_BRUSH );
    painter.FillPath(path);
}

/**
 * Based on TuxGuitar render routines, released under GNU GPL
 * (c) Julian Gabriel Casadesus and others
 */
void paintNote(wxGraphicsContext& painter, float x, float y, float scale, bool hollow)
{
    painter.SetPen( *wxBLACK_PEN );
    painter.SetBrush( *wxBLACK_BRUSH );
    
    wxGraphicsPath path = painter.CreatePath();
    
    path.MoveToPoint( x ,( y + (0.66f * scale) ));
    path.AddCurveToPoint( x,( y + (0.83f * scale) ),( x + (0.166f * scale) ),( y + (1.0f * scale) ),( x + (0.33f * scale) ),( y + (1.0f * scale) ));
    path.AddCurveToPoint(( x + (0.83f * scale) ),( y + (1.0f * scale) ),( x + (1.33f * scale) ),( y + (0.66f * scale) ),( x + (1.33f * scale) ),( y + (0.33f * scale) ));
    path.AddCurveToPoint(( x + (1.33f * scale) ),( y + (0.166f * scale) ),( x + (1.16f * scale) ), y ,( x + (1.0f * scale) ), y );
    path.AddCurveToPoint(( x + (0.5f * scale) ), y , x ,( y + (0.33f * scale) ), x ,( y + (0.66f * scale) ));
    
    if (hollow)
    {
        float xscale = scale*0.7f;
        float yscale = scale*0.8f;
        x += scale/4.5f;
        y += scale/9.0f;
        
        path.MoveToPoint( x ,( y + (0.66f * yscale) ));
        path.AddCurveToPoint( x,( y + (0.83f * yscale) ),
                             ( x + (0.166f * xscale) ),( y + (1.0f * yscale) ),
                             ( x + (0.33f * xscale) ),( y + (1.0f * yscale) ));
        path.AddCurveToPoint(( x + (0.83f * xscale) ),( y + (1.0f * yscale) ),
                             ( x + (1.33f * xscale) ),( y + (0.66f * yscale) ),
                             ( x + (1.33f * xscale) ),( y + (0.33f * yscale) ));
        path.AddCurveToPoint(( x + (1.33f * xscale) ),( y + (0.166f * yscale) ),
                             ( x + (1.16f * xscale) ), y ,
                             ( x + (1.0f * xscale) ), y );
        path.AddCurveToPoint(( x + (0.5f * xscale) ), y ,
                             x ,( y + (0.33f * yscale) ),
                             x ,( y + (0.66f * yscale) ));
    }
    
    painter.FillPath(path);
}

/**
 * Based on TuxGuitar render routines, released under GNU GPL
 * (c) Julian Gabriel Casadesus and others
 */
void paintFooter(wxGraphicsContext& painter, float x, float y,int dir, float scale)
{
    painter.SetPen( *wxBLACK_PEN );
    painter.SetBrush( *wxBLACK_BRUSH );
    
    wxGraphicsPath path = painter.CreatePath();
    
    path.MoveToPoint(( x + (0.64375f * scale) ),( y + ((0.00625f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.659375f * scale) ),( y + ((0.0f * scale) * dir) ),( x + (0.69375f * scale) ),( y + ((0.00625f * scale) * dir) ),( x + (0.70625f * scale) ),( y + ((0.0125f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.725f * scale) ),( y + ((0.025f * scale) * dir) ),( x + (0.73125f * scale) ),( y + ((0.03125f * scale) * dir) ),( x + (0.75f * scale) ),( y + ((0.065625f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.815625f * scale) ),( y + ((0.1875f * scale) * dir) ),( x + (0.86875f * scale) ),( y + ((0.3375f * scale) * dir) ),( x + (0.890625f * scale) ),( y + ((0.4625f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.934375f * scale) ),( y + ((0.70937496f * scale) * dir) ),( x + (0.903125f * scale) ),( y + ((0.890625f * scale) * dir) ),( x + (0.778125f * scale) ),( y + ((1.096875f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.721875f * scale) ),( y + ((1.19375f * scale) * dir) ),( x + (0.653125f * scale) ),( y + ((1.28125f * scale) * dir) ),( x + (0.5f * scale) ),( y + ((1.453125f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.340625f * scale) ),( y + ((1.6375f * scale) * dir) ),( x + (0.290625f * scale) ),( y + ((1.703125f * scale) * dir) ),( x + (0.228125f * scale) ),( y + ((1.790625f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.165625f * scale) ),( y + ((1.8875f * scale) * dir) ),( x + (0.121875f * scale) ),( y + ((1.978125f * scale) * dir) ),( x + (0.09375f * scale) ),( y + ((2.06875f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.078125f * scale) ),( y + ((2.125f * scale) * dir) ),( x + (0.065625f * scale) ),( y + ((2.209375f * scale) * dir) ),( x + (0.065625f * scale) ),( y + ((2.25625f * scale) * dir) ));
    path.AddLineToPoint( x + (0.065625f * scale) ,( y + ((2.271875f * scale) * dir) ));
    path.AddLineToPoint(( x + (0.034375f * scale) ),( y + ((2.271875f * scale) * dir) ));
    path.AddLineToPoint(( x + (0.0f * scale) ),( y + ((2.271875f * scale) * dir) ));
    path.AddLineToPoint(( x + (0.0f * scale) ),( y + ((1.88125f * scale) * dir) ));
    path.AddLineToPoint(( x + (0.0f * scale) ),( y + ((1.490625f * scale) * dir) ));
    path.AddLineToPoint(( x + (0.034375f * scale) ),( y + ((1.490625f * scale) * dir) ));
    path.AddLineToPoint(( x + (0.06875f * scale) ),( y + ((1.490625f * scale) * dir) ));
    path.AddLineToPoint(( x + (0.15f * scale) ),( y + ((1.434375f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.38125f * scale) ),( y + ((1.28125f * scale) * dir) ),( x + (0.521875f * scale) ),( y + ((1.15625f * scale) * dir) ),( x + (0.621875f * scale) ),( y + ((1.021875f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.74375f * scale) ),( y + ((0.85625f * scale) * dir) ),( x + (0.778125f * scale) ),( y + ((0.71874994f * scale) * dir) ),( x + (0.74375f * scale) ),( y + ((0.5124999f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.721875f * scale) ),( y + ((0.38125f * scale) * dir) ),( x + (0.66875f * scale) ),( y + ((0.246875f * scale) * dir) ),( x + (0.6f * scale) ),( y + ((0.128125f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.584375f * scale) ),( y + ((0.10625f * scale) * dir) ),( x + (0.58125f * scale) ),( y + ((0.096875f * scale) * dir) ),( x + (0.58125f * scale) ),( y + ((0.0875f * scale) * dir) ));
    path.AddCurveToPoint(( x + (0.58125f * scale) ),( y + ((0.05f * scale) * dir) ),( x + (0.60625f * scale) ),( y + ((0.01875f * scale) * dir) ),( x + (0.64375f * scale) ),( y + ((0.00625f * scale) * dir) ));
    
    painter.FillPath(path);
}

const float HEIGHT_SIZE = 6.26f;

/**
 * Based on TuxGuitar render routines, released under GNU GPL
 * (c) Julian Gabriel Casadesus and others
 */
void paintEighth(wxGraphicsContext& painter, float x, float y, float scale)
{
    painter.SetPen( *wxBLACK_PEN );
    painter.SetBrush( *wxBLACK_BRUSH );
    
    wxGraphicsPath path = painter.CreatePath();
    path.MoveToPoint((x + (1.6779978f * scale)),(y + (0.070901394f * scale)));
    path.AddCurveToPoint((x + (2.1979408f * scale)),(y + (0.0f * scale)),(x + (2.6469831f * scale)),(y + (0.14180231f * scale)),(x + (3.0014887f * scale)),(y + (0.47267532f * scale)));
    path.AddCurveToPoint((x + (3.214193f * scale)),(y + (0.7090125f * scale)),(x + (3.3087273f * scale)),(y + (0.89808273f * scale)),(x + (3.450531f * scale)),(y + (1.4652932f * scale)));
    path.AddCurveToPoint((x + (3.5214317f * scale)),(y + (1.7252648f * scale)),(x + (3.5923345f * scale)),(y + (1.9852359f * scale)),(x + (3.6159685f * scale)),(y + (2.0325048f * scale)));
    path.AddCurveToPoint((x + (3.7105024f * scale)),(y + (2.2452075f * scale)),(x + (3.8759398f * scale)),(y + (2.3870108f * scale)),(x + (4.0886426f * scale)),(y + (2.4106443f * scale)));
    path.AddCurveToPoint((x + (4.183178f * scale)),(y + (2.4106443f * scale)),(x + (4.2304454f * scale)),(y + (2.4106443f * scale)),(x + (4.348614f * scale)),(y + (2.3397439f * scale)));
    path.AddCurveToPoint((x + (4.6558537f * scale)),(y + (2.19794f * scale)),(x + (5.388501f * scale)),(y + (1.3234913f * scale)),(x + (5.6721063f * scale)),(y + (0.7799146f * scale)));
    path.AddCurveToPoint((x + (5.7430067f * scale)),(y + (0.6144779f * scale)),(x + (5.861175f * scale)),(y + (0.5435767f * scale)),(x + (5.979345f * scale)),(y + (0.5435767f * scale)));
    path.AddCurveToPoint((x + (6.073879f * scale)),(y + (0.5435767f * scale)),(x + (6.192049f * scale)),(y + (0.6144779f * scale)),(x + (6.239316f * scale)),(y + (0.6853788f * scale)));
    path.AddCurveToPoint((x + (6.26295f * scale)),(y + (0.7326472f * scale)),(x + (6.026612f * scale)),(y + (1.5598292f * scale)),(x + (4.7740216f * scale)),(y + (5.7430067f * scale)));
    path.AddCurveToPoint((x + (3.970474f * scale)),(y + (8.484522f * scale)),(x + (3.2850938f * scale)),(y + (10.776992f * scale)),(x + (3.2614603f * scale)),(y + (10.800634f * scale)));
    path.AddCurveToPoint((x + (3.2614603f * scale)),(y + (10.824278f * scale)),(x + (3.1669261f * scale)),(y + (10.895159f * scale)),(x + (3.0723903f * scale)),(y + (10.918801f * scale)));
    path.AddCurveToPoint((x + (2.9305882f * scale)),(y + (10.989707f * scale)),(x + (2.9069548f * scale)),(y + (10.989707f * scale)),(x + (2.7178838f * scale)),(y + (10.989707f * scale)));
    path.AddCurveToPoint((x + (2.528813f * scale)),(y + (10.989707f * scale)),(x + (2.481546f * scale)),(y + (10.989707f * scale)),(x + (2.3397446f * scale)),(y + (10.942445f * scale)));
    path.AddCurveToPoint((x + (2.2688415f * scale)),(y + (10.895159f * scale)),(x + (2.1743073f * scale)),(y + (10.847896f * scale)),(x + (2.1743073f * scale)),(y + (10.824278f * scale)));
    path.AddCurveToPoint((x + (2.1270401f * scale)),(y + (10.800634f * scale)),(x + (2.292475f * scale)),(y + (10.375227f * scale)),(x + (3.4977982f * scale)),(y + (6.94833f * scale)));
    path.AddCurveToPoint((x + (4.254079f * scale)),(y + (4.844924f * scale)),(x + (4.8685584f * scale)),(y + (3.0960243f * scale)),(x + (4.8449225f * scale)),(y + (3.0960243f * scale)));
    path.AddLineToPoint((x + (4.4431496f * scale)),(y + (3.2141926f * scale)));
    path.AddCurveToPoint((x + (3.5923345f * scale)),(y + (3.4977977f * scale)),(x + (3.0723903f * scale)),(y + (3.5923336f * scale)),(x + (2.4342787f * scale)),(y + (3.5923336f * scale)));
    path.AddCurveToPoint((x + (1.914336f * scale)),(y + (3.5923336f * scale)),(x + (1.7725322f * scale)),(y + (3.5687f * scale)),(x + (1.394393f * scale)),(y + (3.3796299f * scale)));
    path.AddCurveToPoint((x + (0.3545066f * scale)),(y + (2.88332f * scale)),(x + (0.0f * scale)),(y + (1.6779974f * scale)),(x + (0.5908443f * scale)),(y + (0.7799146f * scale)));
    path.AddCurveToPoint((x + (0.85081583f * scale)),(y + (0.4254074f * scale)),(x + (1.2525895f * scale)),(y + (0.14180231f * scale)),(x + (1.6779978f * scale)),(y + (0.070901394f * scale)));
    
    painter.FillPath(path);
}

const float QUARTER_SIZE = 6.4f;

/**
 * Based on TuxGuitar render routines, released under GNU GPL
 * (c) Julian Gabriel Casadesus and others
 */
void paintQuarter(wxGraphicsContext& painter, float x, float y, float scale)
{
    painter.SetPen( *wxBLACK_PEN );
    painter.SetBrush( *wxBLACK_BRUSH );
    
    wxGraphicsPath path = painter.CreatePath();
    
    path.MoveToPoint((x + (2.1034088f * scale)),(y + (0.047267675f * scale)));
    path.AddCurveToPoint((x + (2.1979485f * scale)),(y + (0.0f * scale)),(x + (2.2924728f * scale)),(y + (0.0f * scale)),(x + (2.387024f * scale)),(y + (0.023633957f * scale)));
    path.AddCurveToPoint((x + (2.4579277f * scale)),(y + (0.070901394f * scale)),(x + (6.121151f * scale)),(y + (4.4195156f * scale)),(x + (6.2156906f * scale)),(y + (4.6085863f * scale)));
    path.AddCurveToPoint((x + (6.38113f * scale)),(y + (4.915825f * scale)),(x + (6.333866f * scale)),(y + (5.2230635f * scale)),(x + (6.0975113f * scale)),(y + (5.601205f * scale)));
    path.AddCurveToPoint((x + (5.9084587f * scale)),(y + (5.8611765f * scale)),(x + (5.5303154f * scale)),(y + (6.239316f * scale)),(x + (4.821308f * scale)),(y + (6.830161f * scale)));
    path.AddCurveToPoint((x + (4.4667816f * scale)),(y + (7.1137667f * scale)),(x + (4.1359215f * scale)),(y + (7.4210052f * scale)),(x + (4.065014f * scale)),(y + (7.491905f * scale)));
    path.AddCurveToPoint((x + (3.3323631f * scale)),(y + (8.271823f * scale)),(x + (3.166935f * scale)),(y + (9.453509f * scale)),(x + (3.5923424f * scale)),(y + (10.351589f * scale)));
    path.AddCurveToPoint((x + (3.73415f * scale)),(y + (10.65883f * scale)),(x + (3.8050346f * scale)),(y + (10.729736f * scale)),(x + (4.6322327f * scale)),(y + (11.675077f * scale)));
    path.AddCurveToPoint((x + (6.1447945f * scale)),(y + (13.447618f * scale)),(x + (6.0738907f * scale)),(y + (13.376713f * scale)),(x + (6.0738907f * scale)),(y + (13.518524f * scale)));
    path.AddCurveToPoint((x + (6.0738907f * scale)),(y + (13.66031f * scale)),(x + (5.9320793f * scale)),(y + (13.802121f * scale)),(x + (5.790291f * scale)),(y + (13.825765f * scale)));
    path.AddCurveToPoint((x + (5.6721115f * scale)),(y + (13.825765f * scale)),(x + (5.5775757f * scale)),(y + (13.778487f * scale)),(x + (5.4121437f * scale)),(y + (13.6130495f * scale)));
    path.AddCurveToPoint((x + (4.821308f * scale)),(y + (13.02221f * scale)),(x + (3.4741745f * scale)),(y + (12.5968f * scale)),(x + (2.7415237f * scale)),(y + (12.762231f * scale)));
    path.AddCurveToPoint((x + (2.387024f * scale)),(y + (12.833136f * scale)),(x + (2.1979485f * scale)),(y + (12.974947f * scale)),(x + (2.0325203f * scale)),(y + (13.305807f * scale)));
    path.AddCurveToPoint((x + (1.9379692f * scale)),(y + (13.542143f * scale)),(x + (1.8670654f * scale)),(y + (13.825765f * scale)),(x + (1.8434448f * scale)),(y + (14.203887f * scale)));
    path.AddCurveToPoint((x + (1.8198013f * scale)),(y + (14.7238455f * scale)),(x + (1.9143524f * scale)),(y + (15.267421f * scale)),(x + (2.0797806f * scale)),(y + (15.787354f * scale)));
    path.AddCurveToPoint((x + (2.1979485f * scale)),(y + (16.118237f * scale)),(x + (2.2688522f * scale)),(y + (16.307312f * scale)),(x + (2.4342842f * scale)),(y + (16.543648f * scale)));
    path.AddCurveToPoint((x + (2.5760956f * scale)),(y + (16.73272f * scale)),(x + (2.5760956f * scale)),(y + (16.803608f * scale)),(x + (2.4579277f * scale)),(y + (16.874508f * scale)));
    path.AddCurveToPoint((x + (2.3633804f * scale)),(y + (16.921787f * scale)),(x + (2.3161163f * scale)),(y + (16.898151f * scale)),(x + (2.1034088f * scale)),(y + (16.638172f * scale)));
    path.AddCurveToPoint((x + (1.2526054f * scale)),(y + (15.551019f * scale)),(x + (0.47267532f * scale)),(y + (13.991195f * scale)),(x + (0.21271515f * scale)),(y + (12.833136f * scale)));
    path.AddCurveToPoint((x + (0.0f * scale)),(y + (12.005961f * scale)),(x + (0.09454727f * scale)),(y + (11.438742f * scale)),(x + (0.4254074f * scale)),(y + (11.202407f * scale)));
    path.AddCurveToPoint((x + (0.590847f * scale)),(y + (11.107882f * scale)),(x + (0.75629425f * scale)),(y + (11.084239f * scale)),(x + (1.1107941f * scale)),(y + (11.084239f * scale)));
    path.AddCurveToPoint((x + (1.7961845f * scale)),(y + (11.131502f * scale)),(x + (2.7651672f * scale)),(y + (11.3914795f * scale)),(x + (3.7105103f * scale)),(y + (11.793245f * scale)));
    path.AddLineToPoint((x + (4.01775f * scale)),(y + (11.935056f * scale)));
    path.AddLineToPoint((x + (2.7178802f * scale)),(y + (10.398851f * scale)));
    path.AddCurveToPoint((x + (0.73265076f * scale)),(y + (8.059115f * scale)),(x + (0.66174316f * scale)),(y + (7.9645834f * scale)),(x + (0.590847f * scale)),(y + (7.8227787f * scale)));
    path.AddCurveToPoint((x + (0.5199585f * scale)),(y + (7.63371f * scale)),(x + (0.496315f * scale)),(y + (7.4446383f * scale)),(x + (0.5672188f * scale)),(y + (7.2319345f * scale)));
    path.AddCurveToPoint((x + (0.68538666f * scale)),(y + (6.853795f * scale)),(x + (1.0162506f * scale)),(y + (6.4756536f * scale)),(x + (2.0088768f * scale)),(y + (5.648472f * scale)));
    path.AddCurveToPoint((x + (2.3633804f * scale)),(y + (5.3648667f * scale)),(x + (2.67062f * scale)),(y + (5.0812616f * scale)),(x + (2.7415237f * scale)),(y + (5.033994f * scale)));
    path.AddCurveToPoint((x + (3.2851028f * scale)),(y + (4.443149f * scale)),(x + (3.5450783f * scale)),(y + (3.6159673f * scale)),(x + (3.450531f * scale)),(y + (2.8360534f * scale)));
    path.AddCurveToPoint((x + (3.4032707f * scale)),(y + (2.5760818f * scale)),(x + (3.2851028f * scale)),(y + (2.1743064f * scale)),(x + (3.1432915f * scale)),(y + (1.9616027f * scale)));
    path.AddCurveToPoint((x + (3.0723915f * scale)),(y + (1.8434343f * scale)),(x + (2.7887878f * scale)),(y + (1.4889283f * scale)),(x + (2.505188f * scale)),(y + (1.1344206f * scale)));
    path.AddCurveToPoint((x + (2.2215881f * scale)),(y + (0.8035486f * scale)),(x + (1.9616127f * scale)),(y + (0.49630952f * scale)),(x + (1.9379692f * scale)),(y + (0.44904137f * scale)));
    path.AddCurveToPoint((x + (1.8670654f * scale)),(y + (0.30723906f * scale)),(x + (1.9379692f * scale)),(y + (0.11816859f * scale)),(x + (2.1034088f * scale)),(y + (0.047267675f * scale)));
    painter.FillPath(path);
}

const int SIXTEENTH_SIZE = 8.0f;

/**
 * Based on TuxGuitar render routines, released under GNU GPL
 * (c) Julian Gabriel Casadesus and others
 */
void paintSixteenth(wxGraphicsContext& painter, float x, float y,float scale)
{
    painter.SetPen( *wxBLACK_PEN );
    painter.SetBrush( *wxBLACK_BRUSH );
    
    wxGraphicsPath path = painter.CreatePath();
    
    path.MoveToPoint((x + (3.5214243f * scale)),(y + (0.070901394f * scale)));
    path.AddCurveToPoint((x + (4.041381f * scale)),(y + (0.0f * scale)),(x + (4.490409f * scale)),(y + (0.14180231f * scale)),(x + (4.8449125f * scale)),(y + (0.4726758f * scale)));
    path.AddCurveToPoint((x + (5.0576286f * scale)),(y + (0.7090137f * scale)),(x + (5.152153f * scale)),(y + (0.89808273f * scale)),(x + (5.2939644f * scale)),(y + (1.4652946f * scale)));
    path.AddCurveToPoint((x + (5.36487f * scale)),(y + (1.725266f * scale)),(x + (5.435775f * scale)),(y + (1.9852376f * scale)),(x + (5.4593954f * scale)),(y + (2.032505f * scale)));
    path.AddCurveToPoint((x + (5.5775614f * scale)),(y + (2.3397436f * scale)),(x + (5.9084463f * scale)),(y + (2.4815469f * scale)),(x + (6.144781f * scale)),(y + (2.363377f * scale)));
    path.AddCurveToPoint((x + (6.4520226f * scale)),(y + (2.2215748f * scale)),(x + (7.090123f * scale)),(y + (1.394392f * scale)),(x + (7.421006f * scale)),(y + (0.7326474f * scale)));
    path.AddCurveToPoint((x + (7.4682703f * scale)),(y + (0.6144779f * scale)),(x + (7.6100817f * scale)),(y + (0.54357696f * scale)),(x + (7.704606f * scale)),(y + (0.54357696f * scale)));
    path.AddCurveToPoint((x + (7.7991533f * scale)),(y + (0.54357696f * scale)),(x + (7.917321f * scale)),(y + (0.6144779f * scale)),(x + (7.9645834f * scale)),(y + (0.6853788f * scale)));
    path.AddCurveToPoint((x + (7.988206f * scale)),(y + (0.7326474f * scale)),(x + (7.6100817f * scale)),(y + (2.150673f * scale)),(x + (5.861184f * scale)),(y + (8.6972275f * scale)));
    path.AddCurveToPoint((x + (4.6794825f * scale)),(y + (13.069472f * scale)),(x + (3.710497f * scale)),(y + (16.685432f * scale)),(x + (3.6868773f * scale)),(y + (16.709076f * scale)));
    path.AddCurveToPoint((x + (3.6632347f * scale)),(y + (16.73272f * scale)),(x + (3.592329f * scale)),(y + (16.803608f * scale)),(x + (3.4978046f * scale)),(y + (16.827246f * scale)));
    path.AddCurveToPoint((x + (3.3559942f * scale)),(y + (16.89815f * scale)),(x + (3.3323498f * scale)),(y + (16.89815f * scale)),(x + (3.143302f * scale)),(y + (16.89815f * scale)));
    path.AddCurveToPoint((x + (2.9542284f * scale)),(y + (16.89815f * scale)),(x + (2.9069443f * scale)),(y + (16.89815f * scale)),(x + (2.7651548f * scale)),(y + (16.850887f * scale)));
    path.AddCurveToPoint((x + (2.69425f * scale)),(y + (16.803608f * scale)),(x + (2.5997238f * scale)),(y + (16.75634f * scale)),(x + (2.5997238f * scale)),(y + (16.73272f * scale)));
    path.AddCurveToPoint((x + (2.5524387f * scale)),(y + (16.709076f * scale)),(x + (2.69425f * scale)),(y + (16.283669f * scale)),(x + (3.7341404f * scale)),(y + (12.856779f * scale)));
    path.AddCurveToPoint((x + (4.372241f * scale)),(y + (10.753368f * scale)),(x + (4.8921986f * scale)),(y + (9.028101f * scale)),(x + (4.8921986f * scale)),(y + (9.004459f * scale)));
    path.AddCurveToPoint((x + (4.868556f * scale)),(y + (9.004459f * scale)),(x + (4.6794825f * scale)),(y + (9.051743f * scale)),(x + (4.46679f * scale)),(y + (9.122626f * scale)));
    path.AddCurveToPoint((x + (3.6159725f * scale)),(y + (9.406247f * scale)),(x + (3.0723963f * scale)),(y + (9.500772f * scale)),(x + (2.43427f * scale)),(y + (9.500772f * scale)));
    path.AddCurveToPoint((x + (1.9143381f * scale)),(y + (9.500772f * scale)),(x + (1.7725258f * scale)),(y + (9.477153f * scale)),(x + (1.3943825f * scale)),(y + (9.28808f * scale)));
    path.AddCurveToPoint((x + (0.3545103f * scale)),(y + (8.791764f * scale)),(x + (0.0f * scale)),(y + (7.586442f * scale)),(x + (0.59084797f * scale)),(y + (6.6883574f * scale)));
    path.AddCurveToPoint((x + (1.1580448f * scale)),(y + (5.8611765f * scale)),(x + (2.3397465f * scale)),(y + (5.7193727f * scale)),(x + (3.0014915f * scale)),(y + (6.357486f * scale)));
    path.AddCurveToPoint((x + (3.2378254f * scale)),(y + (6.6174574f * scale)),(x + (3.3323498f * scale)),(y + (6.830161f * scale)),(x + (3.4741611f * scale)),(y + (7.4446383f * scale)));
    path.AddCurveToPoint((x + (3.5450668f * scale)),(y + (7.799144f * scale)),(x + (3.6395912f * scale)),(y + (8.059116f * scale)),(x + (3.7341404f * scale)),(y + (8.153648f * scale)));
    path.AddCurveToPoint((x + (3.805046f * scale)),(y + (8.224551f * scale)),(x + (3.9468327f * scale)),(y + (8.295452f * scale)),(x + (4.088643f * scale)),(y + (8.31909f * scale)));
    path.AddCurveToPoint((x + (4.1831684f * scale)),(y + (8.31909f * scale)),(x + (4.2304544f * scale)),(y + (8.31909f * scale)),(x + (4.3486233f * scale)),(y + (8.248187f * scale)));
    path.AddCurveToPoint((x + (4.608577f * scale)),(y + (8.130019f * scale)),(x + (5.152153f * scale)),(y + (7.515539f * scale)),(x + (5.4593954f * scale)),(y + (6.995596f * scale)));
    path.AddCurveToPoint((x + (5.5539417f * scale)),(y + (6.830161f * scale)),(x + (5.601205f * scale)),(y + (6.6883574f * scale)),(x + (5.766636f * scale)),(y + (6.144781f * scale)));
    path.AddCurveToPoint((x + (6.2865934f * scale)),(y + (4.4904165f * scale)),(x + (6.6883574f * scale)),(y + (3.119658f * scale)),(x + (6.6883574f * scale)),(y + (3.0960245f * scale)));
    path.AddCurveToPoint((x + (6.6883574f * scale)),(y + (3.0960245f * scale)),(x + (6.5229273f * scale)),(y + (3.1432915f * scale)),(x + (6.3574743f * scale)),(y + (3.2141924f * scale)));
    path.AddCurveToPoint((x + (5.9557085f * scale)),(y + (3.3323627f * scale)),(x + (5.4121323f * scale)),(y + (3.474164f * scale)),(x + (5.033985f * scale)),(y + (3.5450664f * scale)));
    path.AddCurveToPoint((x + (4.7740307f * scale)),(y + (3.5923338f * scale)),(x + (4.6322203f * scale)),(y + (3.5923338f * scale)),(x + (4.2777157f * scale)),(y + (3.5923338f * scale)));
    path.AddCurveToPoint((x + (3.757759f * scale)),(y + (3.5923338f * scale)),(x + (3.6159725f * scale)),(y + (3.5687003f * scale)),(x + (3.2378254f * scale)),(y + (3.3796296f * scale)));
    path.AddCurveToPoint((x + (2.197935f * scale)),(y + (2.8833203f * scale)),(x + (1.8434324f * scale)),(y + (1.6779971f * scale)),(x + (2.43427f * scale)),(y + (0.77991486f * scale)));
    path.AddCurveToPoint((x + (2.69425f * scale)),(y + (0.4254074f * scale)),(x + (3.096015f * scale)),(y + (0.14180231f * scale)),(x + (3.5214243f * scale)),(y + (0.070901394f * scale)));
    painter.FillPath(path);
}

/**
 * Based on TuxGuitar render routines, released under GNU GPL
 * (c) Julian Gabriel Casadesus and others
 */
void paintThirtySecond(wxGraphicsContext& painter, float x, float y, float scale)
{
    painter.SetPen( *wxBLACK_PEN );
    painter.SetBrush( *wxBLACK_BRUSH );
    
    wxGraphicsPath path = painter.CreatePath();
    
    path.MoveToPoint((x + (4.939474f * scale)),(y + (0.070901155f * scale)));
    path.AddCurveToPoint((x + (5.459408f * scale)),(y + (0.0f * scale)),(x + (5.9084606f * scale)),(y + (0.14180207f * scale)),(x + (6.2629623f * scale)),(y + (0.4726758f * scale)));
    path.AddCurveToPoint((x + (6.4756565f * scale)),(y + (0.70901346f * scale)),(x + (6.570204f * scale)),(y + (0.89808273f * scale)),(x + (6.7119904f * scale)),(y + (1.4652941f * scale)));
    path.AddCurveToPoint((x + (6.782896f * scale)),(y + (1.7252657f * scale)),(x + (6.8538036f * scale)),(y + (1.9852374f * scale)),(x + (6.877445f * scale)),(y + (2.0325048f * scale)));
    path.AddCurveToPoint((x + (6.9719696f * scale)),(y + (2.245209f * scale)),(x + (7.1373997f * scale)),(y + (2.3870108f * scale)),(x + (7.3501167f * scale)),(y + (2.4106443f * scale)));
    path.AddCurveToPoint((x + (7.444641f * scale)),(y + (2.4106443f * scale)),(x + (7.4919033f * scale)),(y + (2.4106443f * scale)),(x + (7.562809f * scale)),(y + (2.3633773f * scale)));
    path.AddCurveToPoint((x + (7.8227882f * scale)),(y + (2.19794f * scale)),(x + (8.34272f * scale)),(y + (1.4889283f * scale)),(x + (8.697241f * scale)),(y + (0.80354834f * scale)));
    path.AddCurveToPoint((x + (8.768129f * scale)),(y + (0.61447763f * scale)),(x + (8.862679f * scale)),(y + (0.54357696f * scale)),(x + (9.004465f * scale)),(y + (0.54357696f * scale)));
    path.AddCurveToPoint((x + (9.099014f * scale)),(y + (0.54357696f * scale)),(x + (9.217182f * scale)),(y + (0.61447763f * scale)),(x + (9.264444f * scale)),(y + (0.6853788f * scale)));
    path.AddCurveToPoint((x + (9.288086f * scale)),(y + (0.7326474f * scale)),(x + (8.815416f * scale)),(y + (2.7887857f * scale)),(x + (6.735634f * scale)),(y + (11.627814f * scale)));
    path.AddCurveToPoint((x + (5.3175983f * scale)),(y + (17.6308f * scale)),(x + (4.1359196f * scale)),(y + (22.57025f * scale)),(x + (4.1359196f * scale)),(y + (22.593893f * scale)));
    path.AddCurveToPoint((x + (4.112276f * scale)),(y + (22.641155f * scale)),(x + (4.041378f * scale)),(y + (22.688417f * scale)),(x + (3.946846f * scale)),(y + (22.73568f * scale)));
    path.AddCurveToPoint((x + (3.781416f * scale)),(y + (22.806585f * scale)),(x + (3.7577744f * scale)),(y + (22.806585f * scale)),(x + (3.5687008f * scale)),(y + (22.806585f * scale)));
    path.AddCurveToPoint((x + (3.3796272f * scale)),(y + (22.806585f * scale)),(x + (3.332365f * scale)),(y + (22.806585f * scale)),(x + (3.1905785f * scale)),(y + (22.759323f * scale)));
    path.AddCurveToPoint((x + (3.1196728f * scale)),(y + (22.71206f * scale)),(x + (3.0251236f * scale)),(y + (22.664799f * scale)),(x + (3.0251236f * scale)),(y + (22.641155f * scale)));
    path.AddCurveToPoint((x + (2.9778614f * scale)),(y + (22.617512f * scale)),(x + (3.0960293f * scale)),(y + (22.192102f * scale)),(x + (3.9941082f * scale)),(y + (18.765215f * scale)));
    path.AddCurveToPoint((x + (4.5376854f * scale)),(y + (16.661814f * scale)),(x + (4.986738f * scale)),(y + (14.936537f * scale)),(x + (5.0103607f * scale)),(y + (14.912916f * scale)));
    path.AddCurveToPoint((x + (5.0103607f * scale)),(y + (14.889275f * scale)),(x + (4.9158325f * scale)),(y + (14.889275f * scale)),(x + (4.6322346f * scale)),(y + (15.007441f * scale)));
    path.AddCurveToPoint((x + (3.7105122f * scale)),(y + (15.314682f * scale)),(x + (2.883337f * scale)),(y + (15.456493f * scale)),(x + (2.2688541f * scale)),(y + (15.43285f * scale)));
    path.AddCurveToPoint((x + (1.9379692f * scale)),(y + (15.409229f * scale)),(x + (1.6780167f * scale)),(y + (15.3383255f * scale)),(x + (1.3943939f * scale)),(y + (15.196516f * scale)));
    path.AddCurveToPoint((x + (0.35450363f * scale)),(y + (14.700201f * scale)),(x + (0.0f * scale)),(y + (13.49488f * scale)),(x + (0.59086037f * scale)),(y + (12.5968f * scale)));
    path.AddCurveToPoint((x + (1.1580582f * scale)),(y + (11.769626f * scale)),(x + (2.3397598f * scale)),(y + (11.627814f * scale)),(x + (3.001505f * scale)),(y + (12.265927f * scale)));
    path.AddCurveToPoint((x + (3.2378407f * scale)),(y + (12.525894f * scale)),(x + (3.332365f * scale)),(y + (12.738611f * scale)),(x + (3.4741764f * scale)),(y + (13.353088f * scale)));
    path.AddCurveToPoint((x + (3.5450802f * scale)),(y + (13.707598f * scale)),(x + (3.6396065f * scale)),(y + (13.967552f * scale)),(x + (3.7341537f * scale)),(y + (14.062099f * scale)));
    path.AddCurveToPoint((x + (3.8286781f * scale)),(y + (14.133005f * scale)),(x + (3.9704895f * scale)),(y + (14.203886f * scale)),(x + (4.112276f * scale)),(y + (14.22753f * scale)));
    path.AddCurveToPoint((x + (4.2068253f * scale)),(y + (14.22753f * scale)),(x + (4.2540874f * scale)),(y + (14.22753f * scale)),(x + (4.348612f * scale)),(y + (14.156624f * scale)));
    path.AddCurveToPoint((x + (4.679497f * scale)),(y + (13.991194f * scale)),(x + (5.3648834f * scale)),(y + (13.140376f * scale)),(x + (5.5775776f * scale)),(y + (12.667706f * scale)));
    path.AddCurveToPoint((x + (5.601219f * scale)),(y + (12.573157f * scale)),(x + (6.546562f * scale)),(y + (9.051743f * scale)),(x + (6.546562f * scale)),(y + (9.004459f * scale)));
    path.AddCurveToPoint((x + (6.546562f * scale)),(y + (8.980838f * scale)),(x + (6.404751f * scale)),(y + (9.0281f * scale)),(x + (6.2393208f * scale)),(y + (9.099006f * scale)));
    path.AddCurveToPoint((x + (5.7430058f * scale)),(y + (9.240808f * scale)),(x + (5.2939777f * scale)),(y + (9.382604f * scale)),(x + (4.8685703f * scale)),(y + (9.453508f * scale)));
    path.AddCurveToPoint((x + (4.561329f * scale)),(y + (9.5007715f * scale)),(x + (4.443161f * scale)),(y + (9.5007715f * scale)),(x + (4.065014f * scale)),(y + (9.5007715f * scale)));
    path.AddCurveToPoint((x + (3.5450802f * scale)),(y + (9.5007715f * scale)),(x + (3.4032707f * scale)),(y + (9.477151f * scale)),(x + (3.0251236f * scale)),(y + (9.288079f * scale)));
    path.AddCurveToPoint((x + (1.9852371f * scale)),(y + (8.791764f * scale)),(x + (1.6307297f * scale)),(y + (7.586441f * scale)),(x + (2.2215939f * scale)),(y + (6.6883574f * scale)));
    path.AddCurveToPoint((x + (2.7887878f * scale)),(y + (5.8611755f * scale)),(x + (3.9704895f * scale)),(y + (5.7193727f * scale)),(x + (4.6322346f * scale)),(y + (6.357485f * scale)));
    path.AddCurveToPoint((x + (4.8685703f * scale)),(y + (6.6174564f * scale)),(x + (4.9630947f * scale)),(y + (6.830161f * scale)),(x + (5.104904f * scale)),(y + (7.4446383f * scale)));
    path.AddCurveToPoint((x + (5.175812f * scale)),(y + (7.799144f * scale)),(x + (5.270336f * scale)),(y + (8.059115f * scale)),(x + (5.3648834f * scale)),(y + (8.153648f * scale)));
    path.AddCurveToPoint((x + (5.50667f * scale)),(y + (8.295452f * scale)),(x + (5.7666492f * scale)),(y + (8.366357f * scale)),(x + (5.9320793f * scale)),(y + (8.271823f * scale)));
    path.AddCurveToPoint((x + (6.168415f * scale)),(y + (8.177285f * scale)),(x + (6.4756565f * scale)),(y + (7.8227797f * scale)),(x + (6.782896f * scale)),(y + (7.3501034f * scale)));
    path.AddCurveToPoint((x + (7.113781f * scale)),(y + (6.877428f * scale)),(x + (7.0901375f * scale)),(y + (6.948329f * scale)),(x + (7.610079f * scale)),(y + (4.963092f * scale)));
    path.AddCurveToPoint((x + (7.8700504f * scale)),(y + (3.970473f * scale)),(x + (8.082766f * scale)),(y + (3.1432917f * scale)),(x + (8.082766f * scale)),(y + (3.1196578f * scale)));
    path.AddCurveToPoint((x + (8.082766f * scale)),(y + (3.1196578f * scale)),(x + (7.9173126f * scale)),(y + (3.1432917f * scale)),(x + (7.72826f * scale)),(y + (3.2141926f * scale)));
    path.AddCurveToPoint((x + (7.279211f * scale)),(y + (3.355996f * scale)),(x + (6.830158f * scale)),(y + (3.4741638f * scale)),(x + (6.452036f * scale)),(y + (3.545066f * scale)));
    path.AddCurveToPoint((x + (6.1920567f * scale)),(y + (3.592334f * scale)),(x + (6.050247f * scale)),(y + (3.592334f * scale)),(x + (5.6957436f * scale)),(y + (3.592334f * scale)));
    path.AddCurveToPoint((x + (5.175812f * scale)),(y + (3.592334f * scale)),(x + (5.0339985f * scale)),(y + (3.5687f * scale)),(x + (4.6558533f * scale)),(y + (3.3796294f * scale)));
    path.AddCurveToPoint((x + (3.6159801f * scale)),(y + (2.88332f * scale)),(x + (3.2614594f * scale)),(y + (1.6779974f * scale)),(x + (3.8523216f * scale)),(y + (0.7799146f * scale)));
    path.AddCurveToPoint((x + (4.112276f * scale)),(y + (0.4254074f * scale)),(x + (4.5140667f * scale)),(y + (0.14180207f * scale)),(x + (4.939474f * scale)),(y + (0.070901155f * scale)));
    painter.FillPath(path);
}

void AriaMaestosa::RenderRoutines::drawSilence(wxGraphicsContext& dc, const Range<int> x, const int y,
                                               const int levelHeight, const int type, const bool triplet,
                                               const bool dotted)
{
    const int x_center = (x.from + x.to)/2;
    
    int silence_center = -1;
    int silence_radius = -1;
    
    if ( type == 1 )
    {
        dc.SetPen(  *wxTRANSPARENT_PEN  );
        silence_radius = RECTANGULAR_SILENCE_SIZE/2;
        
        dc.DrawRectangle(/* x */ x.from + RECTANGULAR_SILENCE_LEFT_MARGIN,
                          /* y */ y,
                          /* w */ RECTANGULAR_SILENCE_SIZE,
                          /* h */ (int)round(levelHeight/2.2f));
        silence_center = x.from + RECTANGULAR_SILENCE_LEFT_MARGIN + silence_radius;
    }
    else if ( type == 2 )
    {
        silence_radius = RECTANGULAR_SILENCE_SIZE/2;
        dc.SetPen(  *wxTRANSPARENT_PEN  );
        
        const int h = (int)round(levelHeight/2.2f);
        
        dc.DrawRectangle(/* x */ x.from + RECTANGULAR_SILENCE_LEFT_MARGIN,
                         /* y */ y + levelHeight - h ,
                         /* w */ RECTANGULAR_SILENCE_SIZE,
                         /* h */ h);
        silence_center = x.from + RECTANGULAR_SILENCE_LEFT_MARGIN + silence_radius;
    }
    else if ( type == 4 )
    {
        float scale = 10.0f;
        silence_radius = int(QUARTER_SIZE*scale/2.0f);
        
        // for dotted silences, place them much closer to the left area, to leave room at the right for the dot
        if (dotted) silence_center = (x.to - silence_radius*2);
        else        silence_center = (x_center + (x.to - silence_radius))/2;
        
        paintQuarter(dc, silence_center - silence_radius, y, scale);
        
        if (dotted) silence_center = (x.from + silence_radius*2);
        else        silence_center = (x_center + (x.to - silence_radius))/2;
    }
    else if ( type == 8 )
    {
        float scale = 10.0f;
        silence_radius = int(scale*HEIGHT_SIZE/2.0f);
        
        if (dotted) silence_center = (x.to - silence_radius*2 - 30);
        else        silence_center = (x.to - silence_radius*2);
        
        paintEighth(dc, silence_center, y + 50, scale);
        
        if (dotted) silence_center = (x.from + silence_radius*2);
        else        silence_center = (x_center + (x.to - silence_radius))/2;
    }
    else if ( type == 16 )
    {
        float scale = 10.0f;
        silence_radius = int(SIXTEENTH_SIZE*scale/2.0f);
        
        if (dotted) silence_center = (x.to - silence_radius*2 - 30);
        else        silence_center = (x.to - silence_radius*2);
        
        paintSixteenth(dc, silence_center, y, scale);
        
        if (dotted) silence_center = (x.from + silence_radius*2);
        else        silence_center = (x_center + (x.to - silence_radius))/2;
    }
    
    // dotted
    if (dotted)
    {
        dc.SetPen(  wxPen( wxColour(0,0,0), 12 ) );
        dc.SetBrush( *wxBLACK_BRUSH );
        wxPoint headLocation( silence_center + silence_radius + DOT_SIZE*2, y+30 );
        dc.DrawEllipse( headLocation.x, headLocation.y, DOT_SIZE, DOT_SIZE );
    }
    
    // triplet
    if (triplet)
    {
        wxPen tiePen( wxColour(0,0,0), 10 ) ;
        dc.SetPen( tiePen );
        dc.SetBrush( *wxTRANSPARENT_BRUSH );
        
        const int radius_x = 50;
        
        const int base_y = y + 150;
        
        static bool size_calculated = false;
        static wxSize triplet_3_size;
        
        if (not size_calculated)
        {
            wxDouble width, height, descent, externalLeading;
            dc.GetTextExtent(wxT("3"), &width, &height, &descent, &externalLeading);
            triplet_3_size.x = width;
            triplet_3_size.y = height;
        }
        
        renderArc(dc, silence_center - 9, base_y, radius_x, 80);
        //dc.SetFont( dc.GetFont(), wxColour(0,0,0) );
        //dc.SetTextForeground( wxColour(0,0,0) );
        dc.DrawText( wxT("3"), silence_center - triplet_3_size.GetWidth()/3 - 11, base_y-20 );
    }
}

#endif

/**
 * Based on TuxGuitar render routines, released under GNU GPL
 * (c) Julian Gabriel Casadesus and others
 */
void AriaMaestosa::RenderRoutines::drawFlag(wxDC* dc, wxGraphicsContext* gc, const int flag_x_origin,
                                            const int flag_y, const int orient)
{
#if wxCHECK_VERSION(2,9,1) && wxUSE_GRAPHICS_CONTEXT
    wxGraphicsPath path = gc->CreatePath();
    
    const float scale = 65.0f;
    const float x = flag_x_origin + 3;
    const float y = flag_y + orient*scale*2.2f;
    
    path.MoveToPoint(( x + (0.64375f * scale) ),( y + ((0.00625f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.659375f * scale) ),( y + ((0.0f * scale) * -orient) ),
                         ( x + (0.69375f * scale) ),( y + ((0.00625f * scale) * -orient) ),
                         ( x + (0.70625f * scale) ),( y + ((0.0125f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.725f * scale) ),( y + ((0.025f * scale) * -orient) ),
                         ( x + (0.73125f * scale) ),( y + ((0.03125f * scale) * -orient) ),
                         ( x + (0.75f * scale) ),( y + ((0.065625f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.815625f * scale) ),( y + ((0.1875f * scale) * -orient) ),
                         ( x + (0.86875f * scale) ),( y + ((0.3375f * scale) * -orient) ),
                         ( x + (0.890625f * scale) ),( y + ((0.4625f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.934375f * scale) ),( y + ((0.70937496f * scale) * -orient) ),
                         ( x + (0.903125f * scale) ),( y + ((0.890625f * scale) * -orient) ),
                         ( x + (0.778125f * scale) ),( y + ((1.096875f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.721875f * scale) ),( y + ((1.19375f * scale) * -orient) ),
                         ( x + (0.653125f * scale) ),( y + ((1.28125f * scale) * -orient) ),
                         ( x + (0.5f * scale) ),( y + ((1.453125f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.340625f * scale) ),( y + ((1.6375f * scale) * -orient) ),
                         ( x + (0.290625f * scale) ),( y + ((1.703125f * scale) * -orient) ),
                         ( x + (0.228125f * scale) ),( y + ((1.790625f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.165625f * scale) ),( y + ((1.8875f * scale) * -orient) ),
                         ( x + (0.121875f * scale) ),( y + ((1.978125f * scale) * -orient) ),
                         ( x + (0.09375f * scale) ),( y + ((2.06875f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.078125f * scale) ),( y + ((2.125f * scale) * -orient) ),
                         ( x + (0.065625f * scale) ),( y + ((2.209375f * scale) * -orient) ),
                         ( x + (0.065625f * scale) ),( y + ((2.25625f * scale) * -orient) ));
    path.AddLineToPoint( x + (0.065625f * scale) ,( y + ((2.271875f * scale) * -orient) ));
    path.AddLineToPoint(( x + (0.034375f * scale) ),( y + ((2.271875f * scale) * -orient) ));
    path.AddLineToPoint(( x + (0.0f * scale) ),( y + ((2.271875f * scale) * -orient) ));
    path.AddLineToPoint(( x + (0.0f * scale) ),( y + ((1.88125f * scale) * -orient) ));
    path.AddLineToPoint(( x + (0.0f * scale) ),( y + ((1.490625f * scale) * -orient) ));
    path.AddLineToPoint(( x + (0.034375f * scale) ),( y + ((1.490625f * scale) * -orient) ));
    path.AddLineToPoint(( x + (0.06875f * scale) ),( y + ((1.490625f * scale) * -orient) ));
    path.AddLineToPoint(( x + (0.15f * scale) ),( y + ((1.434375f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.38125f * scale) ),( y + ((1.28125f * scale) * -orient) ),
                         ( x + (0.521875f * scale) ),( y + ((1.15625f * scale) * -orient) ),
                         ( x + (0.621875f * scale) ),( y + ((1.021875f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.74375f * scale) ),( y + ((0.85625f * scale) * -orient) ),
                         ( x + (0.778125f * scale) ),( y + ((0.71874994f * scale) * -orient) ),
                         ( x + (0.74375f * scale) ),( y + ((0.5124999f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.721875f * scale) ),( y + ((0.38125f * scale) * -orient) ),
                         ( x + (0.66875f * scale) ),( y + ((0.246875f * scale) * -orient) ),
                         ( x + (0.6f * scale) ),( y + ((0.128125f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.584375f * scale) ),( y + ((0.10625f * scale) * -orient) ),
                         ( x + (0.58125f * scale) ),( y + ((0.096875f * scale) * -orient) ),
                         ( x + (0.58125f * scale) ),( y + ((0.0875f * scale) * -orient) ));
    path.AddCurveToPoint(( x + (0.58125f * scale) ),( y + ((0.05f * scale) * -orient) ),
                         ( x + (0.60625f * scale) ),( y + ((0.01875f * scale) * -orient) ),
                         ( x + (0.64375f * scale) ),( y + ((0.00625f * scale) * -orient) ));
    
    gc->FillPath(path);
#else
    wxPoint points[] =
    {
        wxPoint(flag_x_origin, flag_y),
        wxPoint(flag_x_origin + 30/2,  flag_y + orient*60/2),
        wxPoint(flag_x_origin + 110/2, flag_y + orient*110/2),
        wxPoint(flag_x_origin + 90/2,  flag_y + orient*150/2)
    };
    dc->DrawSpline(4, points);
#endif
}


void AriaMaestosa::RenderRoutines::drawSilence(wxDC* dc, const Range<int> x, const int y,
                                               const int levelHeight, const int type, const bool triplet,
                                               const bool dotted)
{
    const int x_center = (x.from + x.to)/2;
    
    // debug draw
    //static int silenceShift = 0;
    //silenceShift += 5;
    //global_dc->DrawLine(x, silences_y + silenceShift % 25, x_to, silences_y + silenceShift % 25);
    
    //{ TODO : use again when repetition is properly back in
    //    LayoutElement* temp = g_printable->getElementForMeasure(measure);
    //    if (temp != NULL and (temp->getType() == REPEATED_RIFF or temp->getType() == SINGLE_REPEATED_MEASURE))
    //        return; //don't render silences in repetions measure!
    //}
    
    dc->SetBrush( *wxBLACK_BRUSH );
    
    int silence_center = -1;
    int silence_radius = -1;
    
    if ( type == 1 )
    {
        dc->SetPen(  *wxTRANSPARENT_PEN  );
        silence_radius = RECTANGULAR_SILENCE_SIZE/2;
        
        dc->DrawRectangle(/* x */ x.from + RECTANGULAR_SILENCE_LEFT_MARGIN,
                          /* y */ y,
                          /* w */ RECTANGULAR_SILENCE_SIZE,
                          /* h */ (int)round(levelHeight/2.2f));
        silence_center = x.from + RECTANGULAR_SILENCE_LEFT_MARGIN + silence_radius;
    }
    else if ( type == 2 )
    {
        silence_radius = RECTANGULAR_SILENCE_SIZE/2;
        dc->SetPen(  *wxTRANSPARENT_PEN  );
        
        const int h = (int)round(levelHeight/2.2f);
        
        dc->DrawRectangle(/* x */ x.from + RECTANGULAR_SILENCE_LEFT_MARGIN,
                          /* y */ y + levelHeight - h ,
                          /* w */ RECTANGULAR_SILENCE_SIZE,
                          /* h */ h);
        silence_center = x.from + RECTANGULAR_SILENCE_LEFT_MARGIN + silence_radius;
    }
    else if ( type == 4 )
    {
        static wxBitmap silenceBigger = getScaledBitmap(wxT("silence4.png"), 6.5f);
        
        silence_radius = silenceBigger.GetWidth()/2;
        // take the average of 'center-aligned' and 'right-aligned'
        
        // for dotted silences, place them much closer to the left area, to leave room at the right for the dot
        if (dotted) silence_center = (x.from + silence_radius*2);
        else        silence_center = (x_center + (x.to - silence_radius))/2;
        
        dc->DrawBitmap( silenceBigger, silence_center - silence_radius, y );
        
        // <debug>
        //global_dc->SetPen(  wxPen( wxColour(255,0,0), 8 ) );
        //global_dc->DrawLine( x.from, silences_y, x.to, silences_y);
        // </debug>
        
    }
    else if ( type == 8 )
    {
        static wxBitmap silenceBigger = getScaledBitmap(wxT("silence8.png"), 6.5f);
        
        silence_radius = silenceBigger.GetWidth()/2;
        
        if (dotted) silence_center = (x.to - silence_radius - 30);
        else        silence_center = (x.to - silence_radius);
        
        dc->DrawBitmap( silenceBigger, silence_center - silence_radius, y + 20);
        
        // <debug>
        //global_dc->SetPen(  wxPen( wxColour(255,0,0), 8 ) );
        //global_dc->DrawLine( x.from, silences_y, x.to, silences_y);
        // </debug>
    }
    else if ( type == 16 )
    {
        static wxBitmap silenceBigger = getScaledBitmap(wxT("silence8.png"), 6.5f);
        
        silence_radius = silenceBigger.GetWidth()/2;
        
        if (dotted) silence_center = (x.to - silence_radius - 30);
        else        silence_center = (x.to - silence_radius);
        
        dc->DrawBitmap( silenceBigger, silence_center - silence_radius, y + 20);
        dc->DrawBitmap( silenceBigger, silence_center - 10, y - 40);
        
        
        /*
         // TODO : use x_center
         dc->SetPen(  wxPen( wxColour(0,0,0), 8 ) );
         silence_radius = 25;
         const int mx = x.to - silence_radius*2;
         const int sy = y + 80;
         
         ASSERT_E(mx, >, -5000);
         ASSERT_E(sy, >, -5000);
         
         std::cout << "1/16th : x=" << mx << ", y=" << sy << "\n";
         
         wxPoint points[] =
         {
         wxPoint(mx,     sy+50),
         wxPoint(mx+25,  sy),
         wxPoint(mx,     sy),
         };
         dc->DrawSpline(3, points);
         wxPoint points2[] =
         {
         wxPoint(mx+20,  sy+5),
         wxPoint(mx+50,  sy-50),
         wxPoint(mx+25,  sy-50),
         };
         dc->DrawSpline(3, points2);
         
         dc->DrawCircle(mx,    sy, 6);
         dc->DrawCircle(mx+25, sy-50, 6);
         
         silence_center = mx + 50/2;
         */
    }
    
    // dotted
    if (dotted)
    {
        dc->SetPen(  wxPen( wxColour(0,0,0), 12 ) );
        dc->SetBrush( *wxBLACK_BRUSH );
        wxPoint headLocation( silence_center + silence_radius + DOT_SIZE*2, y+30 );
        dc->DrawEllipse( headLocation, wxSize(DOT_SIZE, DOT_SIZE) );
    }
    
    // triplet
    if (triplet)
    {
        wxPen tiePen( wxColour(0,0,0), 10 ) ;
        dc->SetPen( tiePen );
        dc->SetBrush( *wxTRANSPARENT_BRUSH );
        
        const int radius_x = 50;
        
        const int base_y = y + 150;
        
        static wxSize triplet_3_size = dc->GetTextExtent(wxT("3"));
        
        renderArc(*dc, silence_center - 9, base_y, radius_x, 80);
        dc->SetTextForeground( wxColour(0,0,0) );
        dc->DrawText( wxT("3"), silence_center - triplet_3_size.GetWidth()/3 - 11, base_y-20 );
    }
}

// -------------------------------------------------------------------------------------------------------

void AriaMaestosa::RenderRoutines::drawNoteHead(wxDC& dc, const wxPoint headCenter, const bool hollowHead)
{
    const int cx = headCenter.x + (hollowHead ? -2 : 0); // FIXME: the -2 is a hack for the head to blend in the stem
    const int cy = headCenter.y;
    wxPoint points[25];
    for (int n=0; n<25; n++)
    {
        const float angle = n/25.0*6.283185f /* 2*PI */;
        
        // FIXME - instead of always substracting to radius, just make it smaller...
        const int px = cx + (HEAD_RADIUS-5)*cos(angle);
        const int py = cy + (HEAD_RADIUS - 14)*sin(angle) - HEAD_RADIUS*(-0.5f + fabsf( (n-12.5f)/12.5f ))/2.0f;
        
        points[n] = wxPoint( px, py );
    }
    
    if (hollowHead) dc.DrawSpline(25, points);
    else            dc.DrawPolygon(25, points, -3);
}

// -------------------------------------------------------------------------------------------------------

void AriaMaestosa::RenderRoutines::renderArc(wxDC& dc, const int center_x, const int center_y,
                                           const int radius_x, const int radius_y)
{
    wxPoint points[] =
    {
        wxPoint(center_x + radius_x*cos(0.1), center_y + radius_y*sin(0.1)),
        wxPoint(center_x + radius_x*cos(0.3), center_y + radius_y*sin(0.3)),
        wxPoint(center_x + radius_x*cos(0.6), center_y + radius_y*sin(0.6)),
        wxPoint(center_x + radius_x*cos(0.9), center_y + radius_y*sin(0.9)),
        wxPoint(center_x + radius_x*cos(1.2), center_y + radius_y*sin(1.2)),
        wxPoint(center_x + radius_x*cos(1.5), center_y + radius_y*sin(1.5)),
        wxPoint(center_x + radius_x*cos(1.8), center_y + radius_y*sin(1.8)),
        wxPoint(center_x + radius_x*cos(2.1), center_y + radius_y*sin(2.1)),
        wxPoint(center_x + radius_x*cos(2.4), center_y + radius_y*sin(2.4)),
        wxPoint(center_x +  radius_x*cos(2.7), center_y + radius_y*sin(2.7)),
        wxPoint(center_x + radius_x*cos(3.0), center_y + radius_y*sin(3.0)),
    };
    dc.DrawSpline(11, points);
    
#ifdef DEBUG_TIES
    dc.SetPen(*wxRED_PEN);
    dc.DrawRectangle( points[0].x - 20, points[0].y - 20, 40, 40);
    dc.SetPen(*wxGREEN_PEN);
    dc.DrawRectangle( points[10].x - 20, points[10].y - 20, 40, 40);
#endif
}

// -------------------------------------------------------------------------------------------------------

void AriaMaestosa::RenderRoutines::renderArc(wxGraphicsContext& dc, const int center_x, const int center_y,
                                             const int radius_x, const int radius_y)
{
#if wxCHECK_VERSION(2,9,1) && wxUSE_GRAPHICS_CONTEXT
    wxGraphicsPath path = dc.CreatePath();
    
    path.MoveToPoint(center_x + radius_x*cos(0.1), center_y + radius_y*sin(0.1));
    for (float angle = 0.3f; angle <= 3.0f; angle += 0.3f)
    {
        path.AddLineToPoint(center_x + radius_x*cos(angle), center_y + radius_y*sin(angle));
    }
    
    dc.StrokePath(path);
#else
    assert(false);
#endif
}

// -------------------------------------------------------------------------------------------------------

wxBitmap AriaMaestosa::RenderRoutines::getScaledBitmap(const wxString& fileName, float scale)
{
    wxImage tempImage(getResourcePrefix() + wxT("score") + wxFileName::GetPathSeparator() + fileName,
                      wxBITMAP_TYPE_PNG);
    wxImage image = getPrintableImage(tempImage);
    return wxBitmap(image.Scale(image.GetWidth()*scale, image.GetHeight()*scale),wxIMAGE_QUALITY_HIGH);
}

// -------------------------------------------------------------------------------------------------------

wxImage AriaMaestosa::RenderRoutines::getPrintableImage(const wxImage& image)
{
#ifdef __WXMSW__
    wxImage printimage = image;
    if (printimage.HasAlpha())
    {
        printimage.ConvertAlphaToMask();
    }
    if (printimage.HasMask())
    {
        const wxColour mask(printimage.GetMaskRed(), printimage.GetMaskGreen(), printimage.GetMaskBlue());
        const wxColour back = *wxWHITE;
        
        printimage.Replace(mask.Red(), mask.Green(), mask.Blue(),
                           back.Red(), back.Green(), back.Blue());
    }
    return printimage;
#else
    return image;
#endif
}
