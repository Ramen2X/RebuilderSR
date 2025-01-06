#include "patchgrid.h"

#include <D3DRM.H>
#include <DDRAW.H>
#include <SHLWAPI.H>
#include <SSTREAM>

#include "../cmn/path.h"

HRESULT CALLBACK ReceiveD3DDevice(GUID *lpGuid, LPSTR szDeviceDescription, LPSTR szDeviceName, LPD3DDEVICEDESC pD3DDD1, LPD3DDEVICEDESC pD3DDD2, LPVOID lpContext)
{
  char buf[256];

  // NOTE: Not really sure where the "0" comes from. I'm guessing it's the index of the ddraw
  //       device, but I don't have any hardware that has a second one (I only have "Primary
  //       Display Driver") so I can't experiment.
  sprintf(buf, "0 0x%lx 0x%x%x 0x%lx 0x%lx", lpGuid->Data1, lpGuid->Data3, lpGuid->Data2,
          *((DWORD*)&lpGuid->Data4), *((DWORD*)&lpGuid->Data4[4]));

  PatchGrid *p = (PatchGrid*)lpContext;
  p->AddD3DDevice(szDeviceName, buf);

  return DDENUMRET_OK;
}

BOOL CALLBACK ReceiveDDrawDevice(GUID *lpGuid, LPSTR szDesc, LPSTR szName, LPVOID lpContext)
{
  LPDIRECTDRAW dd;
  LPDIRECT3D2 d3d2;

  DirectDrawCreate(lpGuid, &dd, NULL);

  dd->QueryInterface(IID_IDirect3D2, (void**)&d3d2);
  d3d2->EnumDevices(ReceiveD3DDevice, lpContext);

  d3d2->Release();
  dd->Release();

  return DDENUMRET_OK;
}

PatchGrid::PatchGrid()
{
  SetBoldModified(true);

  // Controls section
  HSECTION sectionControls = AddSection("Controls");

  AddPatch("UseJoystick",
           "Enables Joystick functionality.",
           AddBoolItem(sectionControls, "Use Joystick", false));

  // Graphics Section
  HSECTION sectionGraphics = AddSection("Graphics");

  DirectDrawEnumerateA(ReceiveDDrawDevice, this);
  m_d3dDeviceItem = AddComboItem(sectionGraphics, "Direct3D Device", m_d3dDeviceNames, 0);

  AddPatch("D3DDevice",
           "Set which Direct3D device to use with LEGO Island.",
           m_d3dDeviceItem);

  AddPatch("FullScreen",
           "Allows you to change modes without administrator privileges and registry editing. NOTE: Windowed mode is NOT compatible with \"Flip Video Memory Pages\".",
           AddBoolItem(sectionGraphics, "Run in Full Screen", true));

  AddPatch("DrawCursor",
           "Renders a custom in-game cursor, rather than a standard Windows pointer.",
           AddBoolItem(sectionGraphics, "Draw Cursor", false));

  AddPatch("FlipSurfaces",
           "", // FIXME: Write description for this
           AddBoolItem(sectionGraphics, "Flip Video Memory Pages", false));

  AddPatch("BackBuffersInVRAM",
           "", // FIXME: Write description for this
           AddBoolItem(sectionGraphics, "Draw 3D to Video Memory", false));

  vector<string> fpsList;
  fpsList.push_back("Default");
  fpsList.push_back("Limited");
  AddPatch("FPSLimit",
           "Modify LEGO Island's frame rate cap",
           AddComboItem(sectionGraphics, "FPS Cap", fpsList, 0));

  AddPatch("CustomFPS",
           "If 'FPS Cap' is set to 'Limited', this will be the frame rate used.",
           AddDoubleItem(sectionGraphics, "FPS Cap - Custom Limit", 24.0));

  // Audio section
  HSECTION sectionMusic = AddSection("Audio");

  AddPatch("MusicToggle",
           "Turns in-game music on or off.",
           AddBoolItem(sectionMusic, "Play Music", true));
}

