#ifndef OPENKO_DYNAMIC_NPC_MAP_H
#define OPENKO_DYNAMIC_NPC_MAP_H

// Concurrent NPC lookup / publication. Keys are fixed protocol IDs, so readers
// never traverse a std::map while another network thread inserts an NPC.
#include <atomic>
#include <array>
#include <vector>
#include <utility>

namespace openko_ext {
template<class NPC, size_t Capacity>
class DynamicNpcMap final {
public:
    DynamicNpcMap() {
        for (auto& slot : _slots) slot.store(nullptr, std::memory_order_relaxed);
    }
    DynamicNpcMap(const DynamicNpcMap&) = delete;
    DynamicNpcMap& operator=(const DynamicNpcMap&) = delete;
    ~DynamicNpcMap() { DeleteAllData(); }

    NPC* GetData(int key) const {
        if (key < 0 || static_cast<size_t>(key) >= Capacity) return nullptr;
        return _slots[key].load(std::memory_order_acquire);
    }
    bool PutData(int key, NPC* npc) {
        if (npc == nullptr || key < 0 || static_cast<size_t>(key) >= Capacity) return false;
        NPC* expected = nullptr;
        if (!_slots[key].compare_exchange_strong(expected, npc, std::memory_order_release,
                                                  std::memory_order_relaxed)) return false;
        _count.fetch_add(1, std::memory_order_relaxed);
        int next = key + 1, prior = _highest.load(std::memory_order_relaxed);
        while (next > prior && !_highest.compare_exchange_weak(prior, next,
                    std::memory_order_release, std::memory_order_relaxed)) {}
        return true;
    }
    int GetSize() const { return _count.load(std::memory_order_acquire); }
    int GetHighWaterMark() const { return _highest.load(std::memory_order_acquire); }
    std::vector<std::pair<int, NPC*>> Snapshot() const {
        std::vector<std::pair<int, NPC*>> out;
        out.reserve(static_cast<size_t>(GetSize()));
        const int count = GetHighWaterMark();
        for (int i=0; i<count; ++i) {
            if (NPC* npc=GetData(i)) out.emplace_back(i,npc);
        }
        return out;
    }
    void DeleteAllData() { // Shutdown only: caller must stop network / NPC workers.
        for (auto& slot : _slots) delete slot.exchange(nullptr, std::memory_order_acq_rel);
        _count.store(0, std::memory_order_relaxed);
        _highest.store(0, std::memory_order_relaxed);
    }
private:
    std::array<std::atomic<NPC*>, Capacity> _slots;
    std::atomic<int> _count{0};
    std::atomic<int> _highest{0};
};
}
#endif
