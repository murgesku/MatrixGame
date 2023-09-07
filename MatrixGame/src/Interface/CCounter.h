// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#define COUNTER_IMAGES 7
#define UP_LIMIT       6
#define DOWN_LIMIT     1

class CIFaceElement;

class CIFaceCounter : public CMain {
    int m_Counter;
    CIFaceElement *m_CounterImage[COUNTER_IMAGES];
    CIFaceElement *m_ButtonUp, *m_ButtonDown;

    void MulRes();
    void DivRes();

public:
    int GetCounter() { return m_Counter; }
    CIFaceElement *GetImage() { return m_CounterImage[m_Counter]; }
    void SetImage(int i, CIFaceElement *image) { m_CounterImage[i] = image; }
    CIFaceElement *GetZeroImage() { return m_CounterImage[0]; }
    int Inc() {
        if (++m_Counter > UP_LIMIT)
            m_Counter = UP_LIMIT;
        return m_Counter;
    }
    int Dec() {
        if (--m_Counter < DOWN_LIMIT)
            m_Counter = DOWN_LIMIT;
        return m_Counter;
    }

    void SetButtonDown(CIFaceElement *button) { m_ButtonDown = button; }
    void SetButtonUp(CIFaceElement *button) { m_ButtonUp = button; }
    void ManageButtons();

    void CheckUp();
    void Reset() {
        m_Counter = DOWN_LIMIT;
        ManageButtons();
    }
    void __stdcall Up(void*) {
        Inc();
        ManageButtons();
        MulRes();
        CheckUp();
    }
    void __stdcall Down(void*) {
        Dec();
        ManageButtons();
        DivRes();
        CheckUp();
    }

    void Disable() {
        if (m_ButtonDown)
            m_ButtonDown->SetState(IFACE_DISABLED);
        if (m_ButtonUp)
            m_ButtonUp->SetState(IFACE_DISABLED);
        m_Counter = 0;
    }
    void Enable() {
        if (m_ButtonDown)
            m_ButtonDown->SetState(IFACE_NORMAL);
        if (m_ButtonUp)
            m_ButtonUp->SetState(IFACE_NORMAL);
        Reset();
        CheckUp();
    }

    CIFaceCounter();
    ~CIFaceCounter();
};