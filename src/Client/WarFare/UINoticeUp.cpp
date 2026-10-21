#include "StdAfx.h"
#include "UINoticeUp.h"

#include <N3Base/N3UIString.h>
#include <N3Base/N3UIDef.h>
#include <N3Base/My_3DStruct.h>

#include <algorithm>

namespace
{
constexpr int PanelWidth = 242;
constexpr int HeaderHeight = 25;
constexpr int RowHeight = 27;
constexpr int DetailHeight = 92;
constexpr int MarginRight = 14;
constexpr int OffsetTop = 160; // Below the default HUD/minimap; can be tuned independently.
}

// Native 2D UI rendering, using the same RHI primitive pattern as the existing GM panel.
void CUINoticeUp::Render()
{
    if (!IsVisible())
        return;

    const auto rc = GetRegion();
    DWORD oldZ = 0, oldFog = 0, oldAlpha = 0, oldSrc = 0, oldDest = 0;
    DWORD oldFVF = 0, oldCull = 0, oldCOP = 0, oldCA1 = 0, oldAOP = 0, oldAA1 = 0;
    auto* device = CN3Base::RHIDevice();
    if (device != nullptr)
    {
        device->GetRenderState(D3DRS_ZENABLE, &oldZ);
        device->GetRenderState(D3DRS_FOGENABLE, &oldFog);
        device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlpha);
        device->GetRenderState(D3DRS_SRCBLEND, &oldSrc);
        device->GetRenderState(D3DRS_DESTBLEND, &oldDest);
        device->GetRenderState(D3DRS_CULLMODE, &oldCull);
        device->GetTextureStageState(0, D3DTSS_COLOROP, &oldCOP);
        device->GetTextureStageState(0, D3DTSS_COLORARG1, &oldCA1);
        device->GetTextureStageState(0, D3DTSS_ALPHAOP, &oldAOP);
        device->GetTextureStageState(0, D3DTSS_ALPHAARG1, &oldAA1);
        device->GetFVF(&oldFVF);

        device->SetRenderState(D3DRS_ZENABLE, FALSE);
        device->SetRenderState(D3DRS_FOGENABLE, FALSE);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        device->SetFVF(FVF_TRANSFORMEDCOLOR);

        auto draw = [&](int x, int y, int width, int height, uint32_t color) {
            __VertexTransformedColor v[4] = {
                {float(x), float(y), UI_DEFAULT_Z, UI_DEFAULT_RHW, color},
                {float(x + width), float(y), UI_DEFAULT_Z, UI_DEFAULT_RHW, color},
                {float(x + width), float(y + height), UI_DEFAULT_Z, UI_DEFAULT_RHW, color},
                {float(x), float(y + height), UI_DEFAULT_Z, UI_DEFAULT_RHW, color}
            };
            device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(__VertexTransformedColor));
        };
        draw(rc.left, rc.top, PanelWidth, HeaderHeight, D3DCOLOR_ARGB(217, 9, 22, 27));
        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            const int y = rc.top + HeaderHeight + static_cast<int>(i) * RowHeight;
            const uint32_t color = (m_selected == static_cast<int>(i))
                ? D3DCOLOR_ARGB(232, 29, 95, 87)
                : (i % 2 ? D3DCOLOR_ARGB(202, 12, 48, 54) : D3DCOLOR_ARGB(202, 13, 59, 61));
            draw(rc.left + 2, y + 1, PanelWidth - 4, RowHeight - 2, color);
            draw(rc.left + 2, y + 1, PanelWidth - 4, 1, D3DCOLOR_ARGB(180, 133, 166, 141));
        }
        if (m_selected >= 0)
        {
            const int y = rc.top + HeaderHeight + static_cast<int>(m_entries.size()) * RowHeight + 3;
            draw(rc.left + 2, y, PanelWidth - 4, DetailHeight, D3DCOLOR_ARGB(230, 12, 25, 31));
        }

        device->SetRenderState(D3DRS_ZENABLE, oldZ);
        device->SetRenderState(D3DRS_FOGENABLE, oldFog);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlpha);
        device->SetRenderState(D3DRS_SRCBLEND, oldSrc);
        device->SetRenderState(D3DRS_DESTBLEND, oldDest);
        device->SetRenderState(D3DRS_CULLMODE, oldCull);
        device->SetTextureStageState(0, D3DTSS_COLOROP, oldCOP);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, oldCA1);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, oldAOP);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, oldAA1);
        device->SetFVF(oldFVF);
    }
    CN3UIBase::Render();
}

