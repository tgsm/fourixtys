#pragma once

#include "common/types.h"

class MMU;

class PI {
public:
    explicit PI(MMU& mmu) : m_mmu(mmu) {}

    void set_dram_address(u32 address) { m_dram_address = address; }

    void set_dma_cart_address(u32 address) { m_dma_cart_address = address; }

    void set_dma_write_length(u32 value) {
        m_dma_write_length = value + 1;
        run_dma_transfer_to_rdram();
    }

private:
    MMU& m_mmu;

    u32 m_dram_address {};
    u32 m_dma_cart_address {};
    u32 m_dma_write_length {};
    u32 m_status {};

    void run_dma_transfer_to_rdram();
};
