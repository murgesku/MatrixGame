// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#ifndef _DEVICE_STATE_INCLUDE
#define _DEVICE_STATE_INCLUDE

#define DEVSTATE_ALL                (-1)
#define DEVSTATE_RENDERSTATES       SETBIT(0)
#define DEVSTATE_SAMPLERSTATES      SETBIT(1)
#define DEVSTATE_LIGHTS             SETBIT(2)
#define DEVSTATE_MATERIAL           SETBIT(3)

#define RS_COUNT        103
#define SS_COUNT        13
#define LI_COUNT        8

class CDeviceState : public CMain
{
    IDirect3DDevice9 *m_Device;

    DWORD             m_RenderStates[RS_COUNT];
    DWORD             m_SamplerStates[SS_COUNT * 8];
    BOOL              m_LightsEnabled[LI_COUNT];
    D3DLIGHT9         m_Lights[LI_COUNT];
    D3DMATERIAL9      m_Material;
    

public:
    CDeviceState(IDirect3DDevice9 *dev):m_Device(dev) {}


    void StoreState(DWORD states = DEVSTATE_ALL);
    void RestoreState(DWORD states = DEVSTATE_ALL);

};


#endif