void CUINoticeUp::Init(CN3UIBase* parent)
{
    CN3UIBase::Init(parent);
    SetID("openko_notice_up");
    SetRegion(RECT{0, 0, PanelWidth, HeaderHeight});
    m_rows.clear();
    m_entries.clear();
    m_selected = -1;

    m_heading = new CN3UIString();
    m_heading->Init(this);
    m_heading->SetRegion(RECT{4, 0, PanelWidth - 4, HeaderHeight});
    m_heading->SetStyle(UISTYLE_STRING_SINGLELINE | UISTYLE_STRING_ALIGNCENTER
        | UISTYLE_STRING_ALIGNVCENTER);
    m_heading->SetFont("Arial", 14, TRUE, FALSE);
    m_heading->SetColor(D3DCOLOR_ARGB(255, 245, 213, 112));
    m_heading->SetString("SERVER INFORMATION");

    for (size_t i = 0; i < MaxEntries; ++i)
    {
        auto* label = new CN3UIString();
        label->Init(this);
        const int top = HeaderHeight + static_cast<int>(i) * RowHeight;
        label->SetRegion(RECT{4, top, PanelWidth - 4, top + RowHeight});
        label->SetStyle(UISTYLE_STRING_SINGLELINE | UISTYLE_STRING_ALIGNCENTER
            | UISTYLE_STRING_ALIGNVCENTER);
        label->SetFont("Arial", 14, TRUE, FALSE);
        label->SetColor(D3DCOLOR_ARGB(255, 209, 242, 231));
        label->SetVisible(false);
        m_rows.push_back(label);
    }

    m_description = new CN3UIString();
    m_description->Init(this);
    m_description->SetRegion(RECT{8, HeaderHeight, PanelWidth - 8, HeaderHeight + DetailHeight});
    m_description->SetFont("Arial", 13, FALSE, FALSE);
    m_description->SetColor(D3DCOLOR_ARGB(255, 255, 235, 166));
    m_description->SetVisible(false);

    SetVisible(false);
}

void CUINoticeUp::Release()
{
    // Base Release destroys the owned UI children; drop non-owning pointers first.
    m_rows.clear();
    m_entries.clear();
    m_heading = nullptr;
    m_description = nullptr;
    m_selected = -1;
    CN3UIBase::Release();
}

void CUINoticeUp::UpdateHeight()
{
    const int rowsEnd = HeaderHeight + static_cast<int>(m_entries.size()) * RowHeight;
    if (m_description != nullptr)
    {
        m_description->SetRegion(RECT{m_rcRegion.left + 8, m_rcRegion.top + rowsEnd + 3,
            m_rcRegion.left + PanelWidth - 8,
            m_rcRegion.top + rowsEnd + 3 + DetailHeight});
    }
    SetSize(PanelWidth, rowsEnd + (m_selected >= 0 ? DetailHeight + 3 : 0));
}

void CUINoticeUp::PositionAtTopRight(int viewportWidth)
{
    SetPos(std::max(0, viewportWidth - PanelWidth - MarginRight), OffsetTop);
}

void CUINoticeUp::SetEntries(const std::vector<Entry>& entries)
{
    m_entries.assign(entries.begin(), entries.begin() + std::min(entries.size(), MaxEntries));
    m_selected = -1;
    if (m_description != nullptr)
    {
        m_description->SetString("");
        m_description->SetVisible(false);
    }
    for (size_t i = 0; i < m_rows.size(); ++i)
    {
        CN3UIString* row = m_rows[i];
        if (i < m_entries.size())
        {
            row->SetString("[ + ]  " + m_entries[i].first);
            row->SetVisible(true);
        }
        else
        {
            row->SetString("");
            row->SetVisible(false);
        }
    }
    UpdateHeight();
    SetVisible(!m_entries.empty());
}

bool CUINoticeUp::ReceiveMessage(CN3UIBase* sender, uint32_t message)
{
    if (message != UIMSG_STRING_LCLICK)
        return CN3UIBase::ReceiveMessage(sender, message);

    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        if (sender != m_rows[i])
            continue;
        m_selected = (m_selected == static_cast<int>(i)) ? -1 : static_cast<int>(i);
        if (m_description != nullptr)
        {
            m_description->SetString(m_selected >= 0 ? m_entries[i].second : "");
            m_description->SetVisible(m_selected >= 0);
        }
        UpdateHeight();
        return true;
    }
    return true;
}
