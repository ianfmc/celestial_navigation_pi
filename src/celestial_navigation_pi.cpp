/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Celestial Navigation Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Sean D'Epagnier   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "../../../include/ocpn_plugin.h"

#include "Sight.h"
#include "celestial_navigation_pi.h"
#include "icons.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return (opencpn_plugin *)new celestial_navigation_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

//---------------------------------------------------------------------------------------------------------
//
//    Celestial_Navigation PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

celestial_navigation_pi::celestial_navigation_pi(void *ppimgr)
    :opencpn_plugin_18 (ppimgr)
{
    // Create the PlugIn icons
    initialize_images();
}

celestial_navigation_pi::~celestial_navigation_pi(void){}


//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

extern "C" int geomag_load(const char *mdfile);

int celestial_navigation_pi::Init(void)
{
    // Get a pointer to the opencpn display canvas, to use as a parent for windows created
    m_parent_window = GetOCPNCanvasWindow();

    // Create the Context Menu Items

    //    In order to avoid an ASSERT on msw debug builds,
    //    we need to create a dummy menu to act as a surrogate parent of the created MenuItems
    //    The Items will be re-parented when added to the real context meenu
    wxMenu dummy_menu;

    /* load the geographical magnetic table */
    wxString geomag_text_path = *GetpSharedDataLocation();
    geomag_text_path.Append(_T("plugins/celestial_navigation/data/IGRF11.COF"));
    geomag_load(geomag_text_path.mb_str());

    //    This PlugIn needs a toolbar icon, so request its insertion
    m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_celestial_navigation,
                                            _img_celestial_navigation, wxITEM_NORMAL,
                                            _("Celestial Navigation"), _T(""), NULL,
                                            CELESTIAL_NAVIGATION_TOOL_POSITION, 0, this);

    m_pCelestialNavigationDialog = NULL;

    return (WANTS_OVERLAY_CALLBACK |
            WANTS_OPENGL_OVERLAY_CALLBACK |
            WANTS_CURSOR_LATLON       |
            WANTS_TOOLBAR_CALLBACK    |
            INSTALLS_TOOLBAR_TOOL
        );
}

bool celestial_navigation_pi::DeInit(void)
{
    RemovePlugInTool(m_leftclick_tool_id);

    if(m_pCelestialNavigationDialog)
    {
        m_pCelestialNavigationDialog->Close();
        delete m_pCelestialNavigationDialog;
        m_pCelestialNavigationDialog = NULL;
    }
    return true;
}

int celestial_navigation_pi::GetAPIVersionMajor()
{
    return MY_API_VERSION_MAJOR;
}

int celestial_navigation_pi::GetAPIVersionMinor()
{
    return MY_API_VERSION_MINOR;
}

int celestial_navigation_pi::GetPlugInVersionMajor()
{
    return PLUGIN_VERSION_MAJOR;
}

int celestial_navigation_pi::GetPlugInVersionMinor()
{
    return PLUGIN_VERSION_MINOR;
}

wxBitmap *celestial_navigation_pi::GetPlugInBitmap()
{
      return _img_celestial_navigation;
}

wxString celestial_navigation_pi::GetCommonName()
{
    return _("Celestial");
}

wxString celestial_navigation_pi::GetShortDescription()
{
    return _("Celestial Navigation PlugIn for OpenCPN");
}

wxString celestial_navigation_pi::GetLongDescription()
{
    return _("Celestial Navigation PlugIn for OpenCPN.\n\
Implements nautical almanac for sun, moon, planets, and various \
navigational stars.  Enable the User to compute position fix from \
celestial measurements.");
}

void celestial_navigation_pi::OnToolbarToolCallback(int id)
{
    if(NULL == m_pCelestialNavigationDialog)
        m_pCelestialNavigationDialog = new CelestialNavigationDialog(m_parent_window);

    m_pCelestialNavigationDialog->Show();
}

int celestial_navigation_pi::GetToolbarToolCount(void)
{
      return 1;
}

void celestial_navigation_pi::SetColorScheme(PI_ColorScheme cs)
{
    if (NULL == m_pCelestialNavigationDialog)
        return;
    DimeWindow(m_pCelestialNavigationDialog);
}

bool celestial_navigation_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    return RenderOverlayAll(&dc, vp);
}

bool celestial_navigation_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    return RenderOverlayAll(NULL, vp);
} 

bool celestial_navigation_pi::RenderOverlayAll(wxDC *dc, PlugIn_ViewPort *vp)
{
   if(!m_pCelestialNavigationDialog)
      return false;

   wxSightListNode *node = m_pCelestialNavigationDialog->m_SightList.GetFirst();
   while ( node )
   {
      Sight *pSightRender = node->GetData();
      if ( pSightRender )
        pSightRender->Render ( dc, *vp );
      
      node = node->GetNext();
   }
   return true;
}

static double s_cursor_lat, s_cursor_lon;

void celestial_navigation_pi::SetCursorLatLon(double lat, double lon)
{
   s_cursor_lat = lat;
   s_cursor_lon = lon;
}

double celestial_navigation_pi_CursorLat()
{
   return s_cursor_lat;
}

double celestial_navigation_pi_CursorLon()
{
   return s_cursor_lon;
}