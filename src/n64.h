#pragma once

#include "gamepak.h"
#include "pif.h"

class N64 {
public:
    N64(PIF& pif, GamePak& gamepak) : m_pif(pif), m_gamepak(gamepak) {}

    PIF& pif() { return m_pif; }
    const PIF& pif() const { return m_pif; }
    GamePak& gamepak() { return m_gamepak; }
    const GamePak& gamepak() const { return m_gamepak; }

private:
    PIF& m_pif;
    GamePak& m_gamepak;
};