template<typename T>
std::string toString(const T &value)
{
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

void PatchGrid::LoadConfiguration(LPCTSTR filename)
{
  for (std::map<std::string, HITEM>::const_iterator it=m_mPatchItems.begin(); it!=m_mPatchItems.end(); it++) {
    CItem *item = FindItem(it->second);

    std::string value;
    value.resize(1024);

    char buf[1024];

    DWORD sz = GetPrivateProfileString(appName, it->first.c_str(), NULL, &value[0], value.size(), filename);

    // If this entry wasn't in the profile, skip it
    if (!sz) {
      continue;
    }

    value.resize(sz);

    // Convert value to string
    switch (item->m_type) {
    case IT_STRING:
    case IT_TEXT:
    case IT_FILE:
    case IT_FOLDER:
    case IT_COMBO:
      SetItemValue(it->second, value);
      break;
    case IT_BOOLEAN:
      SetItemValue(it->second, (bool)StrToIntA(value.c_str()));
      break;
    case IT_INTEGER:
      SetItemValue(it->second, StrToIntA(value.c_str()));
      break;
    case IT_DOUBLE:
      SetItemValue(it->second, atof(value.c_str()));
      break;
    case IT_COLOR:
      SetItemValue(it->second, (COLORREF) StrToIntA(value.c_str()));
      break;
    case IT_CUSTOM:
    case IT_DATE:
    case IT_DATETIME:
    case IT_FONT:
    {
      // Report inability to serialize
      TCHAR buf[200];
      sprintf(buf, "Failed to serialize %s from string.", it->first.c_str());
      MessageBox(buf);
      break;
    }
    }
  }
}

BOOL PatchGrid::SaveConfiguration(LPCTSTR filename)
{
  for (std::map<std::string, HITEM>::const_iterator it=m_mPatchItems.begin(); it!=m_mPatchItems.end(); it++) {
    CItem *item = FindItem(it->second);

    std::string value;

    // Convert value to string
    switch (item->m_type) {
    case IT_STRING:
    case IT_TEXT:
    case IT_FILE:
    case IT_FOLDER:
    case IT_COMBO:
      GetItemValue(it->second, value);
      break;
    case IT_BOOLEAN:
    {
      bool b;
      GetItemValue(it->second, b);
      value = toString(b);
      break;
    }
    case IT_INTEGER:
      int i;
      GetItemValue(it->second, i);
      value = toString(i);
      break;
    case IT_DOUBLE:
      double d;
      GetItemValue(it->second, d);
      value = toString(d);
      break;
    case IT_COLOR:
      COLORREF c;
      GetItemValue(it->second, c);
      value = toString(c);
      break;
    case IT_CUSTOM:
    case IT_DATE:
    case IT_DATETIME:
    case IT_FONT:
    {
      // Report inability to serialize
      TCHAR buf[200];
      sprintf(buf, "Failed to serialize %s to string.", it->first.c_str());
      MessageBox(buf);
      break;
    }
    }

    if (!WritePrivateProfileString(appName, it->first.c_str(), value.c_str(), filename)) {
      return FALSE;
    }

    if (it->second == m_d3dDeviceItem) {
      int device_index;
      GetItemValue(it->second, device_index);
      WritePrivateProfileString(appName, "D3DDeviceID", m_d3dDeviceIDs.at(device_index).c_str(), filename);
    }
  }

  return TRUE;
}

void PatchGrid::AddD3DDevice(const string &name, const string &id)
{
  m_d3dDeviceNames.push_back(name);
  m_d3dDeviceIDs.push_back(id);
}

CString PatchGrid::GetItemDescription(HITEM item)
{
  for (std::map<std::string, HITEM>::const_iterator it=m_mPatchItems.begin(); it!=m_mPatchItems.end(); it++) {
    if (it->second == item) {
      return GetItemDescription(it->first);
    }
  }
  return CString();
}

void PatchGrid::AddPatch(const string &id, const CString &description, HITEM item)
{
  m_mPatchItems[id] = item;
  m_mPatchDescriptions[id] = description;
}
