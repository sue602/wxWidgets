/////////////////////////////////////////////////////////////////////////////
// Name:        univ/statusbr.cpp
// Purpose:     wxStatusBar implementation
// Author:      Vadim Zeitlin
// Modified by:
// Created:     14.10.01
// RCS-ID:      $Id$
// Copyright:   (c) 2000 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#ifdef __GNUG__
    #pragma implementation "univstatusbr.h"
#endif

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STATUSBAR

#ifndef WX_PRECOMP
    #include "wx/settings.h"
    #include "wx/dcclient.h"
#endif

#include "wx/statusbr.h"

#include "wx/univ/renderer.h"

// ============================================================================
// implementation
// ============================================================================

BEGIN_EVENT_TABLE(wxStatusBarUniv, wxStatusBarBase)
    EVT_SIZE(wxStatusBarUniv::OnSize)

    WX_EVENT_TABLE_INPUT_CONSUMER(wxStatusBarUniv)
END_EVENT_TABLE()

WX_FORWARD_TO_INPUT_CONSUMER(wxStatusBarUniv)

// ----------------------------------------------------------------------------
// creation
// ----------------------------------------------------------------------------

void wxStatusBarUniv::Init()
{
}

bool wxStatusBarUniv::Create(wxWindow *parent,
                             wxWindowID id,
                             long style,
                             const wxString& name)
{
    if ( !wxWindow::Create(parent, id,
                           wxDefaultPosition, wxDefaultSize,
                           style, name) )
    {
        return FALSE;
    }

    SetFieldsCount(1);

    CreateInputHandler(wxINP_HANDLER_STATUSBAR);

    SetSize(DoGetBestSize());

    return TRUE;
}

// ----------------------------------------------------------------------------
// drawing
// ----------------------------------------------------------------------------

wxRect wxStatusBarUniv::GetTotalFieldRect(wxCoord *borderBetweenFields)
{
    wxRect rect = GetClientRect();

    // no, don't do this - the borders are meant to be inside this rect
    // wxSize sizeBorders =
    m_renderer->GetStatusBarBorders(borderBetweenFields);
    //rect.Deflate(sizeBorders.x, sizeBorders.y);

    // recalc the field widths if needed
    if ( m_widthsAbs.IsEmpty() )
    {
        // the total width for the fields doesn't include the borders between
        // them
        m_widthsAbs = CalculateAbsWidths(rect.width -
                                         *borderBetweenFields*(m_nFields - 1));
    }

    return rect;
}

void wxStatusBarUniv::DoDraw(wxControlRenderer *renderer)
{
    // get the fields rect
    wxCoord borderBetweenFields;
    wxRect rect = GetTotalFieldRect(&borderBetweenFields);

    // prepare the DC
    wxDC& dc = renderer->GetDC();
    dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

    // do draw the fields
    int flags = IsEnabled() ? 0 : wxCONTROL_DISABLED;
    for ( int n = 0; n < m_nFields; n++ )
    {
        rect.width = m_widthsAbs[n];

        if ( IsExposed(rect) )
        {
            // the size grip may be drawn only on the last field and only if we
            // have the corresponding style and even then only if we really can
            // resize this frame
            if ( n == m_nFields - 1 &&
                 HasFlag(wxST_SIZEGRIP) &&
                 GetParent()->HasFlag(wxRESIZE_BORDER) )
            {
                // NB: we use wxCONTROL_ISDEFAULT for this because it doesn't
                //     have any meaning for the status bar otherwise anyhow
                //     (it's still ugly, of course, but there are too few flags
                //     to squander them for things like this)
                flags |= wxCONTROL_ISDEFAULT;
            }

            m_renderer->DrawStatusField(dc, rect, m_statusText[n], flags);
        }

        rect.x += rect.width + borderBetweenFields;
    }
}

void wxStatusBarUniv::RefreshField(int i)
{
    wxRect rect;
    if ( GetFieldRect(i, rect) )
    {
        RefreshRect(rect);
    }
}

// ----------------------------------------------------------------------------
// fields text
// ----------------------------------------------------------------------------

void wxStatusBarUniv::SetStatusText(const wxString& text, int number)
{
    wxCHECK_RET( number >= 0 && number < m_nFields,
                 _T("invalid status bar field index in SetStatusText()") );

    if ( text == m_statusText[number] )
    {
        // nothing changed
        return;
    }

    m_statusText[number] = text;

    RefreshField(number);
}

wxString wxStatusBarUniv::GetStatusText(int number) const
{
    wxCHECK_MSG( number >= 0 && number < m_nFields, _T(""),
                 _T("invalid status bar field index") );

    return m_statusText[number];
}

// ----------------------------------------------------------------------------
// fields count/widths
// ----------------------------------------------------------------------------

void wxStatusBarUniv::SetFieldsCount(int number, const int *widths)
{
    m_statusText.SetCount(number);
    wxStatusBarBase::SetFieldsCount(number, widths);
    m_widthsAbs.Empty();
}

void wxStatusBarUniv::SetStatusWidths(int n, const int widths[])
{
    wxStatusBarBase::SetStatusWidths(n, widths);

    m_widthsAbs.Empty();
}

// ----------------------------------------------------------------------------
// geometry
// ----------------------------------------------------------------------------

void wxStatusBarUniv::OnSize(wxSizeEvent& event)
{
    // invalidate the widths, we'll have to recalc them
    m_widthsAbs.Empty();

    // refresh entirely, shouldn't matter much as the statusbar is quick to
    // redraw and it would be difficult to avoid it as we'd need to find out
    // which fields exactly were affected...
    Refresh();

    event.Skip();
}

bool wxStatusBarUniv::GetFieldRect(int n, wxRect& rect) const
{
    wxCHECK_MSG( n >= 0 && n < m_nFields, FALSE,
                 _T("invalid field index in GetFieldRect()") );

    // this is a fix for a bug exhibited by the statbar sample: if
    // GetFieldRect() is called from the derived class OnSize() handler, then
    // our geometry info is wrong as our OnSize() didn't invalidate m_widthsAbs
    // yet - so recalc it just in case
    wxStatusBarUniv *self = wxConstCast(this, wxStatusBarUniv);
    self->m_widthsAbs.Empty();

    wxCoord borderBetweenFields;
    rect = self->GetTotalFieldRect(&borderBetweenFields);
    for ( int i = 0; i <= n; i++ )
    {
        rect.width = m_widthsAbs[i];

        if ( i < n )
            rect.x += rect.width + borderBetweenFields;
    }

    return TRUE;
}

wxCoord wxStatusBarUniv::GetHeight() const
{
    wxClientDC dc(wxConstCast(this, wxStatusBarUniv));
    dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

    return dc.GetCharHeight() + 2*GetBorderY();
}

wxSize wxStatusBarUniv::DoGetBestSize() const
{
    return wxSize(100, GetHeight());
}

void wxStatusBarUniv::DoSetSize(int x, int y,
                                int width, int WXUNUSED(height),
                                int sizeFlags)
{
    wxStatusBarBase::DoSetSize(x, y, width, GetHeight(), sizeFlags);
}

// ----------------------------------------------------------------------------
// misc
// ----------------------------------------------------------------------------

void wxStatusBarUniv::SetMinHeight(int WXUNUSED(height))
{
    // nothing to do here, we don't support it - and why would we?
}

int wxStatusBarUniv::GetBorderX() const
{
    return m_renderer->GetStatusBarBorders(NULL).x;
}

int wxStatusBarUniv::GetBorderY() const
{
    return m_renderer->GetStatusBarBorders(NULL).y;
}

#endif // wxUSE_STATUSBAR

