#include "tabs.h"

void TabCtrl::CreateChildren()
{
  // Create property grid
  m_cPatchGrid.Create(AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW), "Patch Grid", WS_CHILD | WS_VISIBLE, CRect(), this, ID_PATCHGRID);
  m_cPatchTitle.Create(CString("Welcome to LEGO Island Rebuilder (SR)"), WS_CHILD | WS_VISIBLE, CRect(), this, ID_PATCHTITLE);
  m_cPatchDesc.Create(CString("Select an option to get started."), WS_CHILD | WS_VISIBLE, CRect(), this, ID_PATCHDESC);
}

HBRUSH TabCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
  HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

  if (pWnd->GetDlgCtrlID() == ID_PATCHTITLE
      || pWnd->GetDlgCtrlID() == ID_PATCHDESC) {
    pDC->SetBkMode(TRANSPARENT);
    return (HBRUSH) GetStockObject(NULL_BRUSH);
  }
  
  return hbr;
}

LRESULT TabCtrl::OnGridSelChange(WPARAM wParam, LPARAM lParam)
{
  HITEM hItem = (HITEM) wParam;

  m_cPatchTitle.SetWindowText(m_cPatchGrid.GetItemText(hItem).c_str());
  m_cPatchDesc.SetWindowText(m_cPatchGrid.GetItemDescription(hItem));

  CRect rect;
  m_cPatchTitle.GetWindowRect(&rect);
  ScreenToClient(&rect);
  InvalidateRect(&rect);
  UpdateWindow();

  m_cPatchDesc.GetWindowRect(&rect);
  ScreenToClient(&rect);
  InvalidateRect(&rect);
  UpdateWindow();

  return 0;
}


BEGIN_MESSAGE_MAP(TabCtrl, CTabCtrl)
  ON_WM_CTLCOLOR()
  ON_MESSAGE(WM_PG_SELECTIONCHANGED, OnGridSelChange)
END_MESSAGE_MAP()